rng2_f32 RndPushClip(rng2_f32 Data) {RndStackPushImpl(Clip, clip, rng2_f32, Data);}
mat3x3_f32 RndPushTransform(mat3x3_f32 Data) {RndStackPushImpl(Transform, transform, mat3x3_f32, Data);}
rnd_tex2d_sampler_kind RndPushSamplerKind(rnd_tex2d_sampler_kind Data) {RndStackPushImpl(SamplerKind, sampler_kind, rnd_tex2d_sampler_kind, Data);}
rng2_f32 RndPopClip() {RndStackPopImpl(Clip, clip, rng2_f32);}
mat3x3_f32 RndPopTransform() {RndStackPopImpl(Transform, transform, mat3x3_f32);}
rnd_tex2d_sampler_kind RndPopSamplerKind() {RndStackPopImpl(SamplerKind, sampler_kind, rnd_tex2d_sampler_kind);}
rng2_f32 RndTopClip() {RndStackTopImpl(Clip, clip, rng2_f32);}
mat3x3_f32 RndTopTransform() {RndStackTopImpl(Transform, transform, mat3x3_f32);}
rnd_tex2d_sampler_kind RndTopSamplerKind() {RndStackTopImpl(SamplerKind, sampler_kind, rnd_tex2d_sampler_kind);}

