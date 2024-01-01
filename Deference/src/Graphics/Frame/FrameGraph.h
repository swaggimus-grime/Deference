#pragma once

#include "Scene/Scene.h"
#include "Bindable/Heap/DescriptorHeap.h"
#include "Bindable/Heap/RenderTarget.h"
#include "Debug/Exception.h"
#include "Pass.h"
#include <vector>
#include "BBox.h"

class Model;

class FrameGraph
{
public:
	FrameGraph() : m_CurrentTarget(0) {}
	Shared<RenderTarget> Run(Graphics& g);
	void ShowUI(Graphics& g);

	void OnResize(Graphics& g, UINT w, UINT h);
	void LoadScene(Graphics& g, const Shared<Scene>& scene);
	void Compile(Graphics& g);

	inline const Scene& GetScene() const { return *m_Scene; }

	Shared<RenderTarget> GetTarget(const PassTargetName& name);

	inline auto GetGlobalResource(const std::string& name)
	{
		auto r = m_GlobalResources.find(name);
		if (r != m_GlobalResources.end())
			return r->second;
		else
			throw DefException("Cannot find global resource with name: " + name);
	}

	inline auto GetGlobalVectorResource(const std::string& name)
	{
		auto r = m_GlobalVectorResources.find(name);
		if (r != m_GlobalVectorResources.end())
			return r->second;
		else
			throw DefException("Cannot find global vector resource with name: " + name);
	}

protected:
	template<typename T>
		requires Derived<Pass, T>
	auto AddPass(Graphics& g, const std::string& name)
	{
		auto p = MakeShared<T>(g, name, this);
		m_Passes.emplace_back(std::move(name), p);
		return std::move(p);
	}

	void AddGlobalResource(const std::string& name, Shared<Resource> r);
	void AddGlobalVectorResource(const std::string& name, std::tuple<HCPU, UINT, UINT> range);

	virtual void PrepLoadScene(Graphics& g) = 0;

private:
	std::vector<std::pair<std::string, Shared<Pass>>> m_Passes;

	std::vector<PassTargetName> m_TargetNames;
	int m_CurrentTarget;

	Shared<Scene> m_Scene;

	Unique<CPUShaderHeap> m_GlobalHeap;
	std::unordered_map<std::string, Shared<Resource>> m_GlobalResources;
	std::unordered_map<std::string, std::tuple<HCPU, UINT, UINT>> m_GlobalVectorResources;
};