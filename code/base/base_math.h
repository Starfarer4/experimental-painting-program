#if !defined(BASE_MATH_H)
/*  =======================================================================
    File: base_math.h
    Date: March 6th 2024  2:22 PM
    Creator: Quinn Van De Keere
    =======================================================================*/
#define BASE_MATH_H

#define PI_F32 3.1415926535897f

#define RadiansFromTurnsF32(x) ((x) * (2 * PI_F32))
#define TurnsFromRadiansF32(x) ((x) / (2 * PI_F32))

#define CeilF32(x) ceilf(x)
#define FloorF32(x) floorf(x)
#define PowF32(a, b) powf((a), (b))
#define RoundF32(x) roundf(x)
#define AbsF32(x) fabsf(x)
#define AbsI64(x) (I64)llabs(x)
#define SqrtF32(x) sqrtf(x)
#define SinF32(x) sinf(RadiansFromTurnsF32(x))
#define CosF32(x) cosf(RadiansFromTurnsF32(x))
#define TanF32(x) tanf(RadiansFromTurnsF32(x))
#define ATan2F32(x, y) TurnsFromRadiansF32(atan2f((x), (y)))
#define ExpF32(x) expf(x)
#define ModF32(x, y) fmodf((x), (y))
#define LogF32(x) logf(x)

F32 FirstQuadF32(F32 Rev);

#include "generated/base_math.meta.h"

typedef union rng2_f32 rng2_f32;
union rng2_f32
{
    struct
    {
        vec2_f32 Min;
        vec2_f32 Max;
    };
    struct
    {
        vec2_f32 TopLeft;
        vec2_f32 BottomRight;
    };
    struct
    {
        F32 Left;
        F32 Top;
        F32 Right;
        F32 Bottom;
    };
    vec2_f32 V[2];
};

typedef union rng2_i16 rng2_i16;
union rng2_i16
{
    struct
    {
        vec2_i16 Min;
        vec2_i16 Max;
    };
    struct
    {
        vec2_i16 TopLeft;
        vec2_i16 BottomRight;
    };
    struct
    {
        I16 Left;
        I16 Top;
        I16 Right;
        I16 Bottom;
    };
    vec2_i16 V[2];
};

typedef union rng2_i32 rng2_i32;
union rng2_i32
{
    struct
    {
        vec2_i32 Min;
        vec2_i32 Max;
    };
    struct
    {
        vec2_i32 TopLeft;
        vec2_i32 BottomRight;
    };
    struct
    {
        I32 Left;
        I32 Top;
        I32 Right;
        I32 Bottom;
    };
    vec2_i32 V[2];
};

typedef union rng2_i64 rng2_i64;
union rng2_i64
{
    struct
    {
        vec2_i64 Min;
        vec2_i64 Max;
    };
    struct
    {
        vec2_i64 TopLeft;
        vec2_i64 BottomRight;
    };
    struct
    {
        I64 Left;
        I64 Top;
        I64 Right;
        I64 Bottom;
    };
    vec2_i64 V[2];
};

typedef struct mat2x2_f32 mat2x2_f32;
struct mat2x2_f32
{
    F32 V[2][2];
};

typedef struct mat3x3_f32 mat3x3_f32;
struct mat3x3_f32
{
    F32 V[3][3];
};

typedef struct mat4x4_f32 mat4x4_f32;
struct mat4x4_f32
{
    F32 V[4][4];
};

typedef vec4_f32 quat_f32;

F64 Mix1F64(F64 First, F64 Second, F64 Time);
F32 Mix1F32(F32 First, F32 Second, F32 Time);

F32 Length2F32(vec2_f32 Vec);
vec3_f32 Normalize3F32(vec3_f32 Vec);
mat3x3_f32 Outer3F32(vec3_f32 a, vec3_f32 b);
vec2_f32 Normalize2F32(vec2_f32 Vec);

rng2_f32 Rng2F32(vec2_f32 Min, vec2_f32 Max);
rng2_f32 Pad2F32(rng2_f32 Rng, F32 Pad);
vec2_f32 Dim2F32(rng2_f32 Rng);
B8 Contains2F32(rng2_f32 Rng, vec2_f32 Point);
B8 Colliding2F32(rng2_f32 First, rng2_f32 Second);
rng2_f32 Intersect2F32(rng2_f32 First, rng2_f32 Second);
rng2_f32 Shift2F32(rng2_f32 Rng, vec2_f32 Vec);

rng2_i16 Rng2I16(vec2_i16 Min, vec2_i16 Max);
rng2_i16 Pad2I16(rng2_i16 Rng, I16 Pad);
vec2_i16 Dim2I16(rng2_i16 Rng);

rng2_i32 Rng2I32(vec2_i32 Min, vec2_i32 Max);
rng2_i32 Pad2I32(rng2_i32 Rng, I32 Pad);
vec2_i32 Dim2I32(rng2_i32 Rng);

