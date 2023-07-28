#include "AccumPass.h"
#include "Bindable/Pipeline/AccumPipeline.h"
#include "Bindable/Pipeline/VertexBuffer.h"
#include "Bindable/Pipeline/IndexBuffer.h"
#include "Entity/Camera.h"
#include "FrameGraph.h"
#include "VecOps.h"

AccumPass::AccumPass(Graphics& g)
	:ScreenPass(g),  m_NumPassedFrames(0), m_PrevFrameHeap(g, 1)
{
	AddInTarget("AO");
	AddOutTarget("Accumulation");
}

void AccumPass::ShowGUI()
{
}

void AccumPass::OnResize(Graphics& g, UINT w, UINT h)
{
	ScreenPass::OnResize(g, w, h);
	m_PrevFrame->Resize(g, w, h);

	m_GPUHeap->Reset();
	auto& ins = GetInTargets();
	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());

	m_PrevFrame->CreateShaderResourceView(g, m_GPUHeap->Next());
	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void AccumPass::OnAdd(Graphics& g, FrameGraph* parent)
{
	Pass::OnAdd(g, parent);
	AddBindable(MakeShared<AccumPipeline>(g));

	auto& ins = GetInTargets();
	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, ins.size() + 1);
	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());

	m_PrevFrame = MakeUnique<RenderTarget>(g);
	m_PrevFrame->CreateView(g, m_PrevFrameHeap.Next());
	m_PrevFrame->CreateShaderResourceView(g, m_GPUHeap->Next());
	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	auto cam = parent->GetCamera();
	m_PrevCamHash = cam->Pos() * cam->Pitch() * cam->Yaw();
}

void AccumPass::Run(Graphics& g, FrameGraph* parent)
{
	auto cam = parent->GetCamera();
	XMFLOAT3 currentCamHash = cam->Pos() * cam->Pitch() * cam->Yaw();
	if (m_PrevCamHash != currentCamHash)
	{
		m_PrevCamHash = std::move(currentCamHash);
		m_NumPassedFrames = 0;
	}

	auto accum = GetOutTarget("Accumulation");
	accum->BindWithDepth(g, m_Depth);
	accum->Clear(g);
	m_Depth.Clear(g);

	BindBindables(g);
	m_GPUHeap->Bind(g);

	auto currentAO = GetInTarget("AO");
	currentAO->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());
	g.CL().SetGraphicsRoot32BitConstant(1, m_NumPassedFrames++, 0);

	Rasterize(g);

	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	accum->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	g.CL().CopyResource(**m_PrevFrame, **accum);
	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	accum->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	currentAO->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	g.Flush();
}