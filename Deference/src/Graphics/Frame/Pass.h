#pragma once

#include "Bindable/Heap/DescriptorHeap.h"

class Graphics;
class FrameGraph;
class RenderTargetHeap;

class Pass
{
public:
	virtual void Run(Graphics& g, FrameGraph* parent) = 0;

	virtual void ShowGUI() {}

	inline auto& GetInTargets() { return m_InTargets; }
	inline auto& GetOutTargets() { return m_OutTargets; }
	inline Shared<RenderTarget> GetOutTarget(const std::string& name) const 
	{ 
		auto it = std::find_if(m_OutTargets.begin(), m_OutTargets.end(), 
			[&](const auto& pair)
			{
				return pair.first == name;
			}
		);
		return it->second;
	}

	inline Shared<RenderTarget> GetInTarget(const std::string& name) const
	{
		auto it = std::find_if(m_InTargets.begin(), m_InTargets.end(),
			[&](const auto& pair)
			{
				return pair.first == name;
			}
		);
		return it->second;
	}

	virtual void OnAdd(Graphics& g, FrameGraph* parent);

protected:
	inline void AddInTarget(const std::string& name) { m_InTargets.push_back({ std::move(name), nullptr }); }
	inline void AddOutTarget(const std::string& name) { m_OutTargets.push_back({ std::move(name), nullptr }); }
	inline void AddBindable(Shared<Bindable> b) { m_Bindables.push_back(std::move(b)); }
	void BindBindables(Graphics& g);

protected:
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_InTargets;
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_OutTargets;

	Unique<RenderTargetHeap> m_RTs;
	std::vector<Shared<Bindable>> m_Bindables;
};