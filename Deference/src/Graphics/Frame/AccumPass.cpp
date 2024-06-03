#include "AccumPass.h"
#include "Scene/Camera.h"
#include "FrameGraph.h"
#include "VecOps.h"

namespace Def
{
	AccumPass::AccumPass(Graphics& g, const std::string& name, FrameGraph* parent)
		:ScreenPass(g, std::move(name), parent), m_NumPassedFrames(0)
	{
		AddInTarget("Target");
		AddOutTarget(g, "Target");

		VertexShader vs(L"src\\Effects\\shaders\\screen.vs.hlsl");
		PixelShader ps(L"src\\Effects\\shaders\\accum.ps.hlsl");

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

		auto& outs = GetOutTargets();
		const auto& formats =
			std::views::iota(outs.begin(), outs.end()) |
			std::views::transform([&](const auto& it) {
			return (*it).second->GetFormat();
				}) |
			std::ranges::to<std::vector>();

		m_Pipeline = MakeUnique<Pipeline>(g, MakeShared<RootSig>(g, _countof(rootParameters), rootParameters), vs, ps, *m_Layout, formats);

		m_PrevFrame = MakeShared<RenderTarget>(g);
		const auto barrier = m_PrevFrame->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g.CL().ResourceBarrier(1, &barrier);
		AddResource(m_PrevFrame);
	}

	void AccumPass::ShowGUI()
	{
	}

	void AccumPass::OnResize(Graphics& g, UINT w, UINT h)
	{
		__super::OnResize(g, w, h);
		m_PrevFrame->Resize(g, w, h);
		//m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void AccumPass::Run(Graphics& g)
	{
		__super::Run(g);

		const auto& cam = *m_Parent->GetScene().m_Camera;
		if (cam.HashChanged())
			m_NumPassedFrames = 0;
		m_GPUHeap->Bind(g);

		auto inputTarget = GetInTarget("Target");
		auto outputTarget = GetOutTarget("Target");
		const auto barrier = inputTarget->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g.CL().ResourceBarrier(1, &barrier);
		g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());
		g.CL().SetGraphicsRoot32BitConstant(1, m_NumPassedFrames++, 0);

		Rasterize(g);

		Commander<D3D12_RESOURCE_BARRIER>::Init()
			.Add(m_PrevFrame->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
			.Add(outputTarget->Transition(D3D12_RESOURCE_STATE_COPY_SOURCE))
			.Transition(g);

		g.CL().CopyResource(**m_PrevFrame, **outputTarget);

		Commander<D3D12_RESOURCE_BARRIER>::Init()
			.Add(m_PrevFrame->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
			.Add(outputTarget->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET))
			.Add(inputTarget->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET))
			.Transition(g);

		g.Flush();
	}
}