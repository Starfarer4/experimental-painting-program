/*  =======================================================================
    File: base_math.c
    Date: March 8th 2024 12:32 PM
    Creator: Quinn Van De Keere
    =======================================================================*/

#include "generated/base_math.meta.c"

F32 DotVector(F32 *Vector1, F32 *Vector2, U32 VectorSize)
{
    F32 Sum = 0.0f;
    for(U32 Element = 0; Element < VectorSize; ++Element)
        Sum += Vector1[Element] * Vector2[Element];
    return Sum;
}

F32 NormalizeVector(F32 *Vector, U32 VectorSize)
{
    F32 Sum = DotVector(Vector, Vector, VectorSize);
    return SqrtF32(Sum);
}

lbfgs_solver_params InitializeDefaultLBFGSParams()
{
    lbfgs_solver_params Result = {0};
    Result.MatrixSize = 6;
    Result.Epsilon = 1e-3f;
    Result.RelativeEpsilon = 1e-3f;
    Result.Past = 0;
    Result.Delta = 0.0f;
    Result.MaxIterations = 0;
    Result.Linesearch = LBFGS_LINESEARCH_BACKTRACKING_STRONG_WOLFE;
    Result.MaxLinesearch = 20;
    Result.MinStep = F32SmallestPositive;
    Result.MaxStep = F32Max;
    Result.LinesearchAccuracy = 1e-3f;
    Result.Wolfe = 0.9f;
    Result.UserData = 0;
    return Result;
}

void LineSearch(F32 *CurrentVector, F32 *CurrentMovingDirection, F32 StepMax, F32 *Step, F32 *FunctionValue, F32 *Gradient,
                F32 *DirGradient, F32 *NewVector, U32 Size, lbfgs_compute_function ComputeFunction, lbfgs_solver_params *Params)
{
    const F32 Decrease = 0.5f;
    const F32 Increase = 2.1f;

    Assert(*Step > 0.0f && "Step must be positive");

    // Save the function value at the current vector
    const F32 FunctionValueInit = *FunctionValue;
    // Projection of the gradient on the search direction
    const F32 InitialDirGradient = DotVector(Gradient, CurrentMovingDirection, Size);
    // Ensure direction points to a descent direction
    Assert(InitialDirGradient <= 0.0f && "The moving direction increases the objective function value");

    const F32 TestDecrease = Params->LinesearchAccuracy * InitialDirGradient;
    F32 Width = 0.0f;

    U32 Iter = 0;
    for(; Iter < Params->MaxLinesearch; ++Iter)
    {
        for(U32 Element = 0; Element < Size; ++Element)
            NewVector[Element] = CurrentVector[Element] + *Step * CurrentMovingDirection[Element];

        // Evaluate the candidate
        *FunctionValue = ComputeFunction(NewVector, Gradient, Size, Params->UserData);

        if(*FunctionValue > FunctionValueInit + *Step * TestDecrease || (*FunctionValue != *FunctionValue))
            Width = Decrease;
        else
        {
            *DirGradient = DotVector(Gradient, CurrentMovingDirection, Size);

            // Armijo condition is met
            if(Params->Linesearch == LBFGS_LINESEARCH_BACKTRACKING_ARMIJO)
                break;

            if(*DirGradient < Params->Wolfe * InitialDirGradient)
                Width = Increase;
            else
            {
                // Regular wolfe condition is met
                if(Params->Linesearch == LBFGS_LINESEARCH_BACKTRACKING_WOLFE)
                    break;

                if(*DirGradient > -Params->Wolfe * InitialDirGradient)
                    Width = Decrease;
                else
                    break; // Strong wolfe condition is met
            }
        }

        
        Assert(*Step >= Params->MinStep && "The line search step became smaller than the minimum value allowed");
        Assert(*Step <= Params->MaxStep && "The line search step became larger than the maximum value allowed");

        *Step *= Width;
    }

    Assert(Iter < Params->MaxLinesearch && "The line search routine reached the maximum number of iterations");
}

