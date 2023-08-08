#include "Random.hlsli"
#include "Shading.hlsli"
#include "Trigonometry.hlsli"

Texture2D<float4> pos : register(t0);
Texture2D<float4> norm : register(t1);
Texture2D<float4> diff : register(t2);
RWTexture2D<float4> output : register(u0);

RaytracingAccelerationStructure scene : register(t3);
Texture2D<float4> env : register(t4);

struct PointLight
{
    float3 pos;
    float intensity;
    float3 color;
};

ConstantBuffer<PointLight> pointLight : register(b0);

struct Constants
{
    uint frameCount;
};
ConstantBuffer<Constants> constants : register(b1);

struct LightData
{
    float distToLight;
    float3 vToL;
    float3 dirToL;
};

LightData GetLightData(float3 vPos, float3 lPos)
{
    LightData data;
    float3 vToL = lPos - vPos;
    data.distToLight = length(vToL);
    data.dirToL = normalize(vToL);
    data.vToL = vToL;
    return data;
}

// Payload for our primary rays.  We really don't use this for this g-buffer pass
struct ShadowRayPayload
{
    float hitDist;
};

float shadowRayVisibility(float3 origin, float3 direction, float minT, float maxT)
{
	// Setup our shadow ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = minT;
    ray.TMax = maxT;

	// Query if anything is between the current point and the light (i.e., at maxT) 
    ShadowRayPayload rayPayload = { maxT + 1.f };
    TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
                  RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 1, 0, ray, rayPayload);

	// Check if anyone was closer than our maxT distance (in which case we're occluded)
    return (rayPayload.hitDist > maxT) ? 1.0f : 0.0f;
}

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

struct IndirectPayload
{
    float3 color; // The color in the ray's direction
    uint rndSeed; // Our current random seed
};

float3 shootIndirectRay(float3 orig, float3 dir, float minT, uint seed)
{
	// Setup shadow ray and the default ray payload
    RayDesc rayColor = { orig, minT, dir, 1.0e+38f };
    IndirectPayload load = { float3(0, 0, 0), seed };

	// Trace our indirect ray.  Use hit group #1 and miss shader #1 (of 2)
    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 1, 2, 1, rayColor, load);
    return load.color;
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
	// Extract data from our scene description to allow shading this point
    ShadingData shadeData = getShadingData(PrimitiveIndex(), attribs);

	// Pick a random light from our scene to shoot a shadow ray towards
    int lightToSample = min(int(1 * nextRand(rayData.rndSeed)),
                             1 - 1);
    
     // A helper to query the Falcor scene to get light data
    LightData l = GetLightData(shadeData.wPos, pointLight.pos);

     // Compute our Lambertion term (NL dot L)
    float NdotL = saturate(dot(shadeData.normal, l.vToL));

     // Shoot our ray.  Return 1.0 for lit, 0.0 for shadowed
    float vis = shadowRayVisibility(shadeData.wPos, l.vToL,
                                  RayTMin(), l.distToLight);

     // Return the shaded Lambertian shading at this indirect hit point
    rayData.color = vis * NdotL * pointLight.color * shadeData.diffuse / M_PI;
}

[shader("raygeneration")]
void DiffuseAndHardShadow()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

    float4 wPos = pos[launchIndex];
    float4 wNorm = norm[launchIndex];
    float4 diffuse = diff[launchIndex];
    
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, constants.frameCount);

	// If we don't hit any geometry, our difuse material contains our background color.
    float3 shadeColor = float3(0.0, 0.0, 0.0);

    if (wPos.w != 0.0f)
    {
        for (int lightIndex = 0; lightIndex < 1; lightIndex++)
        {
            LightData l = GetLightData(wPos.xyz, pointLight.pos);
            float LdotN = saturate(dot(l.vToL, wNorm.xyz));
            float shadowMult = shadowRayVisibility(wPos.xyz, l.dirToL, 0.0001, l.distToLight);
            shadeColor += shadowMult * LdotN * pointLight.color * pointLight.intensity * diffuse.rgb / M_PI;
            

            float3 bounceDir = getCosHemisphereSample(randSeed, wNorm.xyz);
            float NdotL = saturate(dot(wNorm.xyz, bounceDir));
            float3 bounceColor = shootIndirectRay(wPos.xyz, bounceDir,
                                        0.0001, randSeed);

            float sampleProb = NdotL / M_PI;
            shadeColor += (NdotL * bounceColor * diffuse.rgb / M_PI) / sampleProb;
        }
    }
    
    output[launchIndex] = float4(shadeColor, 1.0f);
}