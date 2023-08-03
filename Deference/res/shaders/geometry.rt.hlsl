#include "Vertex.hlsli"

#define M_PI  3.14159265358979323846264338327950288
#define M_1_PI  0.318309886183790671538

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

StructuredBuffer<VertexIn> gVertices : register(t0, space1);
ByteAddressBuffer gIndices : register(t1, space1);
Texture2D<float4> dmap : register(t2, space1);
Texture2D<float4> nmap : register(t3, space1);

struct Camera
{
    matrix viewInv;
    matrix projInv;
    float3 wPos;
};

ConstantBuffer<Camera> cam : register(b0);

float atan2_WAR(float y, float x)
{
    if (x > 0.f)
        return atan(y / x);
    else if (x < 0.f && y >= 0.f)
        return atan(y / x) + M_PI;
    else if (x < 0.f && y < 0.f)
        return atan(y / x) - M_PI;
    else if (x == 0.f && y > 0.f)
        return M_PI / 2.f;
    else if (x == 0.f && y < 0.f)
        return -M_PI / 2.f;
    return 0.f; // x==0 && y==0 (undefined)
}

float2 worldDirToPolorCoords(float3 dir)
{
    float3 p = normalize(dir);

	// atan2_WAR is a work-around due to an apparent compiler bug in atan2
    float u = (1.f + atan2_WAR(p.x, -p.z) * M_1_PI) * 0.5f;
    float v = acos(p.y) * M_1_PI;
    return float2(u, v);
}

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

struct ShadingData
{
    float3 wPos;
    float3 normal;
    float3 diffuse;
    float opacity;
};

[shader("anyhit")]
void PrimaryAnyHit(inout SimplePayload dummy, BuiltInTriangleIntersectionAttributes attribs)
{
	//if (!alphaTest(attribs)) IgnoreHit();
}

float2 BarycentricLerp2(in float2 v0, in float2 v1, in float2 v2, float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

float3 BarycentricLerp3(in float3 v0, in float3 v1, in float3 v2, float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

VertexIn BarycentricLerp(in VertexIn v0, in VertexIn v1, in VertexIn v2, in float3 barycentrics)
{
    VertexIn v;
    v.pos = BarycentricLerp3(v0.pos, v1.pos, v2.pos, barycentrics);
    v.norm = normalize(BarycentricLerp3(v0.norm, v1.norm, v2.norm, barycentrics));
    v.tex = BarycentricLerp2(v0.tex, v1.tex, v2.tex, barycentrics);
    v.tan = normalize(BarycentricLerp3(v0.tan, v1.tan, v2.tan, barycentrics));
    v.bitan = normalize(BarycentricLerp3(v0.bitan, v1.bitan, v2.bitan, barycentrics));

    return v;
}

ShadingData getShadingData(uint primIdx, BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    uint baseIndex = primIdx * 3;
    int address = baseIndex * 4;
    const uint3 indices = gIndices.Load3(address);
    
    VertexIn v = BarycentricLerp(
        gVertices[indices[0]],
        gVertices[indices[1]],
        gVertices[indices[2]], 
        barycentrics
    );
    
    ShadingData data;
    data.normal = v.norm;
    data.wPos = v.pos;
    
    float2 texDims;
    dmap.GetDimensions(texDims.x, texDims.y);
    float4 diffuse = dmap.Load(int3(floor(v.tex * texDims), 0));
    data.diffuse = diffuse.rgb;
    data.opacity = diffuse.a;
    
    return data;
}

[shader("closesthit")]
void PrimaryClosestHit(inout SimplePayload dummy, BuiltInTriangleIntersectionAttributes attribs)
{
    // Which pixel spawned our ray?
    uint2 idx = DispatchRaysIndex();

	// Run helper function to compute important data at the current hit point
    ShadingData shadeData = getShadingData(PrimitiveIndex(), attribs);

	// Save out our G-buffer values to the specified output textures
    gWsPos[idx] = float4(shadeData.wPos, 1.f);
    gWsNorm[idx] = float4(shadeData.normal, length(shadeData.wPos - cam.wPos));
    gMatDif[idx] = float4(shadeData.diffuse, shadeData.opacity);
    //gMatSpec[idx] = float4(shadeData.specular, shadeData.linearRoughness);
}

[shader("raygeneration")]
void GeometryRayGen()
{
	// Convert our ray index into a ray direction in world space. 
    float2 currenPixelLocation = DispatchRaysIndex().xy + float2(0.5f, 0.5f);
    float2 pixelCenter = currenPixelLocation / DispatchRaysDimensions().xy;
    float2 ndc = float2(2, -2) * pixelCenter + float2(-1, 1);
    float4 at = mul(float4(ndc, 1, 1), cam.projInv);
    float3 dir = mul(float4(normalize(at.xyz / at.w), 0), cam.viewInv).xyz;
    
	// Initialize a ray structure for our ray tracer
    RayDesc ray;
    ray.Origin = cam.wPos;
    ray.Direction = dir;
    ray.TMin = 0.0001f;
    ray.TMax = 1e+38f;

	// Initialize our ray payload (a per-ray, user-definable structure).
    SimplePayload payload = { false };

	// Trace our ray
    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
}