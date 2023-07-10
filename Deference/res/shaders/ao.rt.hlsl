// Input and out textures that need to be set by the C++ code
Texture2D<float4> gPos : register(t0);
Texture2D<float4> gNorm : register(t1);
RWTexture2D<float4> gOutput : register(u0);

RaytracingAccelerationStructure scene : register(t2);

struct AORayPayload
{
    float aoValue; // Store 0 if we hit a surface, 1 if we miss all surfaces
};

// A constant buffer we'll populate from our C++ code 
struct RayGenCB
{
    float gAORadius;
    uint gFrameCount;
    float gMinT;
};

ConstantBuffer<RayGenCB> aoConstants : register(b0);

// A wrapper function that encapsulates shooting an ambient occlusion ray query
float shootAmbientOcclusionRay(float3 orig, float3 dir, float minT, float maxT)
{
	// Setup ambient occlusion ray and payload
    AORayPayload rayPayload = { 0.0f }; // Specified value is returned if we hit a surface
    RayDesc rayAO;
    rayAO.Origin = orig; // Where does our ray start?
    rayAO.Direction = dir; // What direction does our ray go?
    rayAO.TMin = minT; // Min distance to detect an intersection
    rayAO.TMax = maxT; // Max distance to detect an intersection

	// Trace our ray.  Ray stops after it's first definite hit; never execute closest hit shader
    TraceRay(scene,
		RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
		0xFF, 0, 1, 0, rayAO, rayPayload);

	// Copy our AO value out of the ray payload.
    return rayPayload.aoValue;
}

uint initRand(uint val0, uint val1, uint backoff = 16)
{
    uint v0 = val0, v1 = val1, s0 = 0;

	[unroll]
    for (uint n = 0; n < backoff; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

// Takes our seed, updates it, and returns a pseudorandom float in [0..1]
float nextRand(inout uint s)
{
    s = (1664525u * s + 1013904223u);
    return float(s & 0x00FFFFFF) / float(0x01000000);
}

// Utility function to get a vector perpendicular to an input vector 
//    (from "Efficient Construction of Perpendicular Vectors Without Branching")
float3 getPerpendicularVector(float3 u)
{
    float3 a = abs(u);
    uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
    uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
    uint zm = 1 ^ (xm | ym);
    return cross(u, float3(xm, ym, zm));
}

// Get a cosine-weighted random vector centered around a specified normal direction.
float3 getCosHemisphereSample(inout uint randSeed, float3 hitNorm)
{
	// Get 2 random numbers to select our sample with
    float2 randVal = float2(nextRand(randSeed), nextRand(randSeed));

	// Cosine weighted hemisphere sample from RNG
    float3 bitangent = getPerpendicularVector(hitNorm);
    float3 tangent = cross(bitangent, hitNorm);
    float r = sqrt(randVal.x);
    float phi = 2.0f * 3.14159265f * randVal.y;

	// Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(1 - randVal.x);
}

// How do we generate the rays that we trace?
[shader("raygeneration")]
void AoRayGen()
{
	// Where is this thread's ray on screen?
    uint2 launchIndex = DispatchRaysIndex();
    uint2 launchDim = DispatchRaysDimensions();

	// Initialize a random seed, per-pixel, based on a screen position and temporally varying count
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, aoConstants.gFrameCount);

	// Load the position and normal from our g-buffer
    float4 worldPos = gPos[launchIndex];
    float4 worldNorm = gNorm[launchIndex];

	// Default ambient occlusion value if we hit the background
    float aoVal = 1.0f;

  // worldPos.w == 0 for background pixels; only shoot AO rays elsewhere
    if (worldPos.w != 0.0f)
    {
        // Random ray, sampled on cosine-weighted hemisphere around normal 
        float3 worldDir = getCosHemisphereSample(randSeed, worldNorm.xyz);

         // Shoot our ambient occlusion ray and update the final AO value
        aoVal = shootAmbientOcclusionRay(worldPos.xyz, worldDir, aoConstants.gMinT, aoConstants.gAORadius);
    }

    gOutput[launchIndex] = float4(aoVal, aoVal, aoVal, 1.0f);
}



// What code is executed when our ray misses all geometry?
[shader("miss")]
void AoMiss(inout AORayPayload rayData)
{
	// Our ambient occlusion value is 1 if we hit nothing.
    rayData.aoValue = 1.0f;
}

// What code is executed when our ray hits a potentially transparent surface?
[shader("anyhit")]
void AoAnyHit(inout AORayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	// Is this a transparent part of the surface?  If so, ignore this hit
    //if (alphaTestFails(attribs))
        //IgnoreHit();
}
