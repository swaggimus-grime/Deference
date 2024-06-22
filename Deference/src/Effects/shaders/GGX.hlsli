#ifndef __GGX_HLSLI__
#define __GGX_HLSLI__

#include "Random.hlsli"
#include "Trigonometry.hlsli"
#include "Light.hlsli"
#include "Shadow.hlsli"

struct ggxConstants
{
    float3 camPos;
    float minT;
    uint maxRec;
    uint frameCount;
    uint on;
};
ConstantBuffer<ggxConstants> ggx : register(b1);

struct IndirectPayload
{
    float3 color; // The color in the ray's direction
    uint rndSeed; // Our current random seed
    uint recDepth; // Recursion depth
};

float3 shootIndirectRay(float3 orig, float3 dir, float minT, uint seed, uint recDepth)
{
	// Setup shadow ray and the default ray payload
    RayDesc rayColor = { orig, minT, dir, 1.0e+38f };
    IndirectPayload load;
    load.color = float3(0, 0, 0);
    load.rndSeed = seed;
    load.recDepth = recDepth + 1;

	// Trace our indirect ray.  Use hit group #1 and miss shader #1 (of 2)
    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 1, 2, 1, rayColor, load);
    return load.color;
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

// The NDF for GGX, see Eqn 19 from 
//    http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
//
// This function can be used for "D" in the Cook-Torrance model:  D*G*F / (4*NdotL*NdotV)
float ggxNormalDistribution(float NdotH, float roughness)
{
    float a2 = roughness * roughness;
    float d = ((NdotH * a2 - NdotH) * NdotH + 1);
    return a2 / max(0.001f, (d * d * M_PI));
}

// This from Schlick 1994, modified as per Karas in SIGGRAPH 2013 "Physically Based Shading" course
//
// This function can be used for "G" in the Cook-Torrance model:  D*G*F / (4*NdotL*NdotV)
float ggxSchlickMaskingTerm(float NdotL, float NdotV, float roughness)
{
	// Karis notes they use alpha / 2 (or roughness^2 / 2)
    float k = roughness * roughness / 2;

	// Karis also notes they can use the following equation, but only for analytical lights
	//float k = (roughness + 1)*(roughness + 1) / 8; 

	// Compute G(v) and G(l).  These equations directly from Schlick 1994
	//     (Though note, Schlick's notation is cryptic and confusing.)
    float g_v = NdotV / (NdotV * (1 - k) + k);
    float g_l = NdotL / (NdotL * (1 - k) + k);

	// Return G(v) * G(l)
    return g_v * g_l;
}

// Traditional Schlick approximation to the Fresnel term (also from Schlick 1994)
//
// This function can be used for "F" in the Cook-Torrance model:  D*G*F / (4*NdotL*NdotV)
float3 schlickFresnel(float3 f0, float u)
{
    return f0 + (float3(1.0f, 1.0f, 1.0f) - f0) * pow(1.0f - u, 5.0f);
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

float3 ggxDirect(inout uint rndSeed, float3 hit, float3 N, float3 V,
                 float3 dif, float3 spec, float rough)
{
	// Pick a random light from our scene to shoot a shadow ray towards
    //int lightToSample = min(int(nextRand(rndSeed) * gLightsCount),
    //                         gLightsCount - 1);
    LightData l = GetLightData(hit, pointLight.pos);

	// Compute our lambertion term (N dot L)
    float NdotL = saturate(dot(N, l.dirToL));

	// Shoot our shadow ray to our randomly selected light
    bool shadowMult = shadowRayVisibility(hit, l.dirToL, ggx.minT, l.distToLight) != 0.f;

	// Compute half vectors and additional dot products for GGX
    float3 H = normalize(V + l.dirToL);
    float NdotH = saturate(dot(N, H));
    float LdotH = saturate(dot(l.dirToL, H));
    float NdotV = saturate(dot(N, V));

	// Evaluate terms for our GGX BRDF model
    float D = ggxNormalDistribution(NdotH, rough);
    float G = ggxSchlickMaskingTerm(NdotL, NdotV, rough);
    float3 F = schlickFresnel(spec, LdotH);

	// Evaluate the Cook-Torrance Microfacet BRDF model
	//     Cancel NdotL here to avoid catastrophic numerical precision issues.
    float3 ggxTerm = D * G * F / (4 * NdotV /* * NdotL */);

	// Compute our final color (combining diffuse lobe plus specular GGX lobe)
    return shadowMult * pointLight.intensity * pointLight.color * ( /* NdotL * */ggxTerm + NdotL * dif / M_PI);
}

float3 ggxIndirect(inout uint rndSeed, float3 hit, float3 N, float3 V, float3 dif, float3 spec, float rough, uint rayDepth)
{
	// We have to decide whether we sample our diffuse or specular/ggx lobe.
    float probDiffuse = probabilityToSampleDiffuse(dif, spec);
    float chooseDiffuse = (nextRand(rndSeed) < probDiffuse);

	// We'll need NdotV for both diffuse and specular...
    float NdotV = dot(N, V);

	// If we randomly selected to sample our diffuse lobe...
    if (chooseDiffuse)
    {
		// Shoot a randomly selected cosine-sampled diffuse ray.
        float3 L = getCosHemisphereSample(rndSeed, N);
        float3 bounceColor = shootIndirectRay(hit, L, ggx.minT, rndSeed, rayDepth);

		// Accumulate the color: (NdotL * incomingLight * dif / pi) 
		// Probability of sampling:  (NdotL / pi) * probDiffuse
        return bounceColor * dif / probDiffuse;
    }
	// Otherwise we randomly selected to sample our GGX lobe
    else
    {
		// Randomly sample the NDF to get a microfacet in our BRDF to reflect off of
        float3 H = getGGXMicrofacet(rndSeed, rough, N);

		// Compute the outgoing direction based on this (perfectly reflective) microfacet
        float3 L = normalize(2.f * dot(V, H) * H - V);

		// Compute our color by tracing a ray in this direction
        float3 bounceColor = shootIndirectRay(hit, L, ggx.minT, rndSeed, rayDepth);

		// Compute some dot products needed for shading
        float NdotL = dot(N, L);
        float NdotH = dot(N, H);
        float LdotH = dot(L, H);

		// Evaluate our BRDF using a microfacet BRDF model
        float D = ggxNormalDistribution(NdotH, rough); // The GGX normal distribution
        float G = ggxSchlickMaskingTerm(NdotL, NdotV, rough); // Use Schlick's masking term approx
        float3 F = schlickFresnel(spec, LdotH); // Use Schlick's approx to Fresnel
        float3 ggxTerm = D * G * F / (4 * NdotL * NdotV); // The Cook-Torrance microfacet BRDF

		// What's the probability of sampling vector H from getGGXMicrofacet()?
        float ggxProb = D * NdotH / (4 * LdotH);

		// Accumulate the color:  ggx-BRDF * incomingLight * NdotL / probability-of-sampling
		//    -> Should really simplify the math above.
        return NdotL * bounceColor * ggxTerm / (ggxProb * (1.0f - probDiffuse));
    }
}

#endif