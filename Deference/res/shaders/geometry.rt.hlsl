#include "Trigonometry.hlsli"
#include "Shading.hlsli"
#include "Random.hlsli"

// A dummy payload for this simple ray; never used
struct SimplePayload
{
    bool dummyValue;
};

RWTexture2D<float4> gWsPos : register(u0);
RWTexture2D<float4> gWsNorm : register(u1);
RWTexture2D<float4> gMatDif : register(u2);
RWTexture2D<float4> gMatSpec : register(u3);
RWTexture2D<float4> gMatEmissive : register(u4);

RaytracingAccelerationStructure scene : register(t0);
Texture2D<float4> env : register(t1);

struct Camera
{
    float3 u;
    float lensRadius;
    float3 v;
    float focalLength;
    float3 w;
    uint frameCount;
    float4 wPos;
    float2 jitter;
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
    gMatSpec[launchIndex] = float4(0, 0, 0, 0);
    gMatEmissive[launchIndex] = float4(0, 0, 0, 0);
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
    gWsNorm[idx] = float4(shadeData.normal, length(shadeData.wPos - cam.wPos.xyz));
    gMatDif[idx] = float4(shadeData.diffuse, shadeData.opacity);
    gMatSpec[idx] = float4(shadeData.specular, shadeData.roughness);
    gMatEmissive[idx] = float4(shadeData.emissive, 1.f);
}

[shader("raygeneration")]
void GeometryRayGen()
{
    // Get our pixel's position on the screen
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

	// Convert our ray index into a ray direction in world space.  
    float2 pixelCenter = (launchIndex + cam.jitter) / launchDim;
    float2 ndc = float2(2, -2) * pixelCenter + float2(-1, 1);
    float3 rayDir = ndc.x * cam.u + ndc.y * cam.v + cam.w;

	// Find the focal point for this pixel.
    rayDir /= length(cam.w); // Make ray have length 1 along the camera's w-axis.
    float3 focalPoint = cam.wPos.xyz + cam.focalLength * rayDir; // Select point on ray a distance gFocalLen along the w-axis

	// Initialize a random number generator
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, cam.frameCount, 16);

	// Get random numbers (in polar coordinates), convert to random cartesian uv on the lens
    float2 rnd = float2(2.0f * 3.14159265f * nextRand(randSeed), cam.lensRadius * nextRand(randSeed));
    float2 uv = float2(cos(rnd.x) * rnd.y, sin(rnd.x) * rnd.y);

	// Use uv coordinate to compute a random origin on the camera lens
    float3 randomOrig = cam.wPos.xyz + uv.x * normalize(cam.u) + uv.y * normalize(cam.v);

	// Initialize a ray structure for our ray tracer
    RayDesc ray;
    ray.Origin = randomOrig; // Start our ray at the world-space camera position
    ray.Direction = normalize(focalPoint - randomOrig); // Our ray direction
    ray.TMin = 0.0f; // Start at 0.0; for camera, no danger of self-intersection
    ray.TMax = 1e+38f; // Maximum distance to look for a ray hit

	// Initialize our ray payload (a per-ray, user-definable structure)
    SimplePayload rayData = { false };

	// Trace our ray
    TraceRay(scene, // A Falcor built-in containing the raytracing acceleration structure
		RAY_FLAG_CULL_BACK_FACING_TRIANGLES, // Ray flags.  (Here, we will skip hits with back-facing triangles)
		0xFF, // Instance inclusion mask.  0xFF => no instances discarded from this mask
		0, // Hit group to index (i.e., when intersecting, call hit shader #0)
		1, // Number of hit groups ('hitProgramCount' is built-in from Falcor with the right number) 
		0, // Miss program index (i.e., when missing, call miss shader #0)
		ray, // Data structure describing the ray to trace
		rayData); // Our user-defined ray payload structure to store intermediate results
}