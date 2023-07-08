#pragma once

#include "Bindable/Heap/Heap.h"

class Graphics;
class GeometryGraph;
class RenderTargetHeap;

class Pass
{
public:
	virtual void Run(Graphics& g, GeometryGraph* parent) = 0;

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

	virtual void OnAdd(Graphics& g, GeometryGraph* parent);

protected:
	inline void AddOutTarget(const std::string& name) { m_OutTargets.push_back({ std::move(name), nullptr }); }

protected:
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_InTargets;
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_OutTargets;
	Unique<RenderTargetHeap> m_RTs;
};