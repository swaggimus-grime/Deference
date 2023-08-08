#include "FrameGraph.h"
#include <imgui.h>

FrameGraph::FrameGraph()
	:m_CurrentTarget(0)
{
}

void FrameGraph::AddModel(Shared<Model> m)
{
	m_Models.push_back(std::move(m));
}

void FrameGraph::FinishScene(Graphics& g)
{
	UINT totalModelDescriptors = 0;

	const auto& blass = ToVector(m_Models, [&](UINT i) {
		const auto& m = m_Models[i];
		totalModelDescriptors += m->GetMeshes().size() * sizeof(MeshArguments) / sizeof(HGPU);
		return m->GetBLAS();
	});

	m_TLAS = MakeShared<TLAS>(g, std::move(blass));

	m_GlobalHeap = MakeUnique<CPUShaderHeap>(g, m_HCPUs.size() + totalModelDescriptors);
	AddGlobalResource("TLAS", m_TLAS);

	for (auto& p : m_GlobalResources)
		p.second->CreateView(g, m_GlobalHeap->Next().m_HCPU);
	
	for (auto& model : m_Models)
	{
		for (auto& mesh : model->GetMeshes())
		{
			mesh.m_VB->CreateView(g, m_GlobalHeap->Next().m_HCPU);
			mesh.m_IB->CreateView(g, m_GlobalHeap->Next().m_HCPU);
			mesh.m_DiffuseMap->CreateView(g, m_GlobalHeap->Next().m_HCPU);
			mesh.m_NormalMap->CreateView(g, m_GlobalHeap->Next().m_HCPU);
		}
	}

	for (auto& pass : m_Passes)
	{
		ConnectTargets(pass);
		pass->OnAdd(g);
		for (auto& out : pass->GetOutTargets())
		{
			m_TargetNames.push_back(out.first);
			m_Targets.push_back(out);
		}
	}
}

Shared<RenderTarget> FrameGraph::Run(Graphics& g)
{
	for (auto& p : m_Passes)
		p->Run(g);

	return GetTarget(m_TargetNames[m_CurrentTarget]);
}

void FrameGraph::OnResize(Graphics& g, UINT w, UINT h)
{
	for (auto& p : m_Passes)
		p->OnResize(g, w, h);
}

void FrameGraph::ShowUI(Graphics& g)
{
	ImGui::Begin("Frame Graph");
	if (ImGui::BeginCombo("Current Target", m_TargetNames[m_CurrentTarget].c_str())) {
		for (int i = 0; i < m_TargetNames.size(); ++i) {
			const bool isSelected = (m_CurrentTarget == i);
			if (ImGui::Selectable(m_TargetNames[i].c_str(), isSelected)) {
				m_CurrentTarget = i;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::End();

	for (auto& pass : m_Passes)
		pass->ShowGUI();
}

void FrameGraph::AddGlobalResource(const std::string& name, Shared<Resource> r)
{
	m_GlobalResources.insert({ std::move(name), std::move(r) });
}

void FrameGraph::ConnectTargets(Shared<Pass> pass)
{
	if (m_Passes.empty())
		return;

	for (auto& in : pass->GetInTargets())
		in.second = GetTarget(in.first);
}

Shared<RenderTarget> FrameGraph::GetTarget(const std::string& name)
{
	auto it = std::find_if(m_Targets.begin(), m_Targets.end(), [&](const auto& p) { return name == p.first; });
	return it->second;
}
