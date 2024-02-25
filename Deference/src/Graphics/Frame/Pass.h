#pragma once

#include "Resource/DescriptorHeap.h"
#include "Resource/RenderTarget.h"
#include "Swapchain.h"

namespace Def
{
	class FrameGraph;

	struct PassTargetName : private std::pair<std::string, std::string>
	{
		std::string Pass;
		std::string Target;

		PassTargetName(std::string pass, std::string target)
			:std::pair<std::string, std::string>(pass, target),
			Pass(std::move(this->first)), Target(std::move(this->second))
		{
		}
	};

	class Pass
	{
	public:
		Pass(const std::string& name, FrameGraph* parent)
			:m_Name(std::move(name)), m_Parent(parent)
		{
		}

		inline auto GetName() const { return m_Name; }
		inline const auto& GetInTargets() const { return m_InTargets; }
		inline const auto& GetOutTargets() const { return m_OutTargets; }
		inline const auto& GetInTarget(const std::string& target) const {
			auto it = std::find_if(m_InTargets.begin(), m_InTargets.end(),
				[&](const auto& p) {
					return p.first == target;
				});
			if (it != m_InTargets.end())
				return it->second;
			else
				throw DefException("Cannot find in target with name " + target);
		}
		inline const auto& GetOutTarget(const std::string& target) const {
			auto it = std::find_if(m_OutTargets.begin(), m_OutTargets.end(),
				[&](const auto& p) {
					return p.first == target;
				});
			if (it != m_OutTargets.end())
				return it->second;
			else
				throw DefException("Cannot find out target with name " + target);
		}

		void Link(const std::string& otherPass, const std::string& otherTarget, std::optional<std::string> targetName = {});
		
		void Compile(Graphics& g);

		virtual void PrepLoadScene(Graphics& g) {}
		void BuildResources(Graphics& g);
		virtual void OnSceneLoad(Graphics& g) {}

		virtual void Run(Graphics& g) = 0;
		virtual void ShowGUI() {}
		virtual void OnResize(Graphics& g, UINT w, UINT h);

	protected:
		void AddResource(Shared<CSUView> r, bool null = false)
		{
			m_Resources.push_back({ std::move(r), null });
		}

		inline virtual void AddOutTarget(Graphics& g, const std::string& target, DXGI_FORMAT fmt = Swapchain::s_Format) {
			m_OutTargets.emplace_back(std::move(target), MakeShared<RenderTarget>(g, fmt));
		}

		inline void AddInTarget(std::string&& target) {
			m_InTargets.emplace_back(std::move(target), nullptr);
		}

		inline void AddResizeable(Shared<Resizeable> r)
		{
			m_Resizeables.push_back(std::move(r));
		}

	protected:
		Unique<GPUHeap> m_GPUHeap;
		Unique<RenderTargetHeap> m_TargetHeap;
		FrameGraph* m_Parent;

	private:
		std::vector<std::pair<Shared<CSUView>, bool>> m_Resources;

		std::string m_Name;
		std::vector<std::pair<std::string, Shared<RenderTarget>>> m_InTargets;
		std::vector<std::pair<std::string, Shared<RenderTarget>>> m_OutTargets;

		std::vector<Shared<Resizeable>> m_Resizeables;
	};
}