// `Vector` is the input for the initial guess and the output for the best point found
// `Size` is the size of the vector
// `Output` is the objective function value at x
// `ComputeFunction` is a function such that f(x, grad) returns the objective function value at x and overwrites grad with the gradient
// `Params` are the other parameters of this LBFGS function
U32 MinimizeLBFGS(F32 *Vector, U32 Size, F32 *Output, lbfgs_compute_function ComputeFunction, lbfgs_solver_params *Params)
{
    temp_arena Scratch = GetScratch(0, 0);

    F32 *Gradient = PushArray(Scratch.Arena, F32, Size);
    F32 *MovingDirection = PushArray(Scratch.Arena, F32, Size);
    F32 *OldVector = PushArray(Scratch.Arena, F32, Size);
    F32 *OldGradient = PushArray(Scratch.Arena, F32, Size);
    F32 *FunctionResults = PushArray(Scratch.Arena, F32, Size);

    F32 MatrixTheta = 1.0f;
    F32 *MatrixS = PushArray(Scratch.Arena, F32, Size * Params->MatrixSize);
    F32 *MatrixY = PushArray(Scratch.Arena, F32, Size * Params->MatrixSize);
    F32 *MatrixYHistory = PushArray(Scratch.Arena, F32, Params->MatrixSize);
    F32 *MatrixAlpha = PushArray(Scratch.Arena, F32, Params->MatrixSize);
    U32 MatrixCorrectionNumber = 0;
    U32 MatrixPointer = Params->MatrixSize; // This ensures that MatrixPointer % Params->MatrixSize == 0 in the first step

    // The length of lag for objective function value to test convergance
    U32 Past = Params->Past;
    
    *Output = ComputeFunction(Vector, Gradient, Size, Params->UserData);
    F32 NormGradient = NormalizeVector(Gradient, Size);
    if(Past > 0)
        FunctionResults[0] = *Output;

    U32 Iterations = 1;
    // Early exit if the initial vector is already a minimizer
    if(!(NormGradient <= Params->Epsilon || NormGradient <= Params->RelativeEpsilon * NormalizeVector(Vector, Size)))
    {
        // Initial direction
        for(U32 Element = 0; Element < Size; ++Element)
            MovingDirection[Element] = -Gradient[Element];
        // Initial step size
        F32 Step = 1.0f / NormalizeVector(MovingDirection, Size);
        // s and y vectors
        F32 *VectorS = PushArray(Scratch.Arena, F32, Size);
        F32 *VectorY = PushArray(Scratch.Arena, F32, Size);

        for(;;)
        {
            // Save the current vector and gradient
            MemoryCopy(OldVector, Vector, sizeof(F32) * Size);
            MemoryCopy(OldGradient, Gradient, sizeof(F32) * Size);
            F32 GradientDot = DotVector(Gradient, MovingDirection, Size);
            const F32 StepMax = Params->MaxStep;

            // Line search to update vector, function, and gradient
            LineSearch(OldVector, MovingDirection, StepMax, &Step, Output, Gradient, &GradientDot, Vector, Size, ComputeFunction, Params);

            // New gradient norm
            NormGradient = NormalizeVector(Gradient, Size);

            // Convergence test -- gradient
            if(NormGradient <= Params->Epsilon || NormGradient <= Params->RelativeEpsilon * NormalizeVector(Vector, Size))
                break;
            // Convergence test -- objective function value
            if(Past > 0)
            {
                const F32 FunctionResult = FunctionResults[Iterations % Past];
                if(Iterations >= Past && AbsF32(FunctionResult - *Output) <=
                   Params->Delta * Max(Max(AbsF32(*Output), AbsF32(FunctionResult)), 1.0f))
                {
                    break;
                }

                FunctionResults[Iterations % Past] = *Output;
            }
            // Maximum number of iterations
            if(Params->MaxIterations != 0 && Iterations >= Params->MaxIterations)
                break;

            // Update VectorS and VectorY
            // s_{k+1} = x_{k+1} - x_k
            // y_{k+1} = g_{k+1} - g_k
            for(U32 Element = 0; Element < Size; ++Element)
                VectorS[Element] = Vector[Element] - OldVector[Element];
            for(U32 Element = 0; Element < Size; ++Element)
                VectorY[Element] = Gradient[Element] - OldGradient[Element];
            if(DotVector(VectorS, VectorY, Size) > F32Epsilon * DotVector(VectorY, VectorY, Size))
            {
                // Add correction to the matrix
                const U32 Location = MatrixPointer % Params->MatrixSize;
                
                for(U32 Element = 0; Element < Size; ++Element)
                    MatrixS[Location * Size + Element] = VectorS[Element];
                for(U32 Element = 0; Element < Size; ++Element)
                    MatrixY[Location * Size + Element] = VectorY[Element];

                // ys = y's = 1/rho
                const F32 YHistory = DotVector(MatrixS + Location * Size, MatrixY + Location * Size, Size);
                MatrixYHistory[Location] = YHistory;

                MatrixTheta = DotVector(MatrixY + Location * Size, MatrixY + Location * Size, Size) / YHistory;

                if(MatrixCorrectionNumber < Params->MatrixSize) ++MatrixCorrectionNumber;

                MatrixPointer = Location + 1;
            }

            // Recursive formula to compute d = -H * g (L-BFGS two-loop recursion)
            {
                F32 *v = Gradient;
                F32 a = -1.0f;
                F32 *Result = MovingDirection;
                
                // Loop 1
                for(U32 Element = 0; Element < Size; ++Element)
                    Result[Element] = v[Element] * a;
                U32 j = MatrixPointer % Params->MatrixSize;
                for(U32 i = 0; i < MatrixCorrectionNumber; ++i)
                {
                    j = (j + Params->MatrixSize - 1) % Params->MatrixSize;
                    MatrixAlpha[j] = DotVector(MatrixS + j * Size, Result, Size) / MatrixYHistory[j];
                    for(U32 Element = 0; Element < Size; ++Element)
                        Result[Element] -= MatrixAlpha[j] * MatrixY[j * Size + Element];
                }

                // Apply initial H0
                for(U32 Element = 0; Element < Size; ++Element)
                    Result[Element] /= MatrixTheta;

                // Loop 2
                for(U32 i = 0; i < MatrixCorrectionNumber; ++i)
                {
                    const F32 Beta = DotVector(MatrixY + j * Size, Result, Size) / MatrixYHistory[j];
                    for(U32 Element = 0; Element < Size; ++Element)
                        Result[Element] += (MatrixAlpha[j] - Beta) * MatrixS[j * Size + Element];
                    j = (j + 1) % Params->MatrixSize;
                }
            }

            Step = 1.0f;
            ++Iterations;
        }
    }

    ReleaseScratch(Scratch);

    return Iterations;
}

