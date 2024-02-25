#include "FrameGraph.h"
#include "Scene/Model.h"
#include <imgui.h>
#include <DirectXTex/DirectXTex.h>

namespace Def {
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

	void FrameGraph::LoadScene(Graphics& g, const Shared<Scene>& scene)
	{
		m_Scene = scene;
		PrepLoadScene(g);

		for (auto& p : m_Passes)
		{
			p.second->PrepLoadScene(g);
			p.second->BuildResources(g);
			p.second->OnSceneLoad(g);
		}
	}

	void FrameGraph::ShowUI(Graphics& g)
	{
		if (ImGui::Begin("Frame Graph"))
		{
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
		}
		ImGui::End();

		for (auto& pass : m_Passes)
			pass.second->ShowGUI();
	}

	void FrameGraph::Compile(Graphics& g)
	{
		for (auto& pass : m_Passes)
		{
			pass.second->Compile(g);
			for (const auto& name : pass.second->GetOutTargets())
				if (name.second->GetFormat() == Swapchain::s_Format)
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
}