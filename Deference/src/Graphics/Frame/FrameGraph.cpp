#include "FrameGraph.h"
#include "Entity/Model.h"
#include <imgui.h>

FrameGraph::FrameGraph(Scene& scene)
	:m_CurrentTarget(0), m_Camera(scene.m_Camera), m_Models(scene.m_Models)
{
}

Shared<RenderTarget> FrameGraph::Run(Graphics& g)
{
	for (auto& p : m_Passes)
		p.second->Run(g);

	return GetTarget(m_TargetNames[m_CurrentTarget]);
}

void FrameGraph::OnResize(Graphics& g, UINT w, UINT h)
{
	for (auto& p : m_Passes)
		p.second->OnResize(g, w, h);
}

void FrameGraph::ShowUI(Graphics& g)
{
	ImGui::Begin("Frame Graph");
	auto& current = m_TargetNames[m_CurrentTarget];
	if (ImGui::BeginCombo("Current Target", (current.Pass + '.' + current.Target).c_str())) {
		for (int i = 0; i < m_TargetNames.size(); ++i) {
			const bool isSelected = (m_CurrentTarget == i);
			auto& selected = m_TargetNames[i];
			if (ImGui::Selectable((selected.Pass + '.' + selected.Target).c_str(), isSelected)) {
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
		pass.second->ShowGUI();
}

void FrameGraph::AddGlobalResource(const std::string& name, Shared<Resource> r)
{
	m_GlobalResources.insert({ std::move(name), std::move(r) });
}

void FrameGraph::AddGlobalVectorResource(const std::string& name, std::tuple<HCPU, UINT, UINT> range)
{
	m_GlobalVectorResources.insert({ std::move(name), std::move(range) });
}

void FrameGraph::Finish(Graphics& g)
{
	m_GlobalHeap = MakeUnique<CPUShaderHeap>(g, m_GlobalResources.size() + 1000);

	for (auto& p : m_GlobalResources)
		m_GlobalHeap->Add(g, p.second);

	RecordPasses(g);
}

void FrameGraph::FinishRecordingPasses()
{
	for (auto& pass : m_Passes)
	{
		for (const auto& name : pass.second->GetOutTargets())
			m_TargetNames.emplace_back(pass.second->GetName(), name.first);
	}
}

Shared<RenderTarget> FrameGraph::GetTarget(const PassTargetName& name)
{
	auto it = std::find_if(m_Passes.begin(), m_Passes.end(),
		[&](const auto& p) {
			return p.first == name.Pass;
		});
	if (it != m_Passes.end())
		return it->second->GetOutTarget(name.Target);
	else
		throw DefException("Cannot find target " + name.Pass + "." + name.Target);
}