F32 FirstQuadF32(F32 Rev)
{    
    Rev = fmodf(Rev, 1.0f);
    if(Rev < 0.0f)
        Rev += 1.0f;

    if(Rev <= 0.25f)
        return Rev;
    else if(Rev <= 0.5f)
        return 0.5f - Rev;
    else if(Rev <= 0.75f)
        return Rev - 0.5f;
    
    return 1.0f - Rev;
}

U64 MathRandAvalancheU64(U64 h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}

void MathRandSeed(rand_state *State, U32 Seed)
{
    U64 Value = (((U64)Seed) << 1) | 1;
    Value = MathRandAvalancheU64(Seed);
    State->State[0] = 0;
    State->State[1] = (Value << 1) | 1;
    MathRandNextU32(State);
    State->State[0] += MathRandAvalancheU64(Seed);
    MathRandNextU32(State);
}

U32 MathRandNextU32(rand_state *State)
{
    U64 OldState = State->State[0];
    State->State[0] = OldState * 0x5851f42d4c957f2dull + State->State[1];
    U32 Shifted = (U32)(((OldState >> 18) ^ OldState) >> 27);
    U32 Rot = (U32)(OldState >> 59);
    return (Shifted >> Rot) | (Shifted << ((-(I32)Rot) & 31));
}

F32 MathRandNextF32(rand_state *State)
{
    U32 Random = MathRandNextU32(State);
    U32 Exponent = 127;
    U32 Mantissa = Random >> 9;
    U32 Result = (Exponent << 23) | Mantissa;
    F32 ResultF32 = *(F32 *)(&Result);
    return ResultF32 - 1.0f;
}

F32 Mix1F32(F32 First, F32 Second, F32 Time)
{
    F32 Result = (First + (Second - First) * Clamp(0.0f, Time, 1.0f));
    return Result;
}

F64 Mix1F64(F64 First, F64 Second, F64 Time)
{
    F64 Result = (First + (Second - First) * Clamp(0.0f, Time, 1.0f));
    return Result;
}

vec2_f32 Normalize2F32(vec2_f32 Vec)
{
    F32 Length = Length2F32(Vec);
    F32 InvLength = Length != 0 ? (1.0f / Length) : 0.0f;
    return Mul2F32(Vec, Vec2F32(InvLength, InvLength));
}

vec3_f32 Normalize3F32(vec3_f32 Vec)
{
    F32 InvLength = 1.0f / SqrtF32(Vec.X * Vec.X + Vec.Y * Vec.Y + Vec.Z * Vec.Z);
    return Mul3F32(Vec, Vec3F32(InvLength, InvLength, InvLength));
}

