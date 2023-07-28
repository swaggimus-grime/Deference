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
	const auto& blass =
		std::views::iota(0u, (UINT)m_Models.size()) |
		std::views::transform([&](UINT i) {
		return m_Models[i]->GetBLAS();
			}) |
		std::ranges::to<std::vector>();

	m_TLAS = MakeUnique<TLAS>(g, std::move(blass));
}

void FrameGraph::AddPass(Graphics& g, Shared<Pass> pass)
{
	ConnectTargets(pass);
	pass->OnAdd(g, this);
	for (auto& out : pass->GetOutTargets())
	{
		m_TargetNames.push_back(out.first);
		m_Targets.push_back(out);
	}
	m_Passes.push_back(std::move(pass));
}

Shared<RenderTarget> FrameGraph::Run(Graphics& g)
{
	for (auto& p : m_Passes)
		p->Run(g, this);

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
