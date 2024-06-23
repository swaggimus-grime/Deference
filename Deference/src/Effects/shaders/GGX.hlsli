/**********************************************************************************************************************
# Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#  * Redistributions of code must retain the copyright notice, this list of conditions and the following disclaimer.
#  * Neither the name of NVIDIA CORPORATION nor the names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT
# SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

#ifndef __GGX_HLSLI__
#define __GGX_HLSLI__

#include "Random.hlsli"
#include "Trigonometry.hlsli"
#include "Light.hlsli"
#include "Shadow.hlsli"
#include "Halton.hlsli"

struct ggxConstants
{
    float3 camPos;
    float minT;
    uint maxRec;
    uint frameCount;
    uint on;
    uint openScene;
};
ConstantBuffer<ggxConstants> ggx : register(b1);

struct IndirectRayPayload
{
    float3 color; // The color in the ray's direction
    uint rndSeed; // Our current random seed
    uint recDepth; // Recursion depth
    HaltonState hState; // Halton state
};

float3 shootIndirectRay(float3 rayOrigin, float3 rayDir, float minT, uint curPathLen, uint seed, HaltonState hState, uint curDepth)
{
	// Setup our indirect ray
    RayDesc rayColor;
    rayColor.Origin = rayOrigin; // Where does it start?
    rayColor.Direction = rayDir; // What direction do we shoot it?
    rayColor.TMin = minT; // The closest distance we'll count as a hit
    rayColor.TMax = 1.0e38f; // The farthest distance we'll count as a hit

	// Initialize the ray's payload data with black return color and the current rng seed
    IndirectRayPayload payload;
    payload.color = float3(0, 0, 0);
    payload.rndSeed = seed;
    payload.recDepth = curDepth + 1;
    payload.hState = hState;

	// Trace our ray to get a color in the indirect direction.  Use hit group #1 and miss shader #1
    TraceRay(scene, 0, 0xFF, 1, 2, 1, rayColor, payload);

	// Return the color we got from our ray
    return payload.color;
}

/** Returns a relative luminance of an input linear RGB color in the ITU-R BT.709 color space
    \param RGBColor linear HDR RGB color in the ITU-R BT.709 color space
*/
inline float luminance(float3 rgb)
{
    return dot(rgb, float3(0.2126f, 0.7152f, 0.0722f));
}

float probabilityToSampleDiffuse(float3 difColor, float3 specColor)
{
    float lumDiffuse = max(0.01f, luminance(difColor));
    float lumSpecular = max(0.01f, luminance(specColor));
    return lumDiffuse / (lumDiffuse + lumSpecular);
}

// Fresnel reflectance - schlick approximation.
float3 FresnelSchlick(in float3 I, in float3 N, in float3 f0)
{
    float cosi = saturate(dot(-I, N));
    return f0 + (1 - f0) * pow(1 - cosi, 5);
}