rng2_i64 Rng2I64(vec2_i64 Min, vec2_i64 Max);
rng2_i64 Pad2I64(rng2_i64 Rng, I64 Pad);
vec2_i64 Dim2I64(rng2_i64 Rng);

mat2x2_f32 Mat2x2F32(F32 Diagonal);
mat2x2_f32 Mul2x2F32(mat2x2_f32 a, mat2x2_f32 b);
vec2_f32 Mul2F32By2x2F32(mat2x2_f32 Matrix, vec2_f32 Vector);
mat2x2_f32 Rotate2x2F32(F32 Turns);

mat3x3_f32 Mat3x3F32(F32 Diagonal);
mat3x3_f32 Translate3x3F32(vec2_f32 Delta);
mat3x3_f32 Scale3x3F32(vec2_f32 Scale);
mat3x3_f32 Rotate3x3F32(F32 Turns);
mat3x3_f32 Mul3x3F32(mat3x3_f32 a, mat3x3_f32 b);
vec2_f32 Mul2F32By3x3F32(mat3x3_f32 Matrix, vec2_f32 Vector);
vec3_f32 Mul3F32By3x3F32(mat3x3_f32 Matrix, vec3_f32 Vector);
mat3x3_f32 Transpose3x3F32(mat3x3_f32 Matrix);

mat4x4_f32 Mat4x4F32(F32 Diagonal);
mat4x4_f32 Translate4x4F32(vec3_f32 Delta);
mat4x4_f32 Scale4x4F32(vec3_f32 Scale);
mat4x4_f32 Rotate4x4F32(vec3_f32 Axis, F32 Turns);
mat4x4_f32 Mul4x4F32(mat4x4_f32 a, mat4x4_f32 b);

quat_f32 IdentityQF32();
quat_f32 RealImaginaryQF32(F32 Real, vec3_f32 Imaginary);
quat_f32 ConjugateQF32(quat_f32 q);
quat_f32 MulQF32(quat_f32 q1, quat_f32 q2);
vec3_f32 Rotate3F32FromQuatF32(vec3_f32 v, quat_f32 q);

#define RGBAFromU32Lit(h) {(((h)&0xff000000)>>24)/255.0f, (((h)&0x00ff0000)>>16)/255.0f, \
            (((h)&0x0000ff00)>>8)/255.0f, (((h)&0x000000ff)>>0)/255.0f}

typedef struct rand_state rand_state;
struct rand_state
{
    U64 State[2];
};

void MathRandSeed(rand_state *State, U32 Seed);
U32 MathRandNextU32(rand_state *State);
F32 MathRandNextF32(rand_state *State);

// Should return the objective function value on a vector, `Vector`, and overwrite the vector `Gradient` with the gradient evaluated on x
typedef F32 (*lbfgs_compute_function)(F32 *Vector, F32 *Gradient, U32 VectorSize, void *UserData);

typedef enum lbfgs_linesearch_backtracking
{
    LBFGS_LINESEARCH_BACKTRACKING_ARMIJO = 1,
    LBFGS_LINESEARCH_BACKTRACKING = 2,
    LBFGS_LINESEARCH_BACKTRACKING_WOLFE = 2,
    LBFGS_LINESEARCH_BACKTRACKING_STRONG_WOLFE = 3
}lbfgs_linesearch_backtracking;

typedef struct lbfgs_solver_params lbfgs_solver_params;
struct lbfgs_solver_params
{
    F32 Epsilon;
    F32 RelativeEpsilon;
    F32 MaxStep;
    F32 MinStep;
    F32 LinesearchAccuracy;
    lbfgs_linesearch_backtracking Linesearch;
    F32 Wolfe;
    F32 Delta;
    U32 MaxIterations;
    U32 MaxLinesearch;
    U32 Past;
    U32 MatrixSize;
    void *UserData;
};

F32 DotVector(F32 *Vector1, F32 *Vector2, U32 VectorSize);
F32 NormalizeVector(F32 *Vector, U32 VectorSize);

void LineSearch(F32 *CurrentVector, F32 *CurrentMovingDirection, F32 StepMax, F32 *Step, F32 *FunctionValue, F32 *Gradient,
                F32 *DirGradient, F32 *NewVector, U32 Size, lbfgs_compute_function ComputeFunction, lbfgs_solver_params *Params);

// `Vector` is the input for the initial guess and the output for the best point found
// `Size` is the size of the vector
// `Output` is the objective function value at x
// `ComputeFunction` is a function such that f(x, grad) returns the objective function value at x and overwrites grad with the gradient
// `Params` are the other parameters of this LBFGS function
U32 MinimizeLBFGS(F32 *Vector, U32 Size, F32 *Output, lbfgs_compute_function ComputeFunction, lbfgs_solver_params *Params);
lbfgs_solver_params InitializeDefaultLBFGSParams();

#endif
