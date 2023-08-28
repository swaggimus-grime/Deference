#pragma once

#include "RaytracingPipeline.h"

class DiffusePipeline : public RaytracingPipeline
{
public:
	DiffusePipeline(Graphics& g);

	static constexpr LPCWSTR shaderFile = L"shaders\\diffuse.rt.hlsl";
	static constexpr LPCWSTR rayGenEP = L"DiffuseAndHardShadow";

	static constexpr LPCWSTR shadowGroup = L"ShadowHit";
	static constexpr LPCWSTR shadowAny = L"ShadowAnyHit";

	static constexpr LPCWSTR shadowMiss = L"ShadowMiss";

};