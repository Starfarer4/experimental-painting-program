#if !defined(RENDERER_FONT_H)
/*  =======================================================================
    File: renderer_font.h
    Date: March 12th 2024  6:05 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define RENDERER_FONT_H

typedef U64 font_handle;

typedef struct font font;
struct font
{
    font *Next;
    
    FT_Face Face;
    
    hb_face_t *HarfFace;
    hb_font_t *HarfFont;
};

typedef struct font_raster_cache_info font_raster_cache_info;
struct font_raster_cache_info
{
    rnd_handle Texture;
    rng2_i16 Subrect;
    vec2_i16 Bearing;
};

typedef struct font_hash_to_info_raster_cache_node font_hash_to_info_raster_cache_node;
struct font_hash_to_info_raster_cache_node
{
    font_hash_to_info_raster_cache_node *Next;
    U64 Hash;
    font_raster_cache_info Info;
};

typedef struct font_hash_to_info_raster_cache_slot font_hash_to_info_raster_cache_slot;
struct font_hash_to_info_raster_cache_slot
{
    font_hash_to_info_raster_cache_node *First;
    font_hash_to_info_raster_cache_node *Last;
};

typedef struct font_piece font_piece;
struct font_piece
{
    font_piece *Next;
    F32 Advance;
    vec2_f32 Offset;
    
    rng2_f32 Source;
    rnd_handle Texture;
};

typedef struct font_piece_list font_piece_list;
struct font_piece_list
{
    font_piece *First;
    font_piece *Last;
    U64 Count;
};

typedef struct font_run font_run;
struct font_run
{
    font_piece_list Pieces;
    vec2_f32 Dimensions;
    F32 Ascent;
    F32 Descent;
};

typedef struct font_raster_result font_raster_result;
struct font_raster_result
{
    void *Atlas;
    vec2_i16 AtlasDimensions;
    vec2_i16 Bearing;
};

typedef struct font_atlas font_atlas;
struct font_atlas
{
    font_atlas *Next;
    font_atlas *Prev;
    
    vec2_i16 Dimensions;
    rnd_handle Texture;

    I16 Shelf;
    I16 ShelfHeight;
    I16 Right;
};

typedef struct font_metrics font_metrics;
struct font_metrics
{
    F32 CapHeight;
    F32 LineGap;
};

void FontStartUp();
font_handle FontOpen(string8 Path);
font_metrics FontMetricsFromFontSize(font_handle FontHandle, F32 Size);
font_raster_result FontRasterGlyph(arena *Arena, font_handle FontHandle, F32 Size, U64 Glyph);
rng2_i16 FontAtlasRegionAlloc(font_atlas *Atlas, vec2_i16 NeededDimensions);
font_run FontPushRunFromString(arena *Arena, font_handle FontHandle, F32 Size, string8 String);

#endif