mat3x3_f32 Outer3F32(vec3_f32 a, vec3_f32 b)
{
    mat3x3_f32 Result = {0};
    for(U32 j = 0; j < 3; ++j)
    {
        for(U32 i = 0; i < 3; ++i)
            Result.V[i][j] = a.V[i] * b.V[j];
    }
    return Result;}

// Rng2F32 --------------------------------------
rng2_f32 Rng2F32(vec2_f32 Min, vec2_f32 Max)
{
    return (rng2_f32){Min, Max};
}

rng2_f32 Pad2F32(rng2_f32 Rng, F32 Pad)
{
    vec2_f32 xv = {Pad, Pad};
    Rng.TopLeft = Sub2F32(Rng.TopLeft, xv);
    Rng.BottomRight = Add2F32(Rng.BottomRight, xv);
    return Rng;
}
vec2_f32 Dim2F32(rng2_f32 Rng)
{
    return Vec2F32(Rng.Right - Rng.Left, Rng.Bottom - Rng.Top);
}

B8 Contains2F32(rng2_f32 Rng, vec2_f32 Point)
{
    return (Rng.Min.X <= Point.X && Point.X < Rng.Max.X && Rng.Min.Y <= Point.Y && Point.Y < Rng.Max.Y);
}

B8 Colliding2F32(rng2_f32 First, rng2_f32 Second)
{
    B8 Result = 1;
    if(First.Right < Second.Left || Second.Right < First.Left) Result = 0;
    if(First.Bottom < Second.Top || Second.Bottom < First.Top) Result = 0;
    return Result;
}

rng2_f32 Intersect2F32(rng2_f32 First, rng2_f32 Second)
{
    rng2_f32 Result = {0};
    Result.Left = Max(First.Left, Second.Left);
    Result.Top = Max(First.Top, Second.Top);
    Result.Right = Min(First.Right, Second.Right);
    Result.Bottom = Min(First.Bottom, Second.Bottom);
    return Result;
}

rng2_f32 Shift2F32(rng2_f32 Rng, vec2_f32 Vec)
{
    Rng.TopLeft = Add2F32(Rng.TopLeft, Vec);
    Rng.BottomRight = Add2F32(Rng.BottomRight, Vec);
    return Rng;
}

F32 Length2F32(vec2_f32 Vec)
{
    F32 Result = SqrtF32(Vec.X * Vec.X + Vec.Y * Vec.Y);
    return Result;
}

// Rng2I16 --------------------------------------
rng2_i16 Rng2I16(vec2_i16 Min, vec2_i16 Max)
{
    return (rng2_i16){Min, Max};
}
rng2_i16 Pad2I16(rng2_i16 Rng, I16 Pad)
{
    vec2_i16 xv = {Pad, Pad};
    Rng.TopLeft = Sub2I16(Rng.TopLeft, xv);
    Rng.BottomRight = Add2I16(Rng.BottomRight, xv);
    return Rng;
}
vec2_i16 Dim2I16(rng2_i16 Rng)
{
    return Vec2I16(Rng.Right - Rng.Left, Rng.Bottom - Rng.Top);
}

B8 Contains2I16(rng2_i16 Rng, vec2_i16 Point)
{
    return (Rng.Min.X <= Point.X && Point.X < Rng.Max.X && Rng.Min.Y <= Point.Y && Point.Y < Rng.Max.Y);
}

// Rng2I32 --------------------------------------
rng2_i32 Rng2I32(vec2_i32 Min, vec2_i32 Max)
{
    return (rng2_i32){Min, Max};
}
rng2_i32 Pad2I32(rng2_i32 Rng, I32 Pad)
{
    vec2_i32 xv = {Pad, Pad};
    Rng.TopLeft = Sub2I32(Rng.TopLeft, xv);
    Rng.BottomRight = Add2I32(Rng.BottomRight, xv);
    return Rng;
}
vec2_i32 Dim2I32(rng2_i32 Rng)
{
    return Vec2I32(Rng.Right - Rng.Left, Rng.Bottom - Rng.Top);
}

B8 Contains2I32(rng2_i32 Rng, vec2_i32 Point)
{
    return (Rng.Min.X <= Point.X && Point.X < Rng.Max.X && Rng.Min.Y <= Point.Y && Point.Y < Rng.Max.Y);
}

