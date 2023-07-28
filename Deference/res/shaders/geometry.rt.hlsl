//#include "Vertex.hlsli"

//#define M_1_PI  0.318309886183790671538

//// A dummy payload for this simple ray; never used
//struct SimplePayload
//{
//    bool dummyValue;
//};

//RWTexture2D<float4> gWsPos : register(u0);
//RWTexture2D<float4> gWsNorm : register(u1);
//RWTexture2D<float4> gMatDif : register(u2);

//RaytracingAccelerationStructure scene : register(t0);
//Texture2D<float4> env : register(t1);

//ByteAddressBuffer gVertices[] : register(t0, space1);
//ByteAddressBuffer gIndices[] : register(t0, space2);
//Texture2D<float4> textures[] : register(t0, space3);

//struct Material
//{
//    int diffMapIndex;
//    int specMapIndex;
//    int normMapIndex;
//    int meshIdx;
//};

//ConstantBuffer<Material> material : register(b0);

//struct Camera
//{
//    matrix viewInv;
//    matrix projInv;
//    float3 wPos;
//};

//ConstantBuffer<Camera> cam : register(b1);

//float atan2_WAR(float y, float x)
//{
//    if (x > 0.f)
//        return atan(y / x);
//    else if (x < 0.f && y >= 0.f)
//        return atan(y / x) + M_PI;
//    else if (x < 0.f && y < 0.f)
//        return atan(y / x) - M_PI;
//    else if (x == 0.f && y > 0.f)
//        return M_PI / 2.f;
//    else if (x == 0.f && y < 0.f)
//        return -M_PI / 2.f;
//    return 0.f; // x==0 && y==0 (undefined)
//}

//float2 worldDirToPolorCoords(float3 dir)
//{
//    float3 p = normalize(dir);

//	// atan2_WAR is a work-around due to an apparent compiler bug in atan2
//    float u = (1.f + atan2_WAR(p.x, -p.z) * M_1_PI) * 0.5f;
//    float v = acos(p.y) * M_1_PI;
//    return float2(u, v);
//}

//[shader("miss")]
//void PrimaryMiss(inout SimplePayload dummy)
//{
//    uint2 launchIndex = DispatchRaysIndex().xy;
    
//    float2 texDims;
//    env.GetDimensions(texDims.x, texDims.y);
    
//    float2 uv = worldDirToPolorCoords(WorldRayDirection());
//    gMatDif[launchIndex] = env[uint2(uv * texDims)];
//}

//struct ShadingData
//{
//    float3 wPos;
//    float3 normal;
//    float3 diffuse;
//    float opacity;
//};

//uint3 getIndices(uint triangleIndex)
//{
//    uint baseIndex = triangleIndex * 3;
//    int address = baseIndex * 4;
//    return gIndices.Load3(address);
//}

//GeometryVertex getVertexAttributes(uint primIdx, BuiltInTriangleIntersectionAttributes attribs)
//{
//    uint3 indices = getIndices(primIdx);
//    GeometryVertex v;
//    v.pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    
//    [unroll]
//    for (int i = 0; i < 3; i++)
//    {
//        int address = (indices[i] * 3) * 4;
//        v.tex = asfloat(gTexCrds.Load2(address)) * barycentrics[i];
//        v.normalW += asfloat(gNormals.Load3(address)) * barycentrics[i];
//        v.bitangentW += asfloat(gBitangents.Load3(address)) * barycentrics[i];
//        v.lightmapC += asfloat(gLightMapUVs.Load2(address)) * barycentrics[i];
//#ifdef USE_INTERPOLATED_POSITION
//        v.posW       += asfloat(gPositions.Load3(address))   * barycentrics[i];
//#endif
//    }
//#ifdef USE_INTERPOLATED_POSITION
//    v.posW = mul(float4(v.posW, 1.f), gWorldMat[0]).xyz;
//#endif
//#ifndef _MS_DISABLE_INSTANCE_TRANSFORM
//    // Transform normal/bitangent to world space
//    // PETRIK TODO: LightTransport relies on getVertexAttributes() returning normalW and bitangentW in world space (as their names imply). If this is made the default, the Falcor RT sample(s) need to be updated.
//    v.normalW = mul(v.normalW, (float3x3) gWorldInvTransposeMat[0]).xyz;
//    v.bitangentW = mul(v.bitangentW, (float3x3) gWorldMat[0]).xyz;
//#endif
//    v.normalW = normalize(v.normalW);
//    v.bitangentW = normalize(v.bitangentW);
//    return v;
//}

//bool alphaTest(BuiltInTriangleIntersectionAttributes attribs)
//{
//    GeometryVertex vsOut = getVertexAttributes(PrimitiveIndex(), attribs);
//    return (vsOut.diffuse.a < 0.1);
//}

//[shader("anyhit")]
//void PrimaryAnyHit(inout SimplePayload dummy, BuiltInTriangleIntersectionAttributes attribs)
//{
//	if (!alphaTest(attribs)) IgnoreHit();
//}

//ShadingData getShadingData(uint primIdx, BuiltInTriangleIntersectionAttributes attribs)
//{
//    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    
//}

//[shader("closesthit")]
//void PrimaryClosest(inout SimplePayload dummy, BuiltInTriangleIntersectionAttributes attribs)
//{
//    // Which pixel spawned our ray?
//    uint2 idx = DispatchRaysIndex();

//	// Run helper function to compute important data at the current hit point
//    ShadingData shadeData = getShadingData(PrimitiveIndex(), attribs);

//	// Save out our G-buffer values to the specified output textures
//    gWsPos[idx] = float4(shadeData.wPos, 1.f);
//    gWsNorm[idx] = float4(shadeData.normal, length(shadeData.wPos - cam.wPos));
//    gMatDif[idx] = float4(shadeData.diffuse, shadeData.opacity);
//    //gMatSpec[idx] = float4(shadeData.specular, shadeData.linearRoughness);
//}

//[shader("raygeneration")]
//void GeometryRayGen()
//{
//	// Convert our ray index into a ray direction in world space. 
//    float2 currenPixelLocation = DispatchRaysIndex().xy + float2(0.5f, 0.5f);
//    float2 pixelCenter = currenPixelLocation / DispatchRaysDimensions().xy;
//    float2 ndc = float2(2, -2) * pixelCenter + float2(-1, 1);
//    float4 at = mul(float4(ndc, 1, 1), cam.projInv);
//    float3 dir = mul(float4(normalize(at.xyz / at.w), 0), cam.viewInv).xyz;
    
//	// Initialize a ray structure for our ray tracer
//    RayDesc ray = { cam.wPos, 0.0f, dir, 1e+38f };

//	// Initialize our ray payload (a per-ray, user-definable structure).
//    SimplePayload payload = { false };

//	// Trace our ray
//    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
//}