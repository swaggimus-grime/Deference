#pragma once

#include "Pass.h"
#include "Resource/UnorderedAccess.h"
#include "Resource/RaytracingPipeline.h"

namespace Def
{
	class RaytracePass : public Pass
	{
	public:
		virtual void OnResize(Graphics& g, UINT w, UINT h) override;

	protected:
		RaytracePass(const std::string& name, FrameGraph* parent);
		virtual void AddOutTarget(Graphics& g, const std::string& target, DXGI_FORMAT fmt = Swapchain::s_Format, XMFLOAT4 clearColor = {0.f, 0.f, 0.f, 1.f}) override;
		void CopyUAVsToRTs(Graphics& g);

	protected:
		struct HitEntry {
			UINT m;
			UINT istride;
			HGPU v;
			HGPU i;
			HGPU uv_0;
			HGPU uv_1;
			HGPU uv_2;
			HGPU n;
			HGPU t;
		};

	protected:
		Unique<RaytracingPipeline> m_Pipeline;
		std::vector<std::pair<std::string, Shared<UnorderedAccess>>> m_UAVs;
	};
}