// Rng2I64
rng2_i64 Rng2I64(vec2_i64 Min, vec2_i64 Max)
{
    return (rng2_i64){Min, Max};
}
rng2_i64 Pad2I64(rng2_i64 Rng, I64 Pad)
{
    vec2_i64 xv = {Pad, Pad};
    Rng.TopLeft = Sub2I64(Rng.TopLeft, xv);
    Rng.BottomRight = Add2I64(Rng.BottomRight, xv);
    return Rng;
}
vec2_i64 Dim2I64(rng2_i64 Rng)
{
    return Vec2I64(Rng.Right - Rng.Left, Rng.Bottom - Rng.Top);
}

B8 Contains2I64(rng2_i64 Rng, vec2_i64 Point)
{
    return (Rng.Min.X <= Point.X && Point.X < Rng.Max.X && Rng.Min.Y <= Point.Y && Point.Y < Rng.Max.Y);
}

// Mat2x2F32 ------------------------------------
mat2x2_f32 Mat2x2F32(F32 Diagonal)
{
    mat2x2_f32 Result = {0};
    Result.V[0][0] = Diagonal;
    Result.V[1][1] = Diagonal;
    return Result;
}

mat2x2_f32 Mul2x2F32(mat2x2_f32 a, mat2x2_f32 b)
{
    mat2x2_f32 c = {0};
    for(U32 j = 0; j < 2; ++j)
    {
        for(U32 i = 0; i < 2; ++i)
        {
            c.V[i][j] = (a.V[0][j] * b.V[i][0] +
                         a.V[1][j] * b.V[i][1]);
        }
    }
    return c;
}

mat2x2_f32 Rotate2x2F32(F32 Turns)
{
    mat2x2_f32 Result = Mat2x2F32(1.0f);
    F32 CosTheta = CosF32(Turns);
    F32 SinTheta = SinF32(Turns);
    Result.V[0][0] = CosTheta;
    Result.V[0][1] = -SinTheta;
    Result.V[1][0] = SinTheta;
    Result.V[1][1] = CosTheta;
    return Result;
}

vec2_f32 Mul2F32By2x2F32(mat2x2_f32 Matrix, vec2_f32 Vector)
{
    vec2_f32 Result = {0};
    for(U32 Row = 0; Row < 2; ++Row)
    {
        for(U32 Col = 0; Col < 2; ++Col)
            Result.V[Row] += Matrix.V[Col][Row] * Vector.V[Col];
    }
    return Result;
}

// Mat3x3F32 ------------------------------------
mat3x3_f32 Mat3x3F32(F32 Diagonal)
{
    mat3x3_f32 Result = {0};
    Result.V[0][0] = Diagonal;
    Result.V[1][1] = Diagonal;
    Result.V[2][2] = Diagonal;
    return Result;
}

mat3x3_f32 Translate3x3F32(vec2_f32 Delta)
{
    mat3x3_f32 Result = Mat3x3F32(1.0f);
    Result.V[2][0] = Delta.X;
    Result.V[2][1] = Delta.Y;
    return Result;
}

mat3x3_f32 Scale3x3F32(vec2_f32 Scale)
{
    mat3x3_f32 Result = Mat3x3F32(1.0f);
    Result.V[0][0] = Scale.X;
    Result.V[1][1] = Scale.Y;
    return Result;
}

mat3x3_f32 Rotate3x3F32(F32 Turns)
{
    mat3x3_f32 Result = Mat3x3F32(1.0f);
    F32 SinTheta = SinF32(Turns);
    F32 CosTheta = CosF32(Turns);
    Result.V[0][0] = CosTheta;
    Result.V[0][1] = -SinTheta;
    Result.V[1][0] = SinTheta;
    Result.V[1][1] = CosTheta;
    return Result;
}

mat3x3_f32 Mul3x3F32(mat3x3_f32 a, mat3x3_f32 b)
{
    mat3x3_f32 c = {0};
    for(U32 j = 0; j < 3; ++j)
    {
        for(U32 i = 0; i < 3; ++i)
        {
            c.V[i][j] = (a.V[0][j] * b.V[i][0] +
                         a.V[1][j] * b.V[i][1] +
                         a.V[2][j] * b.V[i][2]);
        }
    }
    return c;
}

vec3_f32 Mul3F32By3x3F32(mat3x3_f32 Matrix, vec3_f32 Vector)
{
    vec3_f32 Result = {0};
    for(U32 Row = 0; Row < 3; ++Row)
    {
        for(U32 Col = 0; Col < 3; ++Col)
            Result.V[Row] += Matrix.V[Col][Row] * Vector.V[Col];
    }
    return Result;
}

