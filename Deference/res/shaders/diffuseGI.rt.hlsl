Texture2D pos : register(t0);
Texture2D norm : register(t1);
Texture2D diff : register(t2);
Texture2D spec : register(t3);
Texture2D emissive : register(t4);
RaytracingAccelerationStructure scene : register(t5);
Texture2D env : register(t6);
RWTexture2D<float4> output : register(u0);

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
    rayData.color = shadeData.emissive * pointLight.emissive;

    float3 v = normalize(ggx.camPos - shadeData.wPos);
    rayData.color += ggxDirect(rayData.rndSeed, shadeData.wPos, shadeData.normal, 
        v, shadeData.diffuse, shadeData.specular, shadeData.roughness);

    if (rayData.recDepth < ggx.maxRec)
    {
        rayData.color += ggxIndirect(rayData.rndSeed, shadeData.wPos, shadeData.normal, v,
			shadeData.diffuse, shadeData.specular, shadeData.roughness, rayData.recDepth);
    }
    
}

[shader("raygeneration")]
void DiffuseAndHardShadow()
{
    // Where is this ray on screen?
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

	// Load g-buffer data
    float4 worldPos = pos[launchIndex];
    float4 worldNorm = norm[launchIndex];
    float4 difMatlColor = diff[launchIndex];
    float4 specMatlColor = spec[launchIndex];
    float4 pixelEmissive = emissive[launchIndex];
	
	// Does this g-buffer pixel contain a valid piece of geometry?  (0 in pos.w for invalid)
    bool isGeometryValid = (worldPos.w != 0.0f);

	// Extract and compute some material and geometric parameters
    float roughness = specMatlColor.a * specMatlColor.a;
    float3 V = normalize(ggx.camPos - worldPos.xyz);

	// Make sure our normal is pointed the right direction
    if (dot(worldNorm.xyz, V) <= 0.0f)
        worldNorm.xyz = -worldNorm.xyz;
    float NdotV = dot(worldNorm.xyz, V);

	//// Grab our geometric normal.  Also make sure this points the right direction.
	////     This is badly hacked into our G-buffer for now.  We need this because 
	////     sometimes, when normal mapping, our randomly selected indirect ray will 
	////     be *below* the surface (due to the normal map perturbations), which will 
	////     cause light leaking.  We solve by ignoring the ray's contribution if it
	////     is below the horizon.  
 //   float3 noMapN = normalize(extraData.yzw);
 //   if (dot(noMapN, V) <= 0.0f)
 //       noMapN = -noMapN;

	// If we don't hit any geometry, our difuse material contains our background color.
    float3 shadeColor = isGeometryValid ? float3(0, 0, 0) : difMatlColor.rgb;

	// Initialize our random number generator
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, ggx.frameCount, 16);

	// Do shading, if we have geoemtry here (otherwise, output the background color)
    if (isGeometryValid)
    {
        // Add any emissive color from primary rays
        shadeColor = pointLight.emissive * pixelEmissive.rgb;

		// (Optionally) do explicit direct lighting to a random light in the scene
        shadeColor += ggxDirect(randSeed, worldPos.xyz, worldNorm.xyz, V,
				               difMatlColor.rgb, specMatlColor.rgb, roughness);

		// (Optionally) do indirect lighting for global illumination
        if (ggx.maxRec > 0)
            shadeColor += ggxIndirect(randSeed, worldPos.xyz, worldNorm.xyz, V, difMatlColor.rgb, specMatlColor.rgb, roughness, 0);
    }
	
	// Since we didn't do a good job above catching NaN's, div by 0, infs, etc.,
	//    zero out samples that would blow up our frame buffer.  Note:  You can and should
	//    do better, but the code gets more complex with all error checking conditions.
    bool colorsNan = any(isnan(shadeColor));

	// Store out the color of this shaded pixel
    output[launchIndex] = float4(colorsNan ? float3(0, 0, 0) : shadeColor, 1.0f);
}