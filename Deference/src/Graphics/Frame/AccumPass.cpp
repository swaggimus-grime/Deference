#include "AccumPass.h"
#include "Bindable/Pipeline/AccumPipeline.h"
#include "Bindable/Pipeline/VertexBuffer.h"
#include "Bindable/Pipeline/IndexBuffer.h"
#include "Entity/Camera.h"
#include "FrameGraph.h"
#include "VecOps.h"

AccumPass::AccumPass(Graphics& g, FrameGraph* parent)
	:ScreenPass(g),  m_NumPassedFrames(0), m_PrevFrameHeap(g, 1), m_Cam(parent->GetCamera())
{
	AddInTarget("AO");
	AddOutTarget("Accumulation");

	m_Pipeline = MakeUnique<AccumPipeline>(g);
	m_PrevCamHash = m_Cam->Pos() * m_Cam->Pitch() * m_Cam->Yaw();
}

void AccumPass::ShowGUI()
{
}

void AccumPass::OnResize(Graphics& g, UINT w, UINT h)
{
	ScreenPass::OnResize(g, w, h);
	m_PrevFrame->Resize(g, w, h);
	m_PrevFrame->CreateShaderResourceView(g, m_GPUHeap->Next());
	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void AccumPass::OnAdd(Graphics& g)
{
	__super::OnAdd(g);
	
	auto& ins = GetInTargets();
	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, ins.size() + 1);
	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());

	m_PrevFrame = MakeUnique<RenderTarget>(g);
	m_PrevFrame->CreateView(g, m_PrevFrameHeap.Next());
	m_PrevFrame->CreateShaderResourceView(g, m_GPUHeap->Next());
	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void AccumPass::Run(Graphics& g)
{
	__super::Run(g);

	XMFLOAT3 currentCamHash = m_Cam->Pos() * m_Cam->Pitch() * m_Cam->Yaw();
	if (m_PrevCamHash != currentCamHash)
	{
		m_PrevCamHash = std::move(currentCamHash);
		m_NumPassedFrames = 0;
	}

	BindBindables(g);
	m_GPUHeap->Bind(g);

	auto currentAO = GetInTarget("AO");
	currentAO->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());
	g.CL().SetGraphicsRoot32BitConstant(1, m_NumPassedFrames++, 0);

	Rasterize(g);

	auto accum = GetOutTarget("Accumulation");
	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	accum->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	g.CL().CopyResource(**m_PrevFrame, **accum);
	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	accum->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	currentAO->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	g.Flush();
}