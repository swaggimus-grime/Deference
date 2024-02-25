//#include "AccumPass.h"
//#include "Bindable/Pipeline/AccumPipeline.h"
//#include "Bindable/Pipeline/VertexBuffer.h"
//#include "Bindable/Pipeline/IndexBuffer.h"
//#include "Scene/Camera.h"
//#include "FrameGraph.h"
//#include "VecOps.h"
//
//AccumPass::AccumPass(Graphics& g, const std::string& name, FrameGraph* parent)
//	:ScreenPass(g, std::move(name), parent),  m_NumPassedFrames(0), m_PrevFrameHeap(g, 1)
//{
//	AddInTarget("Target");
//	AddOutTarget(g, "Target");
//
//	m_Pipeline = MakeUnique<AccumPipeline>(g);
//	m_PrevCamHash = { 0, 0, 0 };
//
//	m_PrevFrame = MakeShared<RenderTarget>(g);
//	m_PrevFrameHeap.Add(g, m_PrevFrame);
//	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//	AddTargetResource(m_PrevFrame);
//}
//
//void AccumPass::ShowGUI()
//{
//}
//
//void AccumPass::OnResize(Graphics& g, UINT w, UINT h)
//{
//	__super::OnResize(g, w, h);
//	m_PrevFrame->Resize(g, w, h);
//	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//}
//
//void AccumPass::Run(Graphics& g)
//{
//	__super::Run(g);
//
//	const auto& cam = m_Parent->GetScene().GetCamera();
//	XMFLOAT3 currentCamHash = cam.Pos() * cam.Pitch() * cam.Yaw();
//	if (m_PrevCamHash != currentCamHash)
//	{
//		m_PrevCamHash = std::move(currentCamHash);
//		m_NumPassedFrames = 0;
//	}
//
//	BindBindables(g);
//	m_GPUHeap->Bind(g);
//
//	auto currentAO = GetInTarget("Target");
//	currentAO->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());
//	g.CL().SetGraphicsRoot32BitConstant(1, m_NumPassedFrames++, 0);
//
//	Rasterize(g);
//
//	auto accum = GetOutTarget("Target");
//	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
//	accum->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
//	g.CL().CopyResource(**m_PrevFrame, **accum);
//	m_PrevFrame->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//	accum->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
//	currentAO->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
//
//	g.Flush();
//}