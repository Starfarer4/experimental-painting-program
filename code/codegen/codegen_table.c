/*  =======================================================================
    File: codegen_table.c
    Date: March 27th 2024  3:49 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

static MD_String8 TableTagTable = MD_S8LitComp("table");
static MD_String8 TableTagTableGen = MD_S8LitComp("gen");
static MD_String8 TableTagTableGenEnum = MD_S8LitComp("gen_enum");
static MD_String8 TableTagTableGenData = MD_S8LitComp("gen_data");

node_array NodeArrayMake(MD_u64 Count)
{
    MD_Arena *Arena = CodegenState->Arena;
    node_array Result = {0};
    Result.Count = Count;
    Result.v = MD_PushArrayZero(Arena, MD_Node *, Result.Count);
    for(MD_u64 Index = 0; Index < Result.Count; ++Index)
        Result.v[Index] = MD_NilNode();
    return Result;
}

node_grid GridFromNode(MD_Node *Node)
{
    node_grid Grid = {0};

    MD_u64 RowCount = 0;
    MD_u64 ColumnCount = 0;
    for(MD_EachNode(Row, Node->first_child))
    {
        ++RowCount;
        MD_u64 CellCountThisRow = MD_ChildCountFromNode(Row);
        ColumnCount = MD_Max(CellCountThisRow, ColumnCount);
    }

    Grid.Cells = NodeArrayMake(RowCount * ColumnCount);
    Grid.RowParents = NodeArrayMake(RowCount);

    MD_u64 RowIndex = 0;
    for(MD_EachNode(Row, Node->first_child))
    {
        MD_u64 ColumnIndex = 0;
        Grid.RowParents.v[RowIndex] = Row;
        for(MD_EachNode(Cell, Row->first_child))
        {
            Grid.Cells.v[RowIndex * ColumnCount + ColumnIndex] = Cell;
            ++ColumnIndex;
        }
        ++RowIndex;
    }
    
    return Grid;
}

table_header TableHeaderFromTag(MD_Node *Tag)
{
    MD_Arena *Arena = CodegenState->Arena;
    table_header Result = {0};
    Result.ColumnCount = MD_ChildCountFromNode(Tag);
    Result.ColumnDescs = MD_PushArrayZero(Arena, column_desc, Result.ColumnCount);
    MD_u64 Index = 0;
    for(MD_EachNode(ColumnNode, Tag->first_child))
    {
        Result.ColumnDescs[Index].Kind = COLUMN_KIND_DEFAULT;
        Result.ColumnDescs[Index].Name = ColumnNode->string;
        MD_Node *CheckForTag = MD_TagFromString(ColumnNode, MD_S8Lit("check_for_tag"), 0);
        if(!MD_NodeIsNil(CheckForTag))
        {
            Result.ColumnDescs[Index].Kind = COLUMN_KIND_CHECK_FOR_TAG;
            Result.ColumnDescs[Index].TagString = CheckForTag->first_child->string;
        }
        ++Index;
    }
    return Result;
}

MD_u64 RowChildIndexFromColumnName(table_header *Header, MD_String8 ColumnName)
{
    MD_u64 Result = 0;
    for(MD_u64 Index = 0; Index < Header->ColumnCount; ++Index)
    {
        if(MD_S8Match(Header->ColumnDescs[Index].Name, ColumnName, 0))
            break;
        if(Header->ColumnDescs[Index].Kind == COLUMN_KIND_DEFAULT)
            ++Result;
    }
    return Result;
}

MD_i64 TableExprEvaluateNumeric(expand_info *Info, MD_Expr *Expr)
{
    MD_i64 Result = 0;
    table_op Op = Expr->op ? Expr->op->op_id : TABLE_OP_NULL;
    switch(Op)
    {
        case TABLE_OP_EQUAL:
        case TABLE_OP_IS_NOT_EQUAL:
        {
            MD_ArenaTemp Scratch = MD_GetScratch(0, 0);
            MD_String8List LeftStrs = {0};
            MD_String8List RightStrs = {0};
            TableExprEvaluateString(Info, Expr->left, &LeftStrs);
            TableExprEvaluateString(Info, Expr->right, &RightStrs);
            MD_String8 LeftStr = MD_S8ListJoin(Scratch.arena, LeftStrs, 0);
            MD_String8 RightStr = MD_S8ListJoin(Scratch.arena, RightStrs, 0);
            Result = MD_S8Match(LeftStr, RightStr, 0);
            if(Op == TABLE_OP_IS_NOT_EQUAL)
                Result = !Result;
            MD_ReleaseScratch(Scratch);
        }break;

        case TABLE_OP_BOOLEAN_AND:
        case TABLE_OP_BOOLEAN_OR:
        {
            MD_i64 Left = TableExprEvaluateNumeric(Info, Expr->left);
            MD_i64 Right = TableExprEvaluateNumeric(Info, Expr->right);
            switch(Op)
            {
                case TABLE_OP_BOOLEAN_AND: Result = Left && Right; break;
                case TABLE_OP_BOOLEAN_OR: Result = Left || Right; break;
            }
        }break;
    }
    return Result;
}

void TableExprEvaluateString(expand_info *Info, MD_Expr *Expr, MD_String8List *Output)
{
    MD_Arena *Arena = CodegenState->Arena;
    table_op Op = Expr->op ? Expr->op->op_id : TABLE_OP_NULL;
    switch(Op)
    {
        default:
        case TABLE_OP_NULL:
        {
            MD_S8ListPush(Arena, Output, Expr->md_node->string);
        }break;

        case TABLE_OP_DOT:
        {
            MD_Expr *LabelExpr = Expr->left;
            MD_Expr *ColumnQueryExpr = Expr->right;
            MD_Node *LabelNode = LabelExpr->md_node;
            MD_Node *ColumnQueryNode = ColumnQueryExpr->md_node;
            MD_String8 Label = LabelNode->string;
            MD_String8 ColumnQuery = ColumnQueryNode->string;
            MD_b32 ColumnQueryIsByName = ColumnQueryNode->flags & MD_NodeFlag_Identifier;
            MD_b32 ColumnQueryIsByIndex = ColumnQueryNode->flags & MD_NodeFlag_Numeric;

            expand_iter *Iter = 0;
            for(expand_iter *It = Info->FirstExpandIter; It; It = It->next)
            {
                if(MD_S8Match(It->Label, Label, 0))
                {
                    Iter = It;
                    break;
                }
            }

            if(Iter == 0)
            {
                MD_PrintMessageFmt(stderr, MD_CodeLocFromNode(LabelNode), MD_MessageKind_Error, "Expansion label \"%S\" was not found as referring to a valid @expand tag.", Label);
            }

            if(Iter)
            {
                node_grid *Grid = Iter->Grid;
                table_header *Header = Iter->Header;
                MD_Node *Row = Grid->RowParents.v[Iter->Index];

                MD_String8 CellString = {0};
                {
                    if(ColumnQueryIsByName && Header)
                    {
                        MD_u64 ColumnIdx = 0;
                        column_desc *Column = 0;
                        for(MD_u64 ColIdx = 0; ColIdx < Header->ColumnCount; ++ColIdx)
                        {
                            if(MD_S8Match(Header->ColumnDescs[ColIdx].Name, ColumnQuery, 0))
                            {
                                Column = &Header->ColumnDescs[ColIdx];
                                ColumnIdx = ColIdx;
                                break;
                            }
                        }
                        MD_u64 RowChildIdx = RowChildIndexFromColumnName(Header, ColumnQuery);

                        if(Column == 0)
                        {
                            MD_PrintMessageFmt(stderr, MD_CodeLocFromNode(ColumnQueryNode), MD_MessageKind_Error, "Column query \"%S\" did not map to a valid column for expansion label \"%S\".", ColumnQuery, Label);
                        }

                        if(Column)
                        {
                            switch(Column->Kind)
                            {
                                default:
                                case COLUMN_KIND_DEFAULT:
                                {
                                    MD_Node *CellNode = MD_ChildFromIndex(Row, RowChildIdx);
                                    CellString = CellNode->string;
                                    if(MD_S8Match(CellNode->raw_string, MD_S8Lit("/"), 0))
                                        CellString = MD_S8Lit("");
                                }break;

                                case COLUMN_KIND_CHECK_FOR_TAG:
                                {
                                    MD_b32 HasTag = MD_NodeHasTag(Row, Column->TagString, 0);
                                    CellString = HasTag ? MD_S8Lit("1") : MD_S8Lit("0");
                                }break;
                            }
                        }
                    }

                    else if(ColumnQueryIsByIndex)
                    {
                        MD_i64 Index = MD_CStyleIntFromString(ColumnQuery);
                        CellString = MD_ChildFromIndex(Row, Index)->string;
                    }
                }

                MD_S8ListPush(Arena, Output, CellString);
            }
        }break;

        case TABLE_OP_BUMP:
        {
            MD_u64 Dst = MD_CStyleIntFromString(Expr->unary_operand->md_node->string);
            MD_u64 Src = Output->total_size;
            MD_u64 SpacesToPrint = Dst - Src;
            if(Dst > Src)
            {
                for(MD_u64 SpaceIdx = 0; SpaceIdx < SpacesToPrint; ++SpaceIdx)
                {
                    MD_S8ListPush(Arena, Output, MD_S8Lit(" "));
                }
            }
        }break;

        case TABLE_OP_CHECK_IF_TRUE:
        {
            MD_i64 CheckVal = TableExprEvaluateNumeric(Info, Expr->left);
            if(CheckVal)
                TableExprEvaluateString(Info, Expr->right, Output);
        }break;

        case TABLE_OP_CONCAT:
        {
            TableExprEvaluateString(Info, Expr->left, Output);
            TableExprEvaluateString(Info, Expr->right, Output);
        }break;
    }
}

void LoopExpansionDimension(expand_iter *Iter, expand_info *Info, MD_String8List *Output)
{
    MD_Arena *Arena = CodegenState->Arena;
    if(Iter->next)
    {
        for(MD_u64 Index = 0; Index < Iter->Count; ++Index)
        {
            Iter->Index = Index;
            LoopExpansionDimension(Iter->next, Info, Output);
        }
    }
    else
    {
        for(MD_u64 Index = 0; Index < Iter->Count; ++Index)
        {
            Iter->Index = Index;
            MD_String8List ExpansionStrings = {0};
            MD_u64 StartIndex = 0;
            
            for(MD_u64 CharIndex = 0; CharIndex <= Info->StrExpr.size; ++CharIndex)
            {
                MD_b32 IsExprMarker = Info->StrExpr.str[CharIndex] == '$';

                if(CharIndex == Info->StrExpr.size || IsExprMarker)
                {
                    MD_String8 NormalStringChunk = MD_S8Substring(Info->StrExpr, StartIndex, CharIndex);
                    MD_String8 Escaped = EscapedFromString(Arena, NormalStringChunk);
                    MD_S8ListPush(Arena, &ExpansionStrings, Escaped);
                }

                if(IsExprMarker)
                {
                    MD_String8 ExprString = MD_S8Skip(Info->StrExpr, CharIndex + 1);
                    {
                        MD_i64 ParenNest = 0;
                        for(MD_u64 ExprStrCharIndex = 0; ExprStrCharIndex < ExprString.size; ++ExprStrCharIndex)
                        {
                            if(ExprString.str[ExprStrCharIndex] == '(')
                                ++ParenNest;
                            else if(ExprString.str[ExprStrCharIndex] == ')')
                            {
                                --ParenNest;
                                if(ParenNest == 0)
                                {
                                    ExprString.size = ExprStrCharIndex + 1;
                                    break;
                                }
                            }
                        }
                    }

                    MD_ParseResult Parse = MD_ParseOneNode(Arena, ExprString, 0);
                    MD_Node *Node = Parse.node;
                    MD_ExprParseResult ExprParse = MD_ExprParse(Arena, &Info->ExprOprTable, Node->first_child, MD_NilNode());
                    MD_Expr *Expr = ExprParse.expr;
                    TableExprEvaluateString(Info, Expr, &ExpansionStrings);
                    MD_String8 ParsedString = MD_S8Substring(Info->StrExpr, CharIndex + 1, CharIndex + 1 + Parse.string_advance);
                    ParsedString = MD_S8ChopWhitespace(ParsedString);
                    StartIndex = CharIndex + 1 + ParsedString.size;
                }
            }

            MD_String8 ExpansionStr = MD_S8ListJoin(Arena, ExpansionStrings, 0);
            MD_S8ListPush(Arena, Output, ExpansionStr);
        }
    }
}

MD_String8List GenStringListFromNode(table_state *State, MD_ExprOprTable ExprOprTable, MD_Node *Gen)
{
    MD_Arena *Arena = CodegenState->Arena;
    MD_String8List Result = {0};
    MD_ArenaTemp Scratch = MD_GetScratch(0, 0);

    for(MD_EachNode(StrExpr, Gen->first_child))
    {
        expand_iter *FirstIter = 0;
        expand_iter *LastIter = 0;
        for(MD_EachNode(Tag, StrExpr->first_tag))
        {
            if(MD_S8Match(Tag->string, MD_S8Lit("expand"), 0))
            {
                MD_Node *TableNameNode = MD_ChildFromIndex(Tag, 0);
                MD_Node *LabelNode = MD_ChildFromIndex(Tag, 1);
                MD_String8 TableName = TableNameNode->string;
                MD_String8 Label = LabelNode->string;

                // Grab the table associated with the table_name
                node_grid *Grid = 0;
                {
                    MD_MapSlot *Slot = MD_MapLookup(&State->TopLevelNodeGridMap, MD_MapKeyStr(TableName));
                    if(Slot)
                        Grid = Slot->val;
                }

                // Grab the table header associated with the table_name
                table_header *Header = 0;
                {
                    MD_MapSlot *Slot = MD_MapLookup(&State->TopLevelTableHeaderMap, MD_MapKeyStr(TableName));
                    if(Slot)
                        Header = Slot->val;
                }

                // Make iterator node if we've got a grid
                if(Grid)
                {
                    expand_iter *Iter = MD_PushArrayZero(Scratch.arena, expand_iter, 1);
                    MD_QueuePush(FirstIter, LastIter, Iter);
                    Iter->Grid = Grid;
                    Iter->Header = Header;
                    Iter->Label = Label;
                    Iter->Count = Grid->RowParents.Count;
                }

                if(Grid == 0)
                    MD_PrintMessageFmt(stderr, MD_CodeLocFromNode(Tag), MD_MessageKind_Error, "Table \"%S\" was not found.", TableName);
            }
        }

        if(FirstIter)
        {
            expand_info Info = {0};
            Info.StrExpr = StrExpr->string;
            Info.FirstExpandIter = FirstIter;
            Info.ExprOprTable = ExprOprTable;
            LoopExpansionDimension(FirstIter, &Info, &Result);
        }
        else
        {
            MD_String8 Escaped = EscapedFromString(Arena, StrExpr->string);
            MD_S8ListPush(Arena, &Result, Escaped);
        }
    }

    MD_ReleaseScratch(Scratch);
    return Result;
}

void TableGenerate(MD_Node *FileList)
{
    MD_Arena *Arena = CodegenState->Arena;
    table_state State = {0};

    State.TopLevelNodeGridMap = MD_MapMake(Arena);
    State.TopLevelTableHeaderMap = MD_MapMake(Arena);
    State.LayerMapGen = MD_MapMake(Arena);
    State.LayerMapGenEnum = MD_MapMake(Arena);
    State.LayerMapGenData = MD_MapMake(Arena);

    MD_ExprOprList OpsList = {0};
    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Binary, 10, MD_S8Lit("."), TABLE_OP_DOT, 0);
    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Prefix, 9, MD_S8Lit("=>"), TABLE_OP_BUMP, 0);
    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Binary, 6, MD_S8Lit("??"), TABLE_OP_CHECK_IF_TRUE, 0);
    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Binary, 7, MD_S8Lit(".."), TABLE_OP_CONCAT, 0);

    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Binary, 8, MD_S8Lit("=="), TABLE_OP_EQUAL, 0);
    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Binary, 8, MD_S8Lit("!="), TABLE_OP_IS_NOT_EQUAL, 0);
    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Binary, 5, MD_S8Lit("&&"), TABLE_OP_BOOLEAN_AND, 0);
    MD_ExprOprPush(Arena, &OpsList, MD_ExprOprKind_Binary, 4, MD_S8Lit("||"), TABLE_OP_BOOLEAN_OR, 0);
    MD_ExprOprTable TableExprOpTable = MD_ExprBakeOprTableFromList(Arena, &OpsList);

    // Gather phase
    for(MD_EachNode(FileRef, FileList->first_child))
    {
        MD_Node *File = MD_ResolveNodeFromReference(FileRef);
        MD_String8 LayerName = File->string;
        MD_MapKey LayerKey = MD_MapKeyStr(LayerName);
        
        for(MD_EachNode(Node, File->first_child))
        {
            MD_Node *TableTag = MD_TagFromString(Node, TableTagTable, 0);
            if(!MD_NodeIsNil(TableTag))
            {
                node_grid *Grid = MD_PushArrayZero(Arena, node_grid, 1);
                *Grid = GridFromNode(Node);
                MD_MapOverwrite(Arena, &State.TopLevelNodeGridMap, MD_MapKeyStr(Node->string), Grid);
                table_header *Header = MD_PushArrayZero(Arena, table_header, 1);
                *Header = TableHeaderFromTag(TableTag);
                MD_MapOverwrite(Arena, &State.TopLevelTableHeaderMap, MD_MapKeyStr(Node->string), Header);
            }
            
            if(MD_NodeHasTag(Node, TableTagTableGen, 0))
                MD_MapInsert(Arena, &State.LayerMapGen, LayerKey, Node);
            
            if(MD_NodeHasTag(Node, TableTagTableGenEnum, 0))
                MD_MapInsert(Arena, &State.LayerMapGenEnum, LayerKey, Node);
            
            if(MD_NodeHasTag(Node, TableTagTableGenData, 0))
                MD_MapInsert(Arena, &State.LayerMapGenData, LayerKey, Node);
        }
    }

    // Generate data tables
    for(MD_EachNode(FileRef, FileList->first_child))
    {
        MD_Node *File = MD_ResolveNodeFromReference(FileRef);
        MD_String8 LayerName = File->string;
        MD_MapKey LayerKey = MD_MapKeyStr(LayerName);

        for(MD_MapSlot *Slot = MD_MapLookup(&State.LayerMapGenEnum, LayerKey); Slot; Slot = MD_MapScan(Slot->next, LayerKey))
        {
            MD_Node *Gen = (MD_Node *)Slot->val;
            file_pair FilePair = FilePairFromNode(Gen);
            fprintf(FilePair.H, "typedef enum %.*s\n{\n", MD_S8VArg(Gen->string));
            MD_String8List GenStrings = GenStringListFromNode(&State, TableExprOpTable, Gen);
            MD_StringJoin Join = {MD_S8Lit(""), MD_S8Lit("\n"), MD_S8Lit("")};
            MD_String8 GenString = MD_S8ListJoin(Arena, GenStrings, &Join);
            fprintf(FilePair.H, "%.*s", MD_S8VArg(GenString));
            fprintf(FilePair.H, "\n}\n%.*s;\n\n", MD_S8VArg(Gen->string));
        }

        for(MD_MapSlot *Slot = MD_MapLookup(&State.LayerMapGenData, LayerKey); Slot; Slot = MD_MapScan(Slot->next, LayerKey))
        {
            MD_Node *Gen = (MD_Node *)Slot->val;
            MD_Node *Tag = MD_TagFromString(Gen, TableTagTableGenData, 0);
            MD_Node *DataTableTypeNode = Tag->first_child;
            MD_String8 DataTableType = DataTableTypeNode->string;
            MD_String8List GenStrings = GenStringListFromNode(&State, TableExprOpTable, Gen);
            MD_String8 HDeclSpecifier = MD_S8Lit("extern");
            
            file_pair FilePair = FilePairFromNode(Gen);
            fprintf(FilePair.H, "%.*s %.*s %.*s[%llu];\n\n", MD_S8VArg(HDeclSpecifier), MD_S8VArg(DataTableType),
                    MD_S8VArg(Gen->string), GenStrings.node_count);
            fprintf(FilePair.C, "%.*s %.*s[%llu] =\n{\n", MD_S8VArg(DataTableType), MD_S8VArg(Gen->string), GenStrings.node_count);
            MD_StringJoin Join = {MD_S8Lit(""), MD_S8Lit("\n"), MD_S8Lit("")};
            MD_String8 GenString = MD_S8ListJoin(Arena, GenStrings, &Join);
            fprintf(FilePair.C, "%.*s", MD_S8VArg(GenString));
            fprintf(FilePair.C, "\n};\n");
            fprintf(FilePair.C, "\n");
        }
        
        for(MD_MapSlot *Slot = MD_MapLookup(&State.LayerMapGen, LayerKey); Slot; Slot = MD_MapScan(Slot->next, LayerKey))
        {
            MD_Node *Gen = (MD_Node *)Slot->val;
            file_pair FilePair = FilePairFromNode(Gen);
            FILE *File = MD_NodeHasTag(Gen, MD_S8Lit("c"), 0) ? FilePair.C : FilePair.H;
            MD_String8List GenStrings = GenStringListFromNode(&State, TableExprOpTable, Gen);
            MD_StringJoin Join = {MD_S8Lit(""), MD_S8Lit("\n"), MD_S8Lit("")};
            MD_String8 GenString = MD_S8ListJoin(Arena, GenStrings, &Join);
            fprintf(File, "%.*s", MD_S8VArg(GenString));
            fprintf(File, "\n\n");
        }
    }
}
