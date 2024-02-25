#pragma once

#include <array>
#include "RaytracePass.h"
#include "Resource/ConstantBuffer.h"
#include <random>

namespace Def
{
	class RaytracedGeometryPass : public RaytracePass
	{
	public:
		RaytracedGeometryPass(Graphics& g, const std::string& name, FrameGraph* parent);
		virtual void Run(Graphics& g) override;
		virtual void ShowGUI() override;

		virtual void PrepLoadScene(Graphics& g) override;
		virtual void OnSceneLoad(Graphics& g) override;

	private:
		static constexpr LPCWSTR shaderFile = L"src\\Effects\\shaders\\geometry.rt.hlsl";
		static constexpr LPCWSTR rayGenEP = L"GeometryRayGen";
		static constexpr LPCWSTR missEP = L"PrimaryMiss";
		static constexpr LPCWSTR hitGroup = L"PrimaryHit";
		static constexpr LPCWSTR closestEP = L"PrimaryClosestHit";
		static constexpr LPCWSTR anyEP = L"PrimaryAnyHit";

	private:
		Unique<SamplerHeap> m_SamplerHeap;
		Shared<ConstantBuffer> m_Transform;
		std::uniform_real_distribution<float> m_RngDist;
		std::mt19937 m_Rng;
		UINT m_FrameCount = 0;
		float m_FStop = 32.0f;
		float m_FocalLength = 32.0f;
		float m_LensRadius;
	};
}