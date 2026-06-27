/*  =======================================================================
    File: ui_view.c
    Date: April 11th 2024 10:02 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

void *UIViewGetOrPushUserState(ui_view *View, U64 Size)
{
    void *Result = View->UserData;
    if(Result == 0)
        View->UserData = Result = PushArray(View->Arena, U8, Size);
    return Result;
}
