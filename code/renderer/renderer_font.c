/*  =======================================================================
    File: renderer_font.c
    Date: March 12th 2024  6:05 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

typedef struct font_state font_state;
struct font_state
{
    FT_Library Freetype;

    font *FirstFreeFont;

    U64 HashToRasterSlotsCount;
    font_hash_to_info_raster_cache_slot *HashToRasterSlots;

    font_atlas *FirstAtlas;
    font_atlas *LastAtlas;
    
    arena *Arena;
};

typedef struct font_hash_data font_hash_data;
struct font_hash_data
{
    U64 Glyph;
    U32 Size;
    font *Font;
};

global_variable font_state *FontState = 0;

void FontStartUp()
{
    arena *Arena = ArenaAlloc(Megabytes(4));
    FontState = PushStruct(Arena, font_state);
    FontState->Arena = Arena;

    FontState->HashToRasterSlotsCount = 32768;
    FontState->HashToRasterSlots = PushArray(Arena, font_hash_to_info_raster_cache_slot, FontState->HashToRasterSlotsCount);

    FT_Init_FreeType(&FontState->Freetype);
}

font_handle FontOpen(string8 Path)
{
    font *Font = 0;
    
    os_handle FontFile = OSFileOpen(OS_ACCESS_FLAG_READ, Path);
    if(FontFile)
    {
        os_file_properties FileProperties = OSPropertiesFromFile(FontFile);
    
        if(FontState->FirstFreeFont)
        {
            Font = FontState->FirstFreeFont;
            SLLStackPop(FontState->FirstFreeFont);
            MemoryZero(Font, sizeof(font));
        }
        else
            Font = PushStruct(FontState->Arena, font);

        // NOTE: If the fonts will ever be released then we need a different allocation scheme
        void *FontData = ArenaPush(FontState->Arena, FileProperties.Size);
        OSFileRead(FontFile, 0, FileProperties.Size, FontData);
        OSFileClose(FontFile);
    
        // Get FreeType font stuff
        FT_Face Face;
        if(FT_New_Memory_Face(FontState->Freetype, FontData, (FT_Long)FileProperties.Size, 0, &Face))
            Error("FreeType failed to load the requested font from file: %.*s\n", Str8VArg(Path));


        // Get HarfBuzz font stuff
        hb_blob_t *Blob = hb_blob_create(FontData, (unsigned int)FileProperties.Size, HB_MEMORY_MODE_READONLY, 0, 0);
        hb_face_t *HarfFace = hb_face_create(Blob, 0);
        hb_font_t *HarfFont = hb_font_create(HarfFace);
    
        Font->Face = Face;
        Font->HarfFace = HarfFace;
        Font->HarfFont = HarfFont;
    }
    else
    {
        Error("Failed to load font file from path: %.*s\n", Str8VArg(Path));
    }

    return (font_handle)Font;
}

font_metrics FontMetricsFromFontSize(font_handle FontHandle, F32 Size)
{
    font *Font = (font *)FontHandle;
    FT_Face Face = Font->Face;
    FT_Set_Pixel_Sizes(Face, 0, (FT_UInt)((96.0f / 72.0f) * Size));
    FT_Size_Metrics Metrics = Face->size->metrics;
    
    
    font_metrics Result = {0};
    Result.LineGap = (Metrics.height - (Metrics.ascender + Metrics.descender)) / 64.0f;
    Result.CapHeight = (Metrics.ascender - Metrics.descender) / 64.0f;

    return Result;
}

font_raster_result FontRasterGlyph(arena *Arena, font_handle FontHandle, F32 Size, U64 Glyph)
{
    font *Font = (font *)FontHandle;
    FT_Face Face = Font->Face;
    FT_Set_Pixel_Sizes(Face, 0, (FT_UInt)((96.0f / 72.0f) * Size));

    vec2_i16 AtlasDimensions = {0};
    vec2_i16 Bearing = {0};

    // Load and raster the glyph
    FT_Load_Glyph(Face, (FT_UInt)Glyph, FT_LOAD_DEFAULT | FT_LOAD_TARGET_LIGHT);
    FT_Render_Glyph(Face->glyph, FT_RENDER_MODE_LIGHT);
    AtlasDimensions.X = Face->glyph->bitmap.width;
    AtlasDimensions.Y = Face->glyph->bitmap.rows;

    Bearing.X = (I16)Face->glyph->bitmap_left;
    Bearing.Y = (I16)Face->glyph->bitmap_top;
    
    Assert(Face->glyph->format == FT_GLYPH_FORMAT_BITMAP);

    // Return the rastered result
    font_raster_result Result = {0};
    U64 AtlasSize = AtlasDimensions.X * AtlasDimensions.Y;
    Result.Atlas = PushArray(Arena, U8, AtlasSize);
    MemoryCopy(Result.Atlas, Face->glyph->bitmap.buffer, AtlasSize);
    Result.AtlasDimensions = AtlasDimensions;
    Result.Bearing = Bearing;
    
    return Result;
}

rng2_i16 FontAtlasRegionAlloc(font_atlas *Atlas, vec2_i16 NeededDimensions)
{
    rng2_i16 Result = {0};

    if((Atlas->Shelf + NeededDimensions.Y) <= Atlas->Dimensions.Y)
    {
        if((Atlas->Right + NeededDimensions.X) > Atlas->Dimensions.X)
        {
            Atlas->Right = 0;
            Atlas->Shelf += Atlas->ShelfHeight;
            Atlas->ShelfHeight = 0;
        }
        
        Atlas->ShelfHeight = Max(Atlas->ShelfHeight, NeededDimensions.Y);
        
        Result.Left = Atlas->Right;
        Result.Right = Result.Left + NeededDimensions.X;
        Result.Top = Atlas->Shelf;
        Result.Bottom = Result.Top + NeededDimensions.Y;

        Atlas->Right += NeededDimensions.X;
    }

    return Result;
}

font_run FontPushRunFromString(arena *Arena, font_handle FontHandle, F32 Size, string8 String)
{
    font *Font = (font *)FontHandle;
    
    hb_buffer_t *Buffer = hb_buffer_create();
    hb_buffer_add_utf8(Buffer, String.String, (int)String.Size, 0, -1);
    hb_buffer_guess_segment_properties(Buffer);
    hb_font_set_scale(Font->HarfFont, (I32)(((96.0f / 72.0f) * Size) * 64.0f), (I32)(((96.0f / 72.0f) * Size) * 64.0f));

    hb_shape(Font->HarfFont, Buffer, 0, 0);

    font_piece_list Pieces = {0};
    vec2_f32 Dimensions = {0};
    F32 Ascent = 0;
    F32 Descent = 0;

    U64 MaxHeight = 0;
    U32 GlyphCount;
    hb_glyph_info_t *GlyphInfo = hb_buffer_get_glyph_infos(Buffer, &GlyphCount);
    hb_glyph_position_t *GlyphPos = hb_buffer_get_glyph_positions(Buffer, &GlyphCount);
    for(U32 Glyph = 0; Glyph < GlyphCount; ++Glyph, ++Pieces.Count)
    {
        temp_arena Scratch = GetScratch(&Arena, 1);
        
        // Get the info for the font piece
        U64 GlyphID = GlyphInfo[Glyph].codepoint;

        font_raster_cache_info *Info = 0;
        {
            // Look for already rasterized glyph in the cache
            font_hash_data FontHashData = {GlyphID, (U32)(((96.0f / 72.0f) * Size) * 64.0f), Font};
            U64 GlyphHash = HashStruct(&FontHashData);
            
            U64 SlotIndex = GlyphHash % FontState->HashToRasterSlotsCount;
            font_hash_to_info_raster_cache_slot *Slot = &FontState->HashToRasterSlots[SlotIndex];
            for(font_hash_to_info_raster_cache_node *Node = Slot->First; Node; Node = Node->Next)
            {
                if(Node->Hash == GlyphHash)
                {
                    Info = &Node->Info;
                    break;
                }
            }

            // No glyph found in the cache. Rasterize it and put it in the cache
            if(Info == 0)
            {
                font_hash_to_info_raster_cache_node *Node = PushStruct(FontState->Arena, font_hash_to_info_raster_cache_node);
                SLLQueuePush(Slot->First, Slot->Last, Node);
                Node->Hash = GlyphHash;
                Info = &Node->Info;

                font_raster_result RasterResult = FontRasterGlyph(Scratch.Arena, FontHandle, Size, GlyphID);

                font_atlas *ChosenAtlas = 0;
                rng2_i16 ChosenAtlasRegion = {0};
                if(RasterResult.AtlasDimensions.X && RasterResult.AtlasDimensions.Y)
                {
                    if(RasterResult.AtlasDimensions.X && RasterResult.AtlasDimensions.Y)
                    {
                        I16 NumAtlases = 0;
                        for(font_atlas *Atlas = FontState->FirstAtlas;; Atlas = Atlas->Next)
                        {
                            if(Atlas == 0)
                            {
                                Atlas = PushStruct(FontState->Arena, font_atlas);
                                DLLPushBack(FontState->FirstAtlas, FontState->LastAtlas, Atlas);
                                Atlas->Dimensions = Vec2I16(1024, 1024);
                                Atlas->Texture = RndTex2DAlloc(RND_TEX2D_KIND_LONG_LIVED, Vec2I32(Atlas->Dimensions.X, Atlas->Dimensions.Y),
                                                               RND_TEX2D_FORMAT_A8, 0);
                            }

                            if(Atlas)
                            {
                                ChosenAtlasRegion = FontAtlasRegionAlloc(Atlas, RasterResult.AtlasDimensions);
                                if(ChosenAtlasRegion.Left != ChosenAtlasRegion.Right)
                                {
                                    ChosenAtlas = Atlas;
                                    break;
                                }
                            }
                        }
                    }

                    if(ChosenAtlas)
                    {
                        RndFillTex2DRegion(ChosenAtlas->Texture, Rng2I32(Vec2I32(ChosenAtlasRegion.Left, ChosenAtlasRegion.Top),
                                                                         Vec2I32(ChosenAtlasRegion.Right, ChosenAtlasRegion.Bottom)),
                                           RasterResult.Atlas);
                    }
                }

                Info->Texture = ChosenAtlas ? ChosenAtlas->Texture : 0;
                Info->Subrect = ChosenAtlasRegion;
                Info->Bearing = RasterResult.Bearing;
            }
        }
        
        font_piece *Piece = PushStruct(Arena, font_piece);
        SLLQueuePush(Pieces.First, Pieces.Last, Piece);

        Piece->Texture = Info->Texture;
        Piece->Advance = (F32)(GlyphPos[Glyph].x_advance / 64);
        Piece->Offset = Vec2F32((F32)GlyphPos[Glyph].x_offset / 64.0f, (F32)GlyphPos[Glyph].y_offset / 64.0f);
        Piece->Offset.X += Info->Bearing.X;
        Piece->Offset.Y -= Info->Bearing.Y;
        Piece->Source = Rng2F32(Vec2F32(Info->Subrect.TopLeft.X, Info->Subrect.TopLeft.Y),
                                Vec2F32(Info->Subrect.BottomRight.X, Info->Subrect.BottomRight.Y));

        Dimensions.X += Piece->Advance;
        Dimensions.Y = (F32)(Info->Subrect.Bottom - Info->Subrect.Top);
        Ascent = Max(Ascent, Info->Bearing.Y);
        Descent = Max(Descent, Dimensions.Y - Info->Bearing.Y);
        
        ReleaseScratch(Scratch);
    }

    hb_buffer_destroy(Buffer);

    font_run Run = {0};
    Run.Pieces = Pieces;
    Run.Dimensions = Dimensions;
    Run.Ascent = Ascent;
    Run.Descent = Descent;
    return Run;
}
