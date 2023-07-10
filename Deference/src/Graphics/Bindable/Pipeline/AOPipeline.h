#pragma once

#include "RaytracingPipeline.h"

class AOPipeline : public RaytracingPipeline
{
public:
	AOPipeline(Graphics& g);

	static constexpr LPCWSTR shaderFile = L"shaders\\ao.rt.hlsl";
	static constexpr LPCWSTR rayGenEP = L"AoRayGen";
	static constexpr LPCWSTR hitGroup = L"AoHit";
	static constexpr LPCWSTR anyEP = L"AoAnyHit";
	static constexpr LPCWSTR missEP = L"AoMiss";

private:
	Shared<RootSig> m_RayGenSig;
	Shared<RootSig> m_HitSig;
	Shared<RootSig> m_MissSig;
};