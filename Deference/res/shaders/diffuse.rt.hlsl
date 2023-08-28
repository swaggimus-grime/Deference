Texture2D<float4> pos : register(t0);
Texture2D<float4> norm : register(t1);
Texture2D<float4> diff : register(t2);
RWTexture2D<float4> output : register(u0);

RaytracingAccelerationStructure scene : register(t3);

struct PointLight
{
    float3 pos;
    float intensity;
    float3 color;
};

ConstantBuffer<PointLight> pointLight : register(b0);

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

[shader("anyhit")]
void ShadowAnyHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    rayData.hitDist = RayTCurrent();
}

[shader("miss")]
void ShadowMiss(inout ShadowRayPayload rayData)
{
    
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
    ShadowRayPayload rayPayload = { maxT + 1.f };
    TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
                  RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 1, 0, ray, rayPayload);

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
    float3 shadeColor = float3(0.0, 0.0, 0.0);

    if (wPos.w != 0.0f)
    {
        for (int lightIndex = 0; lightIndex < 1; lightIndex++)
        {
            LightData l = GetLightData(wPos.xyz, pointLight.pos);
            // Compute our lambertion term (L dot N)
            float LdotN = saturate(dot(l.vToL, wNorm.xyz));
    
    	       // Shoot our ray
            float shadowMult = shadowRayVisibility(wPos.xyz, l.dirToL, 0.0001, l.distToLight);
    
    	       // Compute our Lambertian shading color
            shadeColor += shadowMult * LdotN * pointLight.color * pointLight.intensity;

        }
    
        // Physically based Lambertian term is albedo/pi
        shadeColor *= diffuse.rgb / 3.141592f;
    }
    else
        shadeColor += diffuse.rgb;
    
    output[launchIndex] = float4(shadeColor, 1.0f);
}