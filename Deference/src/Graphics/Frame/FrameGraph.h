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
		Shared<RenderTarget> Run(Graphics& g);
		void ShowUI(Graphics& g);
		void OnResize(Graphics& g, UINT w, UINT h);
		void LoadScene(Graphics& g, const Shared<Scene>& scene);
		void Compile(Graphics& g);
		inline Scene& GetScene() const { return *m_Scene; }
		Shared<RenderTarget> GetTarget(const PassTargetName& name);

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