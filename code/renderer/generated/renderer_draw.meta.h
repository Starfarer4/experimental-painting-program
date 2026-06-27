typedef struct rnd_clip_node rnd_clip_node; struct rnd_clip_node {rnd_clip_node *Next; rng2_f32 Data;};
typedef struct rnd_transform_node rnd_transform_node; struct rnd_transform_node {rnd_transform_node *Next; mat3x3_f32 Data;};
typedef struct rnd_sampler_kind_node rnd_sampler_kind_node; struct rnd_sampler_kind_node {rnd_sampler_kind_node *Next; rnd_tex2d_sampler_kind Data;};

#define RndDefineBucketStacks \
rnd_clip_node *TopClip; \
rnd_transform_node *TopTransform; \
rnd_sampler_kind_node *TopSamplerKind; \

#define RndDefineBucketNodeNils \
global_variable rnd_clip_node NilClipNode = {0, {0}}; \
global_variable rnd_transform_node NilTransformNode = {0, {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}}; \
global_variable rnd_sampler_kind_node NilSamplerKindNode = {0, RND_TEX2D_SAMPLER_KIND_NEAREST}; \

#define RndBucketStackInits \
Bucket->TopClip = &NilClipNode; \
Bucket->TopTransform = &NilTransformNode; \
Bucket->TopSamplerKind = &NilSamplerKindNode; \

#define RndClipScope(Data) DeferLoop(RndPushClip(Data), RndPopClip())
#define RndTransformScope(Data) DeferLoop(RndPushTransform(Data), RndPopTransform())
#define RndSamplerKindScope(Data) DeferLoop(RndPushSamplerKind(Data), RndPopSamplerKind())

rng2_f32 RndPushClip(rng2_f32 Data);
mat3x3_f32 RndPushTransform(mat3x3_f32 Data);
rnd_tex2d_sampler_kind RndPushSamplerKind(rnd_tex2d_sampler_kind Data);
rng2_f32 RndPopClip();
mat3x3_f32 RndPopTransform();
rnd_tex2d_sampler_kind RndPopSamplerKind();
rng2_f32 RndTopClip();
mat3x3_f32 RndTopTransform();
rnd_tex2d_sampler_kind RndTopSamplerKind();

