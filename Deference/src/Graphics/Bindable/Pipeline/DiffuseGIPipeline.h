#pragma once

#include "RaytracingPipeline.h"

class DiffuseGIPipeline : public RaytracingPipeline
{
public:
	DiffuseGIPipeline(Graphics& g);

	static constexpr LPCWSTR shaderFile = L"shaders\\diffuseGI.rt.hlsl";
	static constexpr LPCWSTR rayGenEP = L"DiffuseAndHardShadow";

	static constexpr LPCWSTR shadowGroup = L"ShadowHit";
	static constexpr LPCWSTR shadowAny = L"ShadowAnyHit";
	static constexpr LPCWSTR shadowClosest = L"ShadowClosestHit";

	static constexpr LPCWSTR indirectGroup = L"IndirectHit";
	static constexpr LPCWSTR indirectAny = L"IndirectAny";
	static constexpr LPCWSTR indirectClosest = L"IndirectClosest";

	static constexpr LPCWSTR shadowMiss = L"ShadowMiss";
	static constexpr LPCWSTR indirectMiss = L"IndirectMiss";

private:
	std::vector<Unique<RootSig>> m_Sigs;
};