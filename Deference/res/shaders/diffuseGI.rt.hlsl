Texture2D<float4> pos : register(t0);
Texture2D<float4> norm : register(t1);
Texture2D<float4> diff : register(t2);
Texture2D<float4> spec : register(t3);

RWTexture2D<float4> output : register(u0);

RaytracingAccelerationStructure scene : register(t4);
Texture2D<float4> env : register(t5);

struct ggxConstants
{
    float3 camPos;
    uint maxRec;
    uint frameCount;
};
ConstantBuffer<ggxConstants> constants : register(b1);

#include "Random.hlsli"
#include "Light.hlsli"
#include "Shadow.hlsli"
#include "GGX.hlsli"
#include "Shading.hlsli"
#include "Trigonometry.hlsli"

[shader("anyhit")]
void ShadowAnyHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    rayData.hitDist = RayTCurrent();
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
}

[shader("miss")]
void ShadowMiss(inout ShadowRayPayload rayData)
{
}

[shader("miss")]
void IndirectMiss(inout IndirectPayload rayData)
{
    float2 dims;
    env.GetDimensions(dims.x, dims.y);
    float2 uv = worldDirToPolorCoords(WorldRayDirection());
    rayData.color = env[uint2(uv * dims)].rgb;
}

// Identical to any hit shaders for most other rays we've defined
[shader("anyhit")]
void IndirectAny(inout IndirectPayload rayData,
                 BuiltInTriangleIntersectionAttributes attribs)
{
    //if (alphaTestFails(attribs))
      //  IgnoreHit();
}

[shader("closesthit")]
void IndirectClosest(inout IndirectPayload rayData,
                     BuiltInTriangleIntersectionAttributes attribs)
{
    // Run a helper functions to extract Falcor scene data for shading
    ShadingData shadeData = getShadingData(PrimitiveIndex(), attribs);

    // Add emissive color
    rayData.color = float3(0, 0, 0);

    float3 v = normalize(constants.camPos - shadeData.wPos);
    rayData.color += ggxDirect(rayData.rndSeed, shadeData.wPos, shadeData.normal, 
        v, shadeData.diffuse, shadeData.specular, 0.0);

    if (rayData.recDepth < constants.maxRec && nextRand(rayData.rndSeed) > 0.5f)
    {
        rayData.color += ggxIndirect(rayData.rndSeed, shadeData.wPos, shadeData.normal, v,
			shadeData.diffuse, shadeData.specular, shadeData.roughness, rayData.recDepth);
    }
    
}

[shader("raygeneration")]
void DiffuseAndHardShadow()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

    float4 wPos = pos[launchIndex];
    float4 wNorm = norm[launchIndex];
    float4 diffuse = diff[launchIndex];
    float4 specular = spec[launchIndex];
    
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, constants.frameCount);

	// If we don't hit any geometry, our difuse material contains our background color.
    float3 shadeColor = float3(0.0, 0.0, 0.0);

    if (wPos.w != 0.0f)
    { 
        float3 v = normalize(constants.camPos - wPos.xyz);
        float roughness = specular.a * specular.a;
        
        float3 directColor;
        shadeColor += ggxDirect(randSeed, wPos.xyz, wNorm.xyz, v,
				               diffuse.rgb, specular.rgb, roughness);
        
        if(constants.maxRec > 0)
            shadeColor += ggxIndirect(randSeed, wPos.xyz, wNorm.xyz, v, 
                diffuse.rgb, specular.rgb, roughness, 0);
    }
    
    bool colorsNan = any(isnan(shadeColor));
    
    output[launchIndex] = float4(colorsNan ? float3(0, 0, 0) : shadeColor, 1.0f);
}