#include "Trigonometry.hlsli"
#include "Shading.hlsli"

// A dummy payload for this simple ray; never used
struct SimplePayload
{
    bool dummyValue;
};

RWTexture2D<float4> gWsPos : register(u0);
RWTexture2D<float4> gWsNorm : register(u1);
RWTexture2D<float4> gMatDif : register(u2);

RaytracingAccelerationStructure scene : register(t0);
Texture2D<float4> env : register(t1);

struct Camera
{
    matrix projToWorld;
    matrix worldToProj;
    float3 wPos;
};

ConstantBuffer<Camera> cam : register(b0);

[shader("miss")]
void PrimaryMiss(inout SimplePayload dummy)
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    
    float2 texDims;
    env.GetDimensions(texDims.x, texDims.y);
    
    float2 uv = worldDirToPolorCoords(WorldRayDirection());
    gWsPos[launchIndex] = float4(0, 0, 0, 0);
    gWsNorm[launchIndex] = float4(0, 0, 0, 0);
    gMatDif[launchIndex] = env[uint2(uv * texDims)];
}

[shader("anyhit")]
void PrimaryAnyHit(inout SimplePayload dummy, BuiltInTriangleIntersectionAttributes attribs)
{
	//if (!alphaTest(attribs)) IgnoreHit();
}

[shader("closesthit")]
void PrimaryClosestHit(inout SimplePayload dummy, BuiltInTriangleIntersectionAttributes attribs)
{
    // Which pixel spawned our ray?
    uint2 idx = DispatchRaysIndex();

	// Run helper function to compute important data at the current hit point
    ShadingData shadeData = getShadingData(PrimitiveIndex(), attribs);

	// Save out our G-buffer values to the specified output textures
    gWsPos[idx] = float4(shadeData.wPos, 1);
    gWsNorm[idx] = float4(shadeData.normal, length(shadeData.wPos - cam.wPos));
    gMatDif[idx] = float4(shadeData.diffuse, shadeData.opacity);
    //gMatSpec[idx] = float4(shadeData.specular, shadeData.linearRoughness);
}

[shader("raygeneration")]
void GeometryRayGen()
{
    float2 currenPixelLocation = DispatchRaysIndex().xy + float2(0.5f, 0.5f);
    float2 pixelCenter = currenPixelLocation / DispatchRaysDimensions().xy;
    float2 ndc = float2(2, -2) * pixelCenter + float2(-1, 1);
    float4 world = mul(float4(ndc, 0, 1), cam.projToWorld);
    world.xyz /= world.w;

	// Initialize a ray structure for our ray tracer
    RayDesc ray = { cam.wPos, 0.0f, normalize(world.xyz - cam.wPos), 1e+38f };

	// Initialize our ray payload (a per-ray, user-definable structure).
    SimplePayload payload = { false };

	// Trace our ray
    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
}