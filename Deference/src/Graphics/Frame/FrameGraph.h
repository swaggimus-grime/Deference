#pragma once

#include "Bindable/Heap/DescriptorHeap.h"
#include "Bindable/Heap/RenderTarget.h"
#include "Debug/Exception.h"
#include "Pass.h"
#include <vector>
#include "BBox.h"

class Model;

struct Scene
{
	std::vector<Shared<Model>> m_Models;
	Shared<const Camera> m_Camera;
	BBox m_BBox;
};

class FrameGraph
{
public:
	Shared<RenderTarget> Run(Graphics& g);
	void ShowUI(Graphics& g);

	inline auto GetCamera() const { return m_Camera; }
	inline auto& GetModels() const { return m_Models; }

	void OnResize(Graphics& g, UINT w, UINT h);

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
	FrameGraph(Scene& scene);

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

	void Finish(Graphics& g);

	virtual void RecordPasses(Graphics& g) = 0;
	void FinishRecordingPasses();

private:
	std::vector<std::pair<std::string, Shared<Pass>>> m_Passes;

	std::vector<PassTargetName> m_TargetNames;
	int m_CurrentTarget;

	std::vector<Shared<Model>> m_Models;
	Shared<const Camera> m_Camera;

	Unique<CPUShaderHeap> m_GlobalHeap;
	std::unordered_map<std::string, Shared<Resource>> m_GlobalResources;
	std::unordered_map<std::string, std::tuple<HCPU, UINT, UINT>> m_GlobalVectorResources;
};