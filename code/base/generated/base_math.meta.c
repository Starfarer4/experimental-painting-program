vec2_f32 Vec2F32(F32 X, F32 Y) {return (vec2_f32){X, Y};}
vec2_f64 Vec2F64(F64 X, F64 Y) {return (vec2_f64){X, Y};}
vec2_i16 Vec2I16(I16 X, I16 Y) {return (vec2_i16){X, Y};}
vec2_i32 Vec2I32(I32 X, I32 Y) {return (vec2_i32){X, Y};}
vec2_i64 Vec2I64(I64 X, I64 Y) {return (vec2_i64){X, Y};}
vec2_f32 Add2F32(vec2_f32 First, vec2_f32 Second) {return Vec2F32(First.X + Second.X, First.Y + Second.Y);}
vec2_f64 Add2F64(vec2_f64 First, vec2_f64 Second) {return Vec2F64(First.X + Second.X, First.Y + Second.Y);}
vec2_i16 Add2I16(vec2_i16 First, vec2_i16 Second) {return Vec2I16(First.X + Second.X, First.Y + Second.Y);}
vec2_i32 Add2I32(vec2_i32 First, vec2_i32 Second) {return Vec2I32(First.X + Second.X, First.Y + Second.Y);}
vec2_i64 Add2I64(vec2_i64 First, vec2_i64 Second) {return Vec2I64(First.X + Second.X, First.Y + Second.Y);}
vec2_f32 Sub2F32(vec2_f32 First, vec2_f32 Second) {return Vec2F32(First.X - Second.X, First.Y - Second.Y);}
vec2_f64 Sub2F64(vec2_f64 First, vec2_f64 Second) {return Vec2F64(First.X - Second.X, First.Y - Second.Y);}
vec2_i16 Sub2I16(vec2_i16 First, vec2_i16 Second) {return Vec2I16(First.X - Second.X, First.Y - Second.Y);}
vec2_i32 Sub2I32(vec2_i32 First, vec2_i32 Second) {return Vec2I32(First.X - Second.X, First.Y - Second.Y);}
vec2_i64 Sub2I64(vec2_i64 First, vec2_i64 Second) {return Vec2I64(First.X - Second.X, First.Y - Second.Y);}
vec2_f32 Div2F32(vec2_f32 First, vec2_f32 Second) {return Vec2F32(First.X / Second.X, First.Y / Second.Y);}
vec2_f64 Div2F64(vec2_f64 First, vec2_f64 Second) {return Vec2F64(First.X / Second.X, First.Y / Second.Y);}
vec2_i16 Div2I16(vec2_i16 First, vec2_i16 Second) {return Vec2I16(First.X / Second.X, First.Y / Second.Y);}
vec2_i32 Div2I32(vec2_i32 First, vec2_i32 Second) {return Vec2I32(First.X / Second.X, First.Y / Second.Y);}
vec2_i64 Div2I64(vec2_i64 First, vec2_i64 Second) {return Vec2I64(First.X / Second.X, First.Y / Second.Y);}
vec2_f32 Mul2F32(vec2_f32 First, vec2_f32 Second) {return Vec2F32(First.X * Second.X, First.Y * Second.Y);}
vec2_f64 Mul2F64(vec2_f64 First, vec2_f64 Second) {return Vec2F64(First.X * Second.X, First.Y * Second.Y);}
vec2_i16 Mul2I16(vec2_i16 First, vec2_i16 Second) {return Vec2I16(First.X * Second.X, First.Y * Second.Y);}
vec2_i32 Mul2I32(vec2_i32 First, vec2_i32 Second) {return Vec2I32(First.X * Second.X, First.Y * Second.Y);}
vec2_i64 Mul2I64(vec2_i64 First, vec2_i64 Second) {return Vec2I64(First.X * Second.X, First.Y * Second.Y);}
F32 Dot2F32(vec2_f32 a, vec2_f32 b) {return a.X * b.X + a.Y * b.Y;}
F64 Dot2F64(vec2_f64 a, vec2_f64 b) {return a.X * b.X + a.Y * b.Y;}
I16 Dot2I16(vec2_i16 a, vec2_i16 b) {return a.X * b.X + a.Y * b.Y;}
I32 Dot2I32(vec2_i32 a, vec2_i32 b) {return a.X * b.X + a.Y * b.Y;}
I64 Dot2I64(vec2_i64 a, vec2_i64 b) {return a.X * b.X + a.Y * b.Y;}
vec2_f32 Mix2F32(vec2_f32 First, vec2_f32 Second, F32 Time) {vec2_f32 Result = Vec2F32((F32)Mix1F32((F32)First.X, (F32)Second.X, Time), (F32)Mix1F32((F32)First.Y, (F32)Second.Y, Time)); return Result;}
vec2_f64 Mix2F64(vec2_f64 First, vec2_f64 Second, F32 Time) {vec2_f64 Result = Vec2F64((F64)Mix1F32((F32)First.X, (F32)Second.X, Time), (F64)Mix1F32((F32)First.Y, (F32)Second.Y, Time)); return Result;}
vec2_i16 Mix2I16(vec2_i16 First, vec2_i16 Second, F32 Time) {vec2_i16 Result = Vec2I16((I16)Mix1F32((F32)First.X, (F32)Second.X, Time), (I16)Mix1F32((F32)First.Y, (F32)Second.Y, Time)); return Result;}
vec2_i32 Mix2I32(vec2_i32 First, vec2_i32 Second, F32 Time) {vec2_i32 Result = Vec2I32((I32)Mix1F32((F32)First.X, (F32)Second.X, Time), (I32)Mix1F32((F32)First.Y, (F32)Second.Y, Time)); return Result;}
vec2_i64 Mix2I64(vec2_i64 First, vec2_i64 Second, F32 Time) {vec2_i64 Result = Vec2I64((I64)Mix1F32((F32)First.X, (F32)Second.X, Time), (I64)Mix1F32((F32)First.Y, (F32)Second.Y, Time)); return Result;}

