#pragma once

#include "Graphics.h"
#include "Scene/Scene.h"
#include "Scene/Model.h"
#include "Resource/DescriptorHeap.h"
#include "Resource/RenderTarget.h"
#include "Debug/Exception.h"
#include "Pass.h"
#include <vector>
#include "BBox.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace Def
{
	class FrameGraph
	{
	public:
		Shared<RenderTarget> Run(Graphics& g)
		{
			for (auto& p : m_Passes)
				p.second->Run(g);

			return GetTarget(m_TargetNames[m_CurrentTarget]);
		}

		void ShowUI(Graphics& g)
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

		void OnResize(Graphics& g, UINT w, UINT h)
		{
			for (auto& p : m_Passes)
				p.second->OnResize(g, w, h);
		}

		void LoadScene(Graphics& g, const Shared<Scene>& scene)
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

		void Compile(Graphics& g)
		{
			for (auto& pass : m_Passes)
			{
				pass.second->Compile(g);
				for (const auto& name : pass.second->GetOutTargets())
					if (name.second->GetFormat() == Swapchain::s_Format)
						m_TargetNames.emplace_back(pass.second->GetName(), name.first);
			}
		}

		inline Scene& GetScene() const { return *m_Scene; }

		Shared<RenderTarget> GetTarget(const PassTargetName& name)
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

		template<class T>
		T& GetGlobals()
		{
			return *reinterpret_cast<T*>(m_GlobalPtr);
		}

	protected:
		FrameGraph() 
			: m_CurrentTarget(0) 
		{}

		template<class P>
		Shared<P> AddPass(Graphics& g, const std::string& name)
		{
			Shared<P> p = MakeShared<P>(g, name, this);
			m_Passes.push_back({ name, p });
			return std::move(p);
		}

		virtual void PrepLoadScene(Graphics& g) = 0;

		void SetGlobals(void* globalPtr)
		{
			m_GlobalPtr = globalPtr;
		}

	private:
		std::vector<std::pair<std::string, Shared<Pass>>> m_Passes;

		std::vector<PassTargetName> m_TargetNames;
		int m_CurrentTarget;

		Shared<Scene> m_Scene;
		void* m_GlobalPtr;
	};
}