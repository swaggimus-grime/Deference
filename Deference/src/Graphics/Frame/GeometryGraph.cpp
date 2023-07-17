#include "GeometryGraph.h"
#include "Entity/Drawable.h"
#include <imgui.h>

GeometryGraph::GeometryGraph()
	:m_CurrentTarget(0)
{
}

void GeometryGraph::AddGeometry(Shared<Drawable> d)
{
	m_Drawables.push_back(std::move(d));
}

void GeometryGraph::AddGeometry(DrawableCollection& d)
{
	for (auto& drawable : d.Drawables())
		AddGeometry(std::move(drawable));
}

void GeometryGraph::AddPass(Graphics& g, Shared<Pass> pass)
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

Shared<RenderTarget> GeometryGraph::Run(Graphics& g)
{
	for (auto& p : m_Passes)
		p->Run(g, this);

	return GetTarget(m_TargetNames[m_CurrentTarget]);
}

void GeometryGraph::ShowUI(Graphics& g)
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
}

void GeometryGraph::ConnectTargets(Shared<Pass> pass)
{
	if (m_Passes.empty())
		return;

	for (auto& in : pass->GetInTargets())
		in.second = GetTarget(in.first);
}

Shared<RenderTarget> GeometryGraph::GetTarget(const std::string& name)
{
	auto it = std::find_if(m_Targets.begin(), m_Targets.end(), [&](const auto& p) { return name == p.first; });
	return it->second;
}