vec3_f32 Vec3F32(F32 X, F32 Y, F32 Z) {return (vec3_f32){X, Y, Z};}
vec3_f64 Vec3F64(F64 X, F64 Y, F64 Z) {return (vec3_f64){X, Y, Z};}
vec3_i16 Vec3I16(I16 X, I16 Y, I16 Z) {return (vec3_i16){X, Y, Z};}
vec3_i32 Vec3I32(I32 X, I32 Y, I32 Z) {return (vec3_i32){X, Y, Z};}
vec3_i64 Vec3I64(I64 X, I64 Y, I64 Z) {return (vec3_i64){X, Y, Z};}
vec3_f32 Add3F32(vec3_f32 First, vec3_f32 Second) {return Vec3F32(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z);}
vec3_f64 Add3F64(vec3_f64 First, vec3_f64 Second) {return Vec3F64(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z);}
vec3_i16 Add3I16(vec3_i16 First, vec3_i16 Second) {return Vec3I16(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z);}
vec3_i32 Add3I32(vec3_i32 First, vec3_i32 Second) {return Vec3I32(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z);}
vec3_i64 Add3I64(vec3_i64 First, vec3_i64 Second) {return Vec3I64(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z);}
vec3_f32 Sub3F32(vec3_f32 First, vec3_f32 Second) {return Vec3F32(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z);}
vec3_f64 Sub3F64(vec3_f64 First, vec3_f64 Second) {return Vec3F64(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z);}
vec3_i16 Sub3I16(vec3_i16 First, vec3_i16 Second) {return Vec3I16(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z);}
vec3_i32 Sub3I32(vec3_i32 First, vec3_i32 Second) {return Vec3I32(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z);}
vec3_i64 Sub3I64(vec3_i64 First, vec3_i64 Second) {return Vec3I64(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z);}
vec3_f32 Div3F32(vec3_f32 First, vec3_f32 Second) {return Vec3F32(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z);}
vec3_f64 Div3F64(vec3_f64 First, vec3_f64 Second) {return Vec3F64(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z);}
vec3_i16 Div3I16(vec3_i16 First, vec3_i16 Second) {return Vec3I16(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z);}
vec3_i32 Div3I32(vec3_i32 First, vec3_i32 Second) {return Vec3I32(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z);}
vec3_i64 Div3I64(vec3_i64 First, vec3_i64 Second) {return Vec3I64(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z);}
vec3_f32 Mul3F32(vec3_f32 First, vec3_f32 Second) {return Vec3F32(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z);}
vec3_f64 Mul3F64(vec3_f64 First, vec3_f64 Second) {return Vec3F64(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z);}
vec3_i16 Mul3I16(vec3_i16 First, vec3_i16 Second) {return Vec3I16(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z);}
vec3_i32 Mul3I32(vec3_i32 First, vec3_i32 Second) {return Vec3I32(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z);}
vec3_i64 Mul3I64(vec3_i64 First, vec3_i64 Second) {return Vec3I64(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z);}
F32 Dot3F32(vec3_f32 a, vec3_f32 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z;}
F64 Dot3F64(vec3_f64 a, vec3_f64 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z;}
I16 Dot3I16(vec3_i16 a, vec3_i16 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z;}
I32 Dot3I32(vec3_i32 a, vec3_i32 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z;}
I64 Dot3I64(vec3_i64 a, vec3_i64 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z;}
vec3_f32 Mix3F32(vec3_f32 First, vec3_f32 Second, F32 Time) {vec3_f32 Result = Vec3F32((F32)Mix1F32((F32)First.X, (F32)Second.X, Time), (F32)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (F32)Mix1F32((F32)First.Z, (F32)Second.Z, Time)); return Result;}
vec3_f64 Mix3F64(vec3_f64 First, vec3_f64 Second, F32 Time) {vec3_f64 Result = Vec3F64((F64)Mix1F32((F32)First.X, (F32)Second.X, Time), (F64)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (F64)Mix1F32((F32)First.Z, (F32)Second.Z, Time)); return Result;}
vec3_i16 Mix3I16(vec3_i16 First, vec3_i16 Second, F32 Time) {vec3_i16 Result = Vec3I16((I16)Mix1F32((F32)First.X, (F32)Second.X, Time), (I16)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (I16)Mix1F32((F32)First.Z, (F32)Second.Z, Time)); return Result;}
vec3_i32 Mix3I32(vec3_i32 First, vec3_i32 Second, F32 Time) {vec3_i32 Result = Vec3I32((I32)Mix1F32((F32)First.X, (F32)Second.X, Time), (I32)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (I32)Mix1F32((F32)First.Z, (F32)Second.Z, Time)); return Result;}
vec3_i64 Mix3I64(vec3_i64 First, vec3_i64 Second, F32 Time) {vec3_i64 Result = Vec3I64((I64)Mix1F32((F32)First.X, (F32)Second.X, Time), (I64)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (I64)Mix1F32((F32)First.Z, (F32)Second.Z, Time)); return Result;}
vec3_f32 Cross3F32(vec3_f32 a, vec3_f32 b) {return Vec3F32(a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X);}
vec3_f64 Cross3F64(vec3_f64 a, vec3_f64 b) {return Vec3F64(a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X);}
vec3_i16 Cross3I16(vec3_i16 a, vec3_i16 b) {return Vec3I16(a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X);}
vec3_i32 Cross3I32(vec3_i32 a, vec3_i32 b) {return Vec3I32(a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X);}
vec3_i64 Cross3I64(vec3_i64 a, vec3_i64 b) {return Vec3I64(a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X);}

