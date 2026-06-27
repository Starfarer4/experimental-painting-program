/*  =======================================================================
    File: renderer.c
    Date: March 13th 2024  6:09 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

U8 RndPassKindBatchTable[2] = {1, 1};
U64 RndPassKindParamsSizeTable[2] = {sizeof(rnd_pass_params_ui), sizeof(rnd_pass_params_checkerboard)};

rnd_pass *RndPassFromKind(arena *Arena, rnd_pass_list *List, rnd_pass_kind Kind)
{
    rnd_pass *Node = List->Last;
    if(!RndPassKindBatchTable[Kind])
        Node = 0;
    if(Node == 0 || Node->Kind != Kind)
    {
        Node = PushStruct(Arena, rnd_pass);
        SLLQueuePush(List->First, List->Last, Node);
        ++List->Count;
        Node->Kind = Kind;
        Node->Params = PushArray(Arena, U8, RndPassKindParamsSizeTable[Kind]);
    }
    return Node;
}
