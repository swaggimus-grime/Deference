#ifndef __TRANSFORM_HLSLI__
#define __TRANSFORM_HLSLI__

struct ModelTransform
{
    matrix model;
    matrix vp;
    float3x3 normMat;
};

ConstantBuffer<ModelTransform> modelTransform : register(b0);

#endif 