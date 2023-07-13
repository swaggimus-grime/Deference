#include "AccumPass.h"
#include "Bindable/Pipeline/AccumPipeline.h"
#include "Bindable/Pipeline/VertexBuffer.h"
#include "Bindable/Pipeline/IndexBuffer.h"
#include "Entity/Camera.h"
#include "GeometryGraph.h"
#include "VecOps.h"

AccumPass::AccumPass(Graphics& g)
	:ScreenPass(g), m_DepthHeap(g), m_NumPassedFrames(0), m_PrevFrameHeap(g, 1)
{
	m_Depth = m_DepthHeap.Add(g);
	m_ResHeap = MakeUnique<CSUHeap>(g, 2);

	AddInTarget("AO");
	AddOutTarget("Accumulation");
}


void AccumPass::OnAdd(Graphics& g, GeometryGraph* parent)
{
	Pass::OnAdd(g, parent);
	AddBindable(MakeShared<Viewport>(g));
	AddBindable(MakeShared<AccumPipeline>(g));

	auto& ins = GetInTargets();
	for (auto& in : ins)
		m_ResHeap->Add<RTV>(g, in.second);
	m_PrevFrame = m_PrevFrameHeap.Add(g);
	m_PrevFrame->Res()->SetName(L"PrevFrame");
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_PrevFrame->Res(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g.CL().ResourceBarrier(1, &barrier);
	}
	g.Flush();

	m_ResHeap->Add<RTV>(g, m_PrevFrame);
	auto cam = parent->GetCamera();
	m_PrevCamHash = cam->Pos() * cam->Pitch() * cam->Yaw();
}

void AccumPass::Run(Graphics& g, GeometryGraph* parent)
{
	auto cam = parent->GetCamera();
	XMFLOAT3 currentCamHash = cam->Pos() * cam->Pitch() * cam->Yaw();
	if (m_PrevCamHash != currentCamHash)
	{
		m_PrevCamHash = std::move(currentCamHash);
		m_NumPassedFrames = 0;
	}

	auto accum = GetOutTarget("Accumulation");
	accum->BindWithOther(g, m_Depth.get());
	accum->Clear(g);
	m_Depth->Clear(g);

	BindBindables(g);
	m_ResHeap->Bind(g);

	auto currentAO = GetInTarget("AO");
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentAO->Res(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g.CL().ResourceBarrier(1, &barrier);
	}
	g.CL().SetGraphicsRootDescriptorTable(0, m_ResHeap->GPUStart());
	g.CL().SetGraphicsRoot32BitConstant(1, m_NumPassedFrames++, 0);

	Rasterize(g);

	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_PrevFrame->Res(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		g.CL().ResourceBarrier(1, &barrier);
	}
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(accum->Res(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
		g.CL().ResourceBarrier(1, &barrier);
	}
	g.CL().CopyResource(m_PrevFrame->Res(), accum->Res());
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_PrevFrame->Res(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g.CL().ResourceBarrier(1, &barrier);
	}
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(accum->Res(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		g.CL().ResourceBarrier(1, &barrier);
	}
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentAO->Res(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		g.CL().ResourceBarrier(1, &barrier);
	}

	g.Flush();
}