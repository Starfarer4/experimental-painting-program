/*  =======================================================================
    File: brush_abr_reader.c
    Date: June 26th 2024 11:36 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

br_abr_brush_section_ptrs BRGetABRBrushSectionPtrs(U8 *Bytes, U8 *End)
{
    br_abr_brush_section_ptrs Result = {0};

    U32 PhotoshopSignature  = 0x3842494D; // 8BIM
    U32 SampleSectionID     = 0x73616D70; // samp
    U32 DescriptorSectionID = 0x64657363; // desc

    while(Bytes < End)
    {
        U32 Signature = ReadBE32(&Bytes);
        if(Signature != PhotoshopSignature)
            break;

        U32 SectionID = ReadBE32(&Bytes);
        if(SectionID == SampleSectionID)          Result.SampleSectionPtr = Bytes;
        else if(SectionID == DescriptorSectionID) Result.DescriptorSectionPtr = Bytes;

        U32 Size = ReadBE32(&Bytes);
        Bytes += Size;
    }

    return Result;
}

string8 BRParseABRKey(arena *Arena, U8 **Bytes)
{
    U32 Length = ReadBE32(Bytes);
    if(Length == 0) Length = 4;
    string8 String = {*Bytes, Length};
    *Bytes += Length;
    return PushStr8Fmt(Arena, Str8Lit("%.*s"), Str8VArg(String));
}

void *BRParseABRRawData(arena *Arena, U8 **Bytes)
{
    U32 Length = ReadBE32(Bytes);
    void *Result = PushArray(Arena, U8, Length);
    MemoryCopy(Result, Bytes, Length);
    *Bytes += Length;
    return Result;
}

string8 BRReadABRWString(arena *Arena, U8 **Bytes)
{
    U32 Length = ReadBE32(Bytes);
    string8 Result = {0};
    if(Length)
    {
        temp_arena Scratch = GetScratch(&Arena, 1);

        // Convert string from big endian to native endian (lol)
        string16 BigEndianWString = {(U16 *)(*Bytes), Length};
        string16 WString = {PushArray(Scratch.Arena, U16, Length), Length};
        for(U64 WChar = 0; WChar < Length; ++WChar)
        {
            U8 *Ptr = (U8 *)&BigEndianWString.String[WChar];
            WString.String[WChar] = (Ptr[0] << 8) | Ptr[1];
        }

        Result = Str8From16(Arena, WString);

        ReleaseScratch(Scratch);
    }
    *Bytes += Length * 2;
    return Result;
}

void BRParseABRList(arena *Arena, br_abr_load_data *Loaded, U8 **Bytes)
{
    U32 Count = ReadBE32(Bytes);
    for(U32 Iter = 0; Iter < Count; ++Iter)
    {
        br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(Bytes);
        BRParseABRType(Arena, Loaded, Type, Bytes);
    }
}

br_abr_unit_float BRParseABRUnitFloat(U8 **Bytes)
{
    br_abr_unit_float Result = {0};
    Result.Type = (br_abr_unit_types)ReadBE32(Bytes);
    U64 ResultBytes = ReadBE64(Bytes);
    Result.Value = *((F64 *)&ResultBytes);
    return Result;
}

B8 BRParseABRBoolean(U8 **Bytes)
{
    B8 Value = ReadBE8(Bytes) != 0;
    return Value;
}

I32 BRParseABRInteger(U8 **Bytes)
{
    U32 ValueBytes = ReadBE32(Bytes);
    I32 Value = *((I32 *)&ValueBytes);
    return Value;
}

F64 BRParseABRFloat(U8 **Bytes)
{
    U64 ValueBytes = ReadBE64(Bytes);
    F64 Value = *((F64 *)&ValueBytes);
    return Value;
}

br_abr_enumerated_value BRParseABREnumerated(arena *Arena, U8 **Bytes)
{
    br_abr_enumerated_value Value = {0};
    Value.Type = BRParseABRKey(Arena, Bytes);
    Value.Value = BRParseABRKey(Arena, Bytes);
    return Value;
}

br_abr_brush_data BRParseABRSampledBrush(arena *Arena, br_abr_load_data *Loaded, U32 Count, U8 **Bytes)
{
    br_abr_brush_data Data = {0};
    Data.Type = BR_ABR_BRUSH_TYPE_SAMPLED;
    
    for(U32 Iter = 0; Iter < Count; ++Iter)
    {
        temp_arena Scratch = GetScratch(&Arena, 1);
        string8 Key = BRParseABRKey(Scratch.Arena, Bytes);
        br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(Bytes);

        if(Str8Match(Key, Str8Lit("Dmtr"), 0))
        {
            br_abr_unit_float Diameter = BRParseABRUnitFloat(Bytes);
            if(Diameter.Type == BR_ABR_UNIT_TYPES_PIXEL)
                Data.Diameter = Diameter.Value;
        }
        else if(Str8Match(Key, Str8Lit("Spcn"), 0))
        {
            br_abr_unit_float Spacing = BRParseABRUnitFloat(Bytes);
            if(Spacing.Type == BR_ABR_UNIT_TYPES_PERCENT)
                Data.Spacing = Spacing.Value;
        }
        else if(Str8Match(Key, Str8Lit("sampledData"), 0))
        {
            Data.SampledDataTag = BRReadABRWString(Arena, Bytes);
        }
        else
        {
            BRParseABRType(Arena, Loaded, Type, Bytes);
        }

        ReleaseScratch(Scratch);
    }

    return Data;
}

br_abr_brush_data BRParseABRComputedBrush(arena *Arena, br_abr_load_data *Loaded, U32 Count, U8 **Bytes)
{
    br_abr_brush_data Data = {0};
    Data.Type = BR_ABR_BRUSH_TYPE_COMPUTED;

    for(U32 Iter = 0; Iter < Count; ++Iter)
    {
        temp_arena Scratch = GetScratch(&Arena, 1);
        string8 Key = BRParseABRKey(Scratch.Arena, Bytes);
        br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(Bytes);

        if(Str8Match(Key, Str8Lit("Dmtr"), 0))
        {
            br_abr_unit_float Diameter = BRParseABRUnitFloat(Bytes);
            if(Diameter.Type == BR_ABR_UNIT_TYPES_PIXEL)
                Data.Diameter = Diameter.Value;
        }
        else if(Str8Match(Key, Str8Lit("Hrdn"), 0))
        {
            br_abr_unit_float Hardness = BRParseABRUnitFloat(Bytes);
            if(Hardness.Type == BR_ABR_UNIT_TYPES_PERCENT)
                Data.Hardness = Hardness.Value;
        }
        else if(Str8Match(Key, Str8Lit("Angl"), 0))
        {
            br_abr_unit_float Angle = BRParseABRUnitFloat(Bytes);
            if(Angle.Type == BR_ABR_UNIT_TYPES_ANGLE)
                Data.Angle = Angle.Value;
        }
        else if(Str8Match(Key, Str8Lit("Rndn"), 0))
        {
            br_abr_unit_float Roundness = BRParseABRUnitFloat(Bytes);
            if(Roundness.Type == BR_ABR_UNIT_TYPES_PERCENT)
                Data.Roundness = Roundness.Value;
        }
        else if(Str8Match(Key, Str8Lit("Spcn"), 0))
        {
            br_abr_unit_float Spacing = BRParseABRUnitFloat(Bytes);
            if(Spacing.Type == BR_ABR_UNIT_TYPES_PERCENT)
                Data.Spacing = Spacing.Value;
        }
        else if(Str8Match(Key, Str8Lit("flipX"), 0))
        {
            Data.FlipX = BRParseABRBoolean(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("flipY"), 0))
        {
            Data.FlipY = BRParseABRBoolean(Bytes);
        }
        else
        {
            BRParseABRType(Arena, Loaded, Type, Bytes);
        }
    }
    
    return Data;
}

br_abr_brush_data BRParseABRBrushDescriptor(arena *Arena, br_abr_load_data *Loaded, U8 **Bytes)
{
    temp_arena Scratch = GetScratch(&Arena, 1);

    string8 Name = BRReadABRWString(Scratch.Arena, Bytes);
    string8 ClassID = BRParseABRKey(Scratch.Arena, Bytes);
    U32 Count = ReadBE32(Bytes);

    br_abr_brush_data Data = {0};

    if(Str8Match(ClassID, Str8Lit("sampledBrush"), 0))
    {
        Data = BRParseABRSampledBrush(Arena, Loaded, Count, Bytes);
    }
    else if(Str8Match(ClassID, Str8Lit("computedBrush"), 0))
    {
        Data = BRParseABRComputedBrush(Arena, Loaded, Count, Bytes);
    }
    else if(Str8Match(ClassID, Str8Lit("dBrush"), 0))
    {
        goto needs_implementation;
    }
    else if(Str8Match(ClassID, Str8Lit("dTips"), 0))
    {
        goto needs_implementation;
    }
    else
    {
needs_implementation:
        Error("Brush of unknown type %.*s\n", Str8VArg(ClassID));
        
        for(U32 Iter = 0; Iter < Count; ++Iter)
        {
            string8 Key = BRParseABRKey(Scratch.Arena, Bytes);
            br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(Bytes);

            BRParseABRType(Arena, Loaded, Type, Bytes);
        }
    }

    ReleaseScratch(Scratch);

    return Data;
}

br_abr_variable_control BRParseABRControlDescriptor(arena *Arena, br_abr_load_data *Loaded, U8 **Bytes)
{
    temp_arena Scratch = GetScratch(&Arena, 1);
    string8 Name = BRReadABRWString(Scratch.Arena, Bytes);
    string8 ClassID = BRParseABRKey(Scratch.Arena, Bytes);
    U32 Count = ReadBE32(Bytes);
    
    br_abr_variable_control Result = {0};

    for(U32 Iter = 0; Iter < Count; ++Iter)
    {
        string8 Key = BRParseABRKey(Scratch.Arena, Bytes);
        br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(Bytes);

        if(Str8Match(Key, Str8Lit("bVTy"), 0))
        {
            Result.Type = (br_abr_control_type)BRParseABRInteger(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("fStp"), 0))
        {
            Result.FadeStep = BRParseABRInteger(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("jitter"), 0))
        {
            br_abr_unit_float Jitter = BRParseABRUnitFloat(Bytes);
            if(Jitter.Type == BR_ABR_UNIT_TYPES_PERCENT)
                Result.SizeJitter = Jitter.Value;
        }
        else if(Str8Match(Key, Str8Lit("Mnm "), 0))
        {
            br_abr_unit_float Minimum = BRParseABRUnitFloat(Bytes);
            if(Minimum.Type == BR_ABR_UNIT_TYPES_PERCENT)
                Result.Minimum = Minimum.Value;
        }
        else
        {
            BRParseABRType(Arena, Loaded, Type, Bytes);
        }
    }
    
    ReleaseScratch(Scratch);

    return Result;
}

inline br_control_type BRABRCtrlTypeToBRCtrlType(br_abr_control_type Type)
{
    br_control_type Result = 0;
    switch(Type)
    {
        default:
        case BR_ABR_CONTROL_TYPE_OFF:
        {
            Result = BR_CONTROL_TYPE_OFF;
        }break;

        case BR_ABR_CONTROL_TYPE_PEN_PRESSURE:
        {
            Result = BR_CONTROL_TYPE_PEN_PRESSURE;
        }break;
    }
    return Result;
}

void BRParseABRBrushPreset(arena *Arena, br_abr_load_data *Loaded, U32 Count, U8 **Bytes)
{
    br_abr_brush_data BrushData = {0};
    br_abr_tip_dynamics_data TipDynamicsData = {0};
    br_abr_paint_dynamics_data PaintDynamicsData = {0};
    string8 PresetName = {0};

    for(U32 Preset = 0; Preset < Count; ++Preset)
    {
        temp_arena Scratch = GetScratch(&Arena, 1);

        string8 Key = BRParseABRKey(Scratch.Arena, Bytes);
        br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(Bytes);

        if(Str8Match(Key, Str8Lit("Nm  "), 0))
            PresetName = BRReadABRWString(Arena, Bytes);
        else if(Type == BR_ABR_DESCRIPTOR_TYPE_DESCRIPTOR && Str8Match(Key, Str8Lit("Brsh"), 0))
        {
            BrushData = BRParseABRBrushDescriptor(Arena, Loaded, Bytes);
        }
        
        // NOTE: Tip dynamics
        else if(Str8Match(Key, Str8Lit("useTipDynamics"), 0))
        {
            TipDynamicsData.Enabled = BRParseABRBoolean(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("flipX"), 0))
        {
            TipDynamicsData.FlipXJitter = BRParseABRBoolean(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("flipY"), 0))
        {
            TipDynamicsData.FlipYJitter = BRParseABRBoolean(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("brushProjection"), 0))
        {
            TipDynamicsData.BrushProjection = BRParseABRBoolean(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("minimumDiameter"), 0))
        {
            br_abr_unit_float MinimumDiameter = BRParseABRUnitFloat(Bytes);
            if(MinimumDiameter.Type == BR_ABR_UNIT_TYPES_PERCENT)
                TipDynamicsData.MinimumDiameter = MinimumDiameter.Value;
        }
        else if(Str8Match(Key, Str8Lit("szVr"), 0))
        {
            TipDynamicsData.SizeControl = BRParseABRControlDescriptor(Arena, Loaded, Bytes);
        }
        
        // NOTE: Paint dynamics
        else if(Str8Match(Key, Str8Lit("usePaintDynamics"), 0))
        {
            PaintDynamicsData.Enabled = BRParseABRBoolean(Bytes);
        }
        else if(Str8Match(Key, Str8Lit("prVr"), 0))
        {
            PaintDynamicsData.FlowJitter = BRParseABRControlDescriptor(Arena, Loaded, Bytes);
        }
        else if(Str8Match(Key, Str8Lit("opVr"), 0))
        {
            PaintDynamicsData.OpacityJitter = BRParseABRControlDescriptor(Arena, Loaded, Bytes);
        }
        else
            BRParseABRType(Arena, Loaded, Type, Bytes);

        ReleaseScratch(Scratch);
    }

    if(BrushData.Type != BR_ABR_BRUSH_TYPE_NONE)
    {
        Log("Parsed brush: %.*s\n", Str8VArg(PresetName));
        
        br_brush_node *Node = PushStruct(Arena, br_brush_node);
        br_brush *Brush = &Node->Brush;

        Brush->TipKind = BR_TIP_KIND_COMPUTED;

        Brush->Name = PresetName;
        Brush->SizePx = (F32)BrushData.Diameter;
        Brush->SpacingPct = (F32)BrushData.Spacing / 100.0f;
        Brush->ComputedTip.Hardness = (F32)BrushData.Hardness / 100.0f;
        Brush->ComputedTip.Angle = (F32)BrushData.Angle;
        Brush->ComputedTip.Roundness = (F32)BrushData.Roundness / 100.0f;
        Brush->ComputedTip.FlipX = BrushData.FlipX;
        Brush->ComputedTip.FlipY = BrushData.FlipY;

        // Shape dynamics
        Brush->ShapeDynamics.Enabled = TipDynamicsData.Enabled;
        Brush->ShapeDynamics.SizeJitter = (F32)TipDynamicsData.SizeJitter;
        Brush->ShapeDynamics.SizeControl = BRABRCtrlTypeToBRCtrlType(TipDynamicsData.SizeControl.Type);
        Brush->ShapeDynamics.MinimumDiameter = (F32)TipDynamicsData.MinimumDiameter;

        // Transfer
        Brush->Transfer.Enabled = PaintDynamicsData.Enabled;
        Brush->Transfer.OpacityJitter = (F32)PaintDynamicsData.OpacityJitter.SizeJitter;
        Brush->Transfer.OpacityControl = BRABRCtrlTypeToBRCtrlType(PaintDynamicsData.OpacityJitter.Type);
        Brush->Transfer.OpacityControlMinimum = (F32)PaintDynamicsData.OpacityJitter.Minimum;
        Brush->Transfer.FlowJitter = (F32)PaintDynamicsData.FlowJitter.SizeJitter;
        Brush->Transfer.FlowControl = BRABRCtrlTypeToBRCtrlType(PaintDynamicsData.FlowJitter.Type);
        Brush->Transfer.FlowControlMinimum = (F32)PaintDynamicsData.FlowJitter.Minimum;

        DLLPushBack(Loaded->First, Loaded->Last, Node);
    }
}

void BRParseABRDescriptor(arena *Arena, br_abr_load_data *Loaded, U8 **Bytes)
{
    temp_arena Scratch = GetScratch(&Arena, 1);

    string8 Name = BRReadABRWString(Scratch.Arena, Bytes);
    string8 ClassID = BRParseABRKey(Scratch.Arena, Bytes);

    U32 Count = ReadBE32(Bytes);

    if(Str8Match(ClassID, Str8Lit("brushPreset"), 0))
    {
        BRParseABRBrushPreset(Arena, Loaded, Count, Bytes);
    }
    else
    {
        for(U32 Iter = 0; Iter < Count; ++Iter)
        {
            string8 Key = BRParseABRKey(Scratch.Arena, Bytes);
            br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(Bytes);
            BRParseABRType(Scratch.Arena, Loaded, Type, Bytes);
        }
    }

    ReleaseScratch(Scratch);
}

void BRParseABRType(arena *Arena, br_abr_load_data *Loaded, br_abr_descriptor_types Type, U8 **Bytes)
{
    temp_arena Scratch = GetScratch(&Arena, 1);
    switch(Type)
    {
        case BR_ABR_DESCRIPTOR_TYPE_LIST: BRParseABRList(Arena, Loaded, Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_DESCRIPTOR: BRParseABRDescriptor(Arena, Loaded, Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_STRING: BRReadABRWString(Scratch.Arena, Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_UNIT_FLOAT: BRParseABRUnitFloat(Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_BOOLEAN: BRParseABRBoolean(Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_INTEGER: BRParseABRInteger(Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_FLOAT: BRParseABRFloat(Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_ENUMERATED: BRParseABREnumerated(Scratch.Arena, Bytes); break;
        case BR_ABR_DESCRIPTOR_TYPE_RAW_DATA: BRParseABRRawData(Scratch.Arena, Bytes); break;
        default: Assert(!"Unsupported descriptor type in file! This should not be reached...");
    }
    ReleaseScratch(Scratch);
}

void BRParseABRDescriptorSection(arena *Arena, br_abr_load_data *Loaded, U8 *Bytes)
{
    U32 SectionSize = ReadBE32(&Bytes);
    U8 *End = Bytes + SectionSize;
    Bytes += 22; // Skip junk data

    if(Bytes < End)
    {
        temp_arena Scratch = GetScratch(&Arena, 1);
        string8 Key = BRParseABRKey(Scratch.Arena, &Bytes);
        ReleaseScratch(Scratch);

        br_abr_descriptor_types Type = (br_abr_descriptor_types)ReadBE32(&Bytes);
        BRParseABRType(Arena, Loaded, Type, &Bytes);
    }
}

void BRLoadFromABR6(arena *Arena, br_abr_load_data *Loaded, U16 VersionMinor, U8 *Bytes, U8 *End)
{
    U32 UnusedDataLength = 0;
    switch(VersionMinor)
    {
        case 1: UnusedDataLength = 10;  break;
        case 2: UnusedDataLength = 264; break;
        default: Error("Attempted to load an ABR brush file from an unsupported version!\n"); break;
    }

    if(UnusedDataLength)
    {
        br_abr_brush_section_ptrs SectionPtrs = BRGetABRBrushSectionPtrs(Bytes, End);
        if(SectionPtrs.DescriptorSectionPtr) BRParseABRDescriptorSection(Arena, Loaded, SectionPtrs.DescriptorSectionPtr);
    }
}

void BRLoadFromABR(arena *Arena, U8 *Bytes, U64 Size, br_brush_list *List)
{
    U8 *End = Bytes + Size;

    U16 VersionMajor = ReadBE16(&Bytes);
    U16 VersionMinor = ReadBE16(&Bytes);

    br_abr_load_data LoadedData = {0};
    
    switch(VersionMajor)
    {
        case 1:
        case 2:
        {
        }break;

        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        {
            BRLoadFromABR6(Arena, &LoadedData, VersionMinor, Bytes, End);
        }break;

        default:
        {
            Error("Attempted to load an ABR brush file from an unsupported version!\n");
        }break;
    }

    U64 Count = 0;
    for(br_brush_node *Brush = LoadedData.First; Brush; Brush = Brush->Next) ++Count;
    
    List->First = LoadedData.First;
    List->Last = LoadedData.Last;
    List->Count = Count;
}
