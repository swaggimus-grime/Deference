Texture2D pos : register(t0);
Texture2D norm : register(t1);
Texture2D diff : register(t2);
Texture2D spec : register(t3);
Texture2D emissive : register(t4);

RWTexture2D<float4> output : register(u0);

RaytracingAccelerationStructure scene : register(t5);
Texture2D env : register(t6);

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
    //float2 dims;
    //env.GetDimensions(dims.x, dims.y);
    //float2 uv = worldDirToPolorCoords(WorldRayDirection());
    //rayData.color = env[uint2(uv * dims)].rgb;
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
    rayData.color = shadeData.emissive * pointLight.emissive;

    float3 v = normalize(constants.camPos - shadeData.wPos);
    rayData.color += ggxDirect(rayData.rndSeed, shadeData.wPos, shadeData.normal, 
        v, shadeData.diffuse, shadeData.specular, shadeData.roughness);

    if (rayData.recDepth < constants.maxRec /*&& nextRand(rayData.rndSeed) > 0.5f*/)
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
    
    float3 v = normalize(constants.camPos - wPos.xyz);
    
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, constants.frameCount);
    
    // If we don't hit any geometry, our difuse material contains our background color.
    float3 shadeColor = emissive[launchIndex].rgb * pointLight.emissive;
    
    if (wPos.w != 0.0f)
    { 
        float roughness = specular.a * specular.a;
    
        shadeColor += ggxDirect(randSeed, wPos.xyz, wNorm.xyz, v,
    			               diffuse.rgb, specular.rgb, roughness);
    
        if(constants.maxRec > 0)
            shadeColor += ggxIndirect(randSeed, wPos.xyz, wNorm.xyz, v, 
                diffuse.rgb, specular.rgb, roughness, 0);
        //shadeColor += pointLight.intensity * max(dot(normalize(normalize(pointLight.pos - wPos.xyz) + v), wNorm.xyz), 0) * diffuse.xyz;
    }
    
    bool colorsNan = any(isnan(shadeColor));
    output[launchIndex] = float4(colorsNan ? float3(0, 0, 0) : shadeColor, 1.0f);
}