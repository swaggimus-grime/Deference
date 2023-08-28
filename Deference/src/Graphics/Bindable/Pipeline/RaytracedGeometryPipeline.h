#pragma once

#include "RaytracingPipeline.h"

class RaytracedGeometryPipeline : public RaytracingPipeline
{
public:
	RaytracedGeometryPipeline(Graphics& g);

	static constexpr LPCWSTR shaderFile = L"shaders\\geometry.rt.hlsl";
	static constexpr LPCWSTR rayGenEP = L"GeometryRayGen";
	static constexpr LPCWSTR missEP = L"PrimaryMiss";
	static constexpr LPCWSTR hitGroup = L"PrimaryHit";
	static constexpr LPCWSTR closestEP = L"PrimaryClosestHit";
	static constexpr LPCWSTR anyEP = L"PrimaryAnyHit";

};