// Copy from Learn OpenGL
float DistributionGGX(in float3 N, in float3 H, in float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.1415926 * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(in float NdotV, in float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(in float3 N, in float3 V, in float3 L, in float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Get a GGX half vector / microfacet normal, sampled according to the distribution computed by
//     the function ggxNormalDistribution() above.  
//
// When using this function to sample, the probability density is pdf = D * NdotH / (4 * HdotV)
float3 getGGXMicrofacet(inout uint randSeed, float roughness, float3 hitNorm)
{
	// Get our uniform random numbers
    float2 randVal = float2(nextRand(randSeed), nextRand(randSeed));

	// Get an orthonormal basis from the normal
    float3 B = getPerpendicularVector(hitNorm);
    float3 T = cross(B, hitNorm);

	// GGX NDF sampling
    float a2 = roughness * roughness;
    float cosThetaH = sqrt(max(0.0f, (1.0 - randVal.x) / ((a2 - 1.0) * randVal.x + 1)));
    float sinThetaH = sqrt(max(0.0f, 1.0f - cosThetaH * cosThetaH));
    float phiH = randVal.y * M_PI * 2.0f;

	// Get our GGX NDF sample (i.e., the half vector)
    return T * (sinThetaH * cos(phiH)) + B * (sinThetaH * sin(phiH)) + hitNorm * cosThetaH;
}

float3 lambertianDirect(inout uint rndSeed, HaltonState hState, float3 hit, float3 norm, float3 difColor)
{
	// Pick a random light from our scene to shoot a shadow ray towards
    //int lightToSample = min(int(nextRand(rndSeed) * gLightsCount), gLightsCount - 1);

	// Query the scene to find info about the randomly selected light
    LightData light = GetLightData(hit, pointLight.pos);

	// Compute our lambertion term (L dot N)
    float LdotN = saturate(dot(norm, light.dirToL));

	// Shoot our shadow ray to our randomly selected light
    float shadowMult = shadowRayVisibility(rndSeed, hit, light.dirToL, ggx.minT, light.distToLight);

	// Return the Lambertian shading color using the physically based Lambertian term (albedo / pi)
    return shadowMult * LdotN * pointLight.intensity * difColor / M_PI;
}

float3 lambertianIndirect(inout uint rndSeed, HaltonState hState, float3 hit, float3 norm, float3 difColor, uint rayDepth)
{
	// Shoot a randomly selected cosine-sampled diffuse ray.
    float3 L = getCosHemisphereSample(rndSeed, norm);
    float3 bounceColor = shootIndirectRay(hit, L, ggx.minT, 0, rndSeed, hState, rayDepth);

	// Accumulate the color: (NdotL * incomingLight * difColor / pi) 
	// Probability of sampling:  (NdotL / pi)
    return bounceColor * difColor;
}

float3 ggxDirect(inout uint rndSeed, HaltonState hState, float3 hit, float3 N, float3 V, float3 dif, float3 spec, float rough)
{
    return lambertianDirect(rndSeed, hState, hit, N, dif) * dif;
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * 3.1415826 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float3 ggxIndirect(inout uint rndSeed, HaltonState hState, float3 hit, float3 N, float3 noNormalN, float3 V, float3 dif, float3 spec, float rough, uint rayDepth, bool inverseRoughness)
{
    if (!inverseRoughness)
    {
        rough = 1.0 - rough;
    }
	// We have to decide whether we sample our diffuse or specular/ggx lobe.
    float probDiffuse = probabilityToSampleDiffuse(dif, spec);
	//int chooseDiffuse = 0;

	// We'll need NdotV for both diffuse and specular...
    float NdotV = saturate(dot(N, V));

    
    if (frac(haltonNext(hState) + nextRand(rndSeed)) > rough)
    {
        return lambertianIndirect(rndSeed, hState, hit, N, dif, rayDepth);
		//return float3(0.0);
    }
	// Otherwise we randomly selected to sample our GGX lobe
    else
    {
        float rnd1 = frac(haltonNext(hState) + nextRand(rndSeed));
        float rnd2 = frac(haltonNext(hState) + nextRand(rndSeed));
        float2 Xi = float2(rnd1, rnd2);
        float3 H = ImportanceSampleGGX(Xi, N, 1.0 - rough);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float3 bounceColor = shootIndirectRay(hit, L, ggx.minT, 0, rndSeed, hState, rayDepth);
        return bounceColor;
		//return float3(0.0, 1.0, 0.0);
    }
}

//float3 ggxDirect(inout uint rndSeed, HaltonState hState, float3 hit, float3 N, float3 V, float3 dif, float3 spec, float rough)
//{
//	// Pick a random light from our scene to shoot a shadow ray towards

//	// Query the scene to find info about the randomly selected light
//    LightData light = GetLightData(hit, pointLight.pos);

//	// Compute our lambertion term (N dot L)
//    float NdotL = saturate(dot(N, light.dirToL));

//	// Shoot our shadow ray to our randomly selected light
//    float shadowMult = shadowRayVisibility(rndSeed, hit, light.dirToL, ggx.minT, light.distToLight);

//	// Compute half vectors and additional dot products for GGX
//    float3 H = normalize(V + light.dirToL);
//    float NdotH = saturate(dot(N, H));
//    float LdotH = saturate(dot(light.dirToL, H));
//    float NdotV = saturate(dot(N, V));

//	// Evaluate terms for our GGX BRDF model
//    float D = DistributionGGX(N, H, rough);
//    float G = GeometrySchlickGGX(NdotV, rough);
//    float3 F = FresnelSchlick(light.dirToL, N, spec);

//	// Evaluate the Cook-Torrance Microfacet BRDF model
//	//     Cancel out NdotL here & the next eq. to avoid catastrophic numerical precision issues.
//    float3 ggxTerm = D * G * F / (4 * NdotV /* * NdotL */);

//	// Compute our final color (combining diffuse lobe plus specular GGX lobe)
//    return shadowMult * pointLight.intensity * ( /* NdotL * */ggxTerm + NdotL * dif / M_PI);
//}


#endif