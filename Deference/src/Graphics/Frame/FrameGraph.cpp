//#include "FrameGraph.h"
//#include "FrameGraph.h"
//#include "Pass.h"
//#include "util.h"
//#include "Entity/Step.h"
//#include "DXR/nv_helpers_dx12/TopLevelASGenerator.h"
//
//FrameGraph::FrameGraph(Graphics& g, const SceneData& scene)
//	:m_RTHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1), m_DSHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1)
//{
//	m_DS = m_DSHeap.AddResource<DepthStencil>(g);
//	m_RT = m_RTHeap.AddResource<RenderTarget>(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
//}
//
//void FrameGraph::Run(Graphics& g)
//{
//	for (auto& b : m_Bindables)
//		b->Bind(g);
//
//	for (auto& pass : m_Passes)
//		pass->Run(g);
//}
//
//void FrameGraph::AddPass(Graphics& g, Shared<Pass> pass)
//{
//	pass->OnAdd(g);
//	m_Passes.emplace_back(std::move(pass));
//}
//
//void FrameGraph::AddBindable(Shared<Bindable> bindable)
//{
//	m_Bindables.push_back(std::move(bindable));
//}
//
//void FrameGraph::Connect(const std::string& outPassTarget, Shared<Pass> inPass, const std::string& inTarget)
//{
//	auto passTarget = ParseTokens(outPassTarget, '.');
//	if (passTarget[0] == "#")
//	{
//		if (passTarget[1] == "rt")
//			inPass->GetTarget(inTarget) = m_RT;
//		else if (passTarget[1] == "ds")
//			inPass->GetTarget(inTarget) = m_DS;
//		else
//			throw DefException(std::format("Failed to find target with name \"{}\" in pass \"{}\"", passTarget[1], passTarget[0]));
//	}
//	else
//		inPass->GetTarget(inTarget) = GetPass(passTarget[0])->GetTarget(passTarget[1]);
//}
//
//void FrameGraph::AddSteps(const std::vector<Step>& steps)
//{
//    for (auto& step : steps)
//        GetPass(step.PassName())->AddStep(std::move(step));
//}
//
//Shared<Pass> FrameGraph::GetPass(const std::string& name)
//{
//    return *(std::find_if(m_Passes.begin(), m_Passes.end(), [&](const auto& pass) {return pass->Name() == name; }));
//}