vec2_f32 Mul2F32By3x3F32(mat3x3_f32 Matrix, vec2_f32 Vector)
{
    vec3_f32 V3Res = Mul3F32By3x3F32(Matrix, Vec3F32(Vector.X, Vector.Y, 1.0f));
    vec2_f32 Result = Vec2F32(V3Res.X / V3Res.Z, V3Res.Y / V3Res.Z);
    return Result;
}

mat3x3_f32 Transpose3x3F32(mat3x3_f32 Matrix)
{
    mat3x3_f32 Result = {0};
    for(U32 j = 0; j < 3; ++j)
    {
        for(U32 i = 0; i < 3; ++i)
            Result.V[j][i] = Matrix.V[i][j];
    }
    return Result;
}

// Mat4x4F32 ------------------------------------
mat4x4_f32 Mat4x4F32(F32 Diagonal)
{
    mat4x4_f32 Result = {0};
    Result.V[0][0] = Diagonal;
    Result.V[1][1] = Diagonal;
    Result.V[2][2] = Diagonal;
    Result.V[3][3] = Diagonal;
    return Result;
}

mat4x4_f32 Translate4x4F32(vec3_f32 Delta)
{
    mat4x4_f32 Result = Mat4x4F32(1.0f);
    Result.V[3][0] = Delta.X;
    Result.V[3][1] = Delta.Y;
    Result.V[3][2] = Delta.Z;
    return Result;
}

mat4x4_f32 Scale4x4F32(vec3_f32 Scale)
{
    mat4x4_f32 Result = Mat4x4F32(1.0f);
    Result.V[0][0] = Scale.X;
    Result.V[1][1] = Scale.Y;
    Result.V[2][2] = Scale.Z;
    return Result;
}

mat4x4_f32 Rotate4x4F32(vec3_f32 Axis, F32 Turns)
{
    mat4x4_f32 Result = Mat4x4F32(1.0f);
    Axis = Normalize3F32(Axis);
    F32 SinTheta = SinF32(Turns);
    F32 CosTheta = CosF32(Turns);
    F32 CosValue = 1.0f - CosTheta;
    Result.V[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.V[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.V[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);
    Result.V[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.V[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.V[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);
    Result.V[2][0] = (Axis.Z * Axis.X * CosValue) * (Axis.Y * SinTheta);
    Result.V[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.V[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;
    return Result;
}

mat4x4_f32 Mul4x4F32(mat4x4_f32 a, mat4x4_f32 b)
{
    mat4x4_f32 c = {0};
    for(U32 j = 0; j < 4; ++j)
    {
        for(U32 i = 0; i < 4; ++i)
        {
            c.V[i][j] = (a.V[0][j] * b.V[i][0] +
                         a.V[1][j] * b.V[i][1] +
                         a.V[2][j] * b.V[i][2] +
                         a.V[3][j] * b.V[i][3]);
        }
    }
    return c;
}

// Quaternion ops -------------------------------
quat_f32 IdentityQF32()
{
    return (quat_f32){0.0f, 0.0f, 0.0f, 1.0f};
}

quat_f32 RealImaginaryQF32(F32 Real, vec3_f32 Imaginary)
{
    return (quat_f32){Imaginary.X, Imaginary.Y, Imaginary.Z, Real};
}

quat_f32 ConjugateQF32(quat_f32 q)
{
    return (quat_f32)
    {
        -q.X,
        -q.Y,
        -q.Z,
        q.W
    };
}

quat_f32 MulQF32(quat_f32 q1, quat_f32 q2)
{    
    quat_f32 OutQuaternion;

    F32 qax = q1.X, qay = q1.Y, qaz = q1.Z, qaw = q1.W;
    F32 qbx = q2.X, qby = q2.Y, qbz = q2.Z, qbw = q2.W;
    
    OutQuaternion.X = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
    OutQuaternion.Y = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
    OutQuaternion.Z = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
    OutQuaternion.W = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;

    return OutQuaternion;
}

vec3_f32 Rotate3F32FromQuatF32(vec3_f32 v, quat_f32 q)
{
    vec4_f32 Result = MulQF32(MulQF32(q, Vec4F32(v.X, v.Y, v.Z, 0.0f)), ConjugateQF32(q));
    return Vec3F32(Result.X, Result.Y, Result.Z);
}

