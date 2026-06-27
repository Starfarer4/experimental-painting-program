#if !defined(QUALIA_VIEWS_H)
/*  =======================================================================
    File: qualia_views.h
    Date: April 11th 2024 10:14 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define QUALIA_VIEWS_H

#include "generated/qualia_views.meta.h"

void GFViewsStartUp();
ui_view *GFCreateUIView(gf_view_kind Kind);
void GFDeleteUIView(ui_view *View);

#endif
