#if !defined(CODEGEN_TABLE_H)
/*  =======================================================================
    File: codegen_table.h
    Date: March 27th 2024  3:49 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define CODEGEN_TABLE_H


typedef enum table_op
{
    TABLE_OP_NULL,
    
    TABLE_OP_BEGIN_STRING_OPS,
    TABLE_OP_DOT,
    TABLE_OP_BUMP,
    TABLE_OP_CHECK_IF_TRUE,
    TABLE_OP_CONCAT,
    TABLE_OP_END_STRING_OPS,

    TABLE_OP_BEGIN_NUMERIC_OPS,
    TABLE_OP_EQUAL,
    TABLE_OP_IS_NOT_EQUAL,
    TABLE_OP_BOOLEAN_AND,
    TABLE_OP_BOOLEAN_OR,
    TABLE_OP_END_NUMERIC_OPS,
    
    TABLE_OP_COUNT
}table_op;

typedef struct node_array node_array;
struct node_array
{
    MD_u64 Count;
    MD_Node **v;
};

typedef struct node_grid node_grid;
struct node_grid
{
    node_array Cells;
    node_array RowParents;
};

typedef enum column_kind
{
    COLUMN_KIND_DEFAULT,
    COLUMN_KIND_CHECK_FOR_TAG,
    
    COLUMN_KIND_COUNT
}column_kind;

typedef struct column_desc column_desc;
struct column_desc
{
    column_kind Kind;
    MD_String8 Name;
    MD_String8 TagString;
};

typedef struct table_header table_header;
struct table_header
{
    MD_u64 ColumnCount;
    column_desc *ColumnDescs;
};

typedef struct expand_iter expand_iter;
struct expand_iter
{
    expand_iter *next;
    node_grid *Grid;
    table_header *Header;
    MD_String8 Label;
    MD_u64 Index;
    MD_u64 Count;
};

typedef struct expand_info expand_info;
struct expand_info
{
    MD_String8 StrExpr;
    expand_iter *FirstExpandIter;
    MD_ExprOprTable ExprOprTable;
};

typedef struct table_state table_state;
struct table_state
{
    MD_Map TopLevelNodeGridMap;
    MD_Map TopLevelTableHeaderMap;

    MD_Map LayerMapGen;
    MD_Map LayerMapGenEnum;
    MD_Map LayerMapGenData;
};

node_array NodeArrayMake(MD_u64 Count);
node_grid GridFromNode(MD_Node *Node);
table_header TableHeaderFromTag(MD_Node *Tag);
MD_u64 RowChildIndexFromColumnName(table_header *Header, MD_String8 ColumnName);
MD_i64 TableExprEvaluateNumeric(expand_info *Info, MD_Expr *Expr);
void TableExprEvaluateString(expand_info *Info, MD_Expr *Expr, MD_String8List *Output);
void LoopExpansionDimension(expand_iter *Iter, expand_info *Info, MD_String8List *Output);
MD_String8List GenStringListFromNode(table_state *State, MD_ExprOprTable ExprOprTable, MD_Node *Gen);
void TableGenerate(MD_Node *FileList);


#endif
