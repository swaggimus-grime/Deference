#pragma once

#include "Pass.h"

class Model;

class FrameGraph
{
public:
	FrameGraph();
	void AddModel(Shared<Model> m);
	void FinishScene(Graphics& g);

	inline void SetCamera(Shared<Camera> cam) { m_Camera = cam; }

	Shared<RenderTarget> Run(Graphics& g);
	void ShowUI(Graphics& g);

	inline auto GetCamera() const { return m_Camera; }
	inline auto& GetModels() const { return m_Models; }
	inline auto GetTLAS() const { return m_TLAS; }

	void OnResize(Graphics& g, UINT w, UINT h);

	struct MeshArguments
	{
		HGPU m_VertexBuffer;
		HGPU m_IndexBuffer;
		HGPU m_DiffuseMap;
		HGPU m_NormalMap;
	};

	inline auto GetGlobalResource(const std::string& name)
	{
		auto r = m_GlobalResources.find(name);
		if (r != m_GlobalResources.end())
			return r->second;
		else
			throw DefException("Cannot find global resource with name: " + name);
	}
protected:
	template<typename T>
		requires Derived<Pass, T>
	void AddPass(Graphics& g)
	{
		m_Passes.push_back(std::move(MakeShared<T>(g)));
	}

	void AddGlobalResource(const std::string& name, Shared<Resource> r);

private:
	void ConnectTargets(Shared<Pass> pass);
	Shared<RenderTarget> GetTarget(const std::string& name);

private:
	std::vector<Shared<Pass>> m_Passes;

	std::vector<std::string> m_TargetNames;
	int m_CurrentTarget;
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_Targets;

	std::vector<Shared<Model>> m_Models;
	Shared<TLAS> m_TLAS;
	Shared<Camera> m_Camera;

	Unique<CPUShaderHeap> m_GlobalHeap;
	std::unordered_map<std::string, Shared<Resource>> m_GlobalResources;
};