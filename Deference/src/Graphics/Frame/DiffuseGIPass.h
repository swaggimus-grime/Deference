#pragma once

#include "RaytracePass.h"

namespace Def
{
	class ConstantBuffer;

	class DiffuseGIPass : public RaytracePass
	{
	public:
		DiffuseGIPass(Graphics& g, const std::string& name, FrameGraph* parent);
		virtual void ShowGUI() override;
		virtual void Run(Graphics& g) override;
		virtual void PrepLoadScene(Graphics& g) override;
		virtual void OnSceneLoad(Graphics& g) override;

	private:
		static constexpr LPCWSTR shaderFile = L"src\\Effects\\shaders\\diffuseGI.rt.hlsl";
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
		Unique<SamplerHeap> m_SamplerHeap;
		Shared<ConstantBuffer> m_Constants;
		Shared<ConstantBuffer> m_Light;
		UINT m_FrameCount;
	};
}