F32 Dot4F32(vec4_f32 a, vec4_f32 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;}
F64 Dot4F64(vec4_f64 a, vec4_f64 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;}
I16 Dot4I16(vec4_i16 a, vec4_i16 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;}
I32 Dot4I32(vec4_i32 a, vec4_i32 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;}
I64 Dot4I64(vec4_i64 a, vec4_i64 b) {return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;}
vec4_f32 Vec4F32(F32 X, F32 Y, F32 Z, F32 W) {return (vec4_f32){X, Y, Z, W};}
vec4_f64 Vec4F64(F64 X, F64 Y, F64 Z, F64 W) {return (vec4_f64){X, Y, Z, W};}
vec4_i16 Vec4I16(I16 X, I16 Y, I16 Z, I16 W) {return (vec4_i16){X, Y, Z, W};}
vec4_i32 Vec4I32(I32 X, I32 Y, I32 Z, I32 W) {return (vec4_i32){X, Y, Z, W};}
vec4_i64 Vec4I64(I64 X, I64 Y, I64 Z, I64 W) {return (vec4_i64){X, Y, Z, W};}
vec4_f32 Add4F32(vec4_f32 First, vec4_f32 Second) {return Vec4F32(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z, First.W + Second.W);}
vec4_f64 Add4F64(vec4_f64 First, vec4_f64 Second) {return Vec4F64(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z, First.W + Second.W);}
vec4_i16 Add4I16(vec4_i16 First, vec4_i16 Second) {return Vec4I16(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z, First.W + Second.W);}
vec4_i32 Add4I32(vec4_i32 First, vec4_i32 Second) {return Vec4I32(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z, First.W + Second.W);}
vec4_i64 Add4I64(vec4_i64 First, vec4_i64 Second) {return Vec4I64(First.X + Second.X, First.Y + Second.Y, First.Z + Second.Z, First.W + Second.W);}
vec4_f32 Sub4F32(vec4_f32 First, vec4_f32 Second) {return Vec4F32(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z, First.W - Second.W);}
vec4_f64 Sub4F64(vec4_f64 First, vec4_f64 Second) {return Vec4F64(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z, First.W - Second.W);}
vec4_i16 Sub4I16(vec4_i16 First, vec4_i16 Second) {return Vec4I16(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z, First.W - Second.W);}
vec4_i32 Sub4I32(vec4_i32 First, vec4_i32 Second) {return Vec4I32(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z, First.W - Second.W);}
vec4_i64 Sub4I64(vec4_i64 First, vec4_i64 Second) {return Vec4I64(First.X - Second.X, First.Y - Second.Y, First.Z - Second.Z, First.W - Second.W);}
vec4_f32 Div4F32(vec4_f32 First, vec4_f32 Second) {return Vec4F32(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z, First.W / Second.W);}
vec4_f64 Div4F64(vec4_f64 First, vec4_f64 Second) {return Vec4F64(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z, First.W / Second.W);}
vec4_i16 Div4I16(vec4_i16 First, vec4_i16 Second) {return Vec4I16(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z, First.W / Second.W);}
vec4_i32 Div4I32(vec4_i32 First, vec4_i32 Second) {return Vec4I32(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z, First.W / Second.W);}
vec4_i64 Div4I64(vec4_i64 First, vec4_i64 Second) {return Vec4I64(First.X / Second.X, First.Y / Second.Y, First.Z / Second.Z, First.W / Second.W);}
vec4_f32 Mul4F32(vec4_f32 First, vec4_f32 Second) {return Vec4F32(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z, First.W * Second.W);}
vec4_f64 Mul4F64(vec4_f64 First, vec4_f64 Second) {return Vec4F64(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z, First.W * Second.W);}
vec4_i16 Mul4I16(vec4_i16 First, vec4_i16 Second) {return Vec4I16(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z, First.W * Second.W);}
vec4_i32 Mul4I32(vec4_i32 First, vec4_i32 Second) {return Vec4I32(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z, First.W * Second.W);}
vec4_i64 Mul4I64(vec4_i64 First, vec4_i64 Second) {return Vec4I64(First.X * Second.X, First.Y * Second.Y, First.Z * Second.Z, First.W * Second.W);}
vec4_f32 Mix4F32(vec4_f32 First, vec4_f32 Second, F32 Time) {vec4_f32 Result = Vec4F32((F32)Mix1F32((F32)First.X, (F32)Second.X, Time), (F32)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (F32)Mix1F32((F32)First.Z, (F32)Second.Z, Time), (F32)Mix1F32((F32)First.W, (F32)Second.W, Time)); return Result;}
vec4_f64 Mix4F64(vec4_f64 First, vec4_f64 Second, F32 Time) {vec4_f64 Result = Vec4F64((F64)Mix1F32((F32)First.X, (F32)Second.X, Time), (F64)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (F64)Mix1F32((F32)First.Z, (F32)Second.Z, Time), (F64)Mix1F32((F32)First.W, (F32)Second.W, Time)); return Result;}
vec4_i16 Mix4I16(vec4_i16 First, vec4_i16 Second, F32 Time) {vec4_i16 Result = Vec4I16((I16)Mix1F32((F32)First.X, (F32)Second.X, Time), (I16)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (I16)Mix1F32((F32)First.Z, (F32)Second.Z, Time), (I16)Mix1F32((F32)First.W, (F32)Second.W, Time)); return Result;}
vec4_i32 Mix4I32(vec4_i32 First, vec4_i32 Second, F32 Time) {vec4_i32 Result = Vec4I32((I32)Mix1F32((F32)First.X, (F32)Second.X, Time), (I32)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (I32)Mix1F32((F32)First.Z, (F32)Second.Z, Time), (I32)Mix1F32((F32)First.W, (F32)Second.W, Time)); return Result;}
vec4_i64 Mix4I64(vec4_i64 First, vec4_i64 Second, F32 Time) {vec4_i64 Result = Vec4I64((I64)Mix1F32((F32)First.X, (F32)Second.X, Time), (I64)Mix1F32((F32)First.Y, (F32)Second.Y, Time), (I64)Mix1F32((F32)First.Z, (F32)Second.Z, Time), (I64)Mix1F32((F32)First.W, (F32)Second.W, Time)); return Result;}

