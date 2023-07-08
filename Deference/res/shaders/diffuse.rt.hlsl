Texture2D<float4> pos : register(t0);
Texture2D<float4> norm : register(t1);
Texture2D<float4> diff : register(t2);
RWTexture2D<float4> output : register(u0);

RaytracingAccelerationStructure scene : register(t3);

struct LightData
{
    float distToLight;
    float3 vToL;
    float3 dirToL;
};

LightData GetLightData(float3 vPos, float3 lPos)
{
    LightData data;
    float3 vToL = vPos + lPos;
    data.distToLight = length(vToL);
    data.dirToL = normalize(vToL);
    data.vToL = vToL;
    return data;
}

// Payload for our primary rays.  We really don't use this for this g-buffer pass
struct ShadowRayPayload
{
    int hitDist;
};

[shader("anyhit")]
void ShadowAnyHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	//// Run a Falcor helper to extract the hit point's geometric data
 //   VertexOut vsOut = getVertexAttributes(PrimitiveIndex(), attribs);

 //   // Extracts the diffuse color from the material (the alpha component is opacity)
 //   ExplicitLodTextureSampler lodSampler = { 0 }; // Specify the tex lod/mip to use here
 //   float4 baseColor = sampleTexture(gMaterial.resources.baseColor, gMaterial.resources.samplerState,
 //       vsOut.texC, gMaterial.baseColor, EXTRACT_DIFFUSE_TYPE(gMaterial.flags), lodSampler);

	//// Test if this hit point passes a standard alpha test.  If not, discard/ignore the hit.
 //   if (baseColor.a < gMaterial.alphaThreshold)
 //       IgnoreHit();

	// We update the hit distance with our current hitpoint
    rayData.hitDist = RayTCurrent();
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    rayData.hitDist = RayTCurrent();
}

float shadowRayVisibility(float3 origin, float3 direction, float minT, float maxT)
{
	// Setup our shadow ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = minT;
    ray.TMax = maxT;

	// Query if anything is between the current point and the light (i.e., at maxT) 
    ShadowRayPayload rayPayload = { maxT + 1.0f };
    TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 2, 0, ray, rayPayload);

	// Check if anyone was closer than our maxT distance (in which case we're occluded)
    return (rayPayload.hitDist > maxT) ? 1.0f : 0.0f;
}

[shader("raygeneration")]
void DiffuseAndHardShadow()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

    float4 wPos = pos[launchIndex];
    float4 wNorm = norm[launchIndex];
    float4 diffuse = diff[launchIndex];

	// If we don't hit any geometry, our difuse material contains our background color.
    float3 shadeColor = diffuse.rgb;

	//// Our camera sees the background if worldPos.w is 0, only shoot an AO ray elsewhere
    if (wPos.w != 0.0f)
    {
        shadeColor = float3(0.0, 0.0, 0.0);
    
        for (int lightIndex = 0; lightIndex < 1; lightIndex++)
        {
            LightData l = GetLightData(wPos.xyz, float3(0, 5, 0));
            // Compute our lambertion term (L dot N)
            float LdotN = saturate(dot(wNorm.xyz, l.vToL));
    
    	       // Shoot our ray
            float shadowMult = shadowRayVisibility(wPos.xyz, l.dirToL, 1.0e-4f, l.distToLight);
    
    	       // Compute our Lambertian shading color
            shadeColor += shadowMult * LdotN * 1; // 1 is our intensity
            shadeColor = float3(.6f, 1.f, 1.f);
        }
    
        // Physically based Lambertian term is albedo/pi
        shadeColor *= diffuse.rgb / 3.141592f;
    }
    
    output[launchIndex] = float4(shadeColor, 1.0f);
}