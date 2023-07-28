#pragma once

#include "GeometryPass.h"

class Model;

class FrameGraph
{
public:
	FrameGraph();
	void AddModel(Shared<Model> m);
	void FinishScene(Graphics& g);

	void AddPass(Graphics& g, Shared<Pass> pass);
	inline void SetCamera(Shared<Camera> cam) { m_Camera = cam; }
	inline auto GetCamera() const { return m_Camera; }

	Shared<RenderTarget> Run(Graphics& g);
	void ShowUI(Graphics& g);

	inline auto& GetModels() const { return m_Models; }
	inline TLAS& GetTLAS() const { return *m_TLAS; }

	void OnResize(Graphics& g, UINT w, UINT h);

private:
	void ConnectTargets(Shared<Pass> pass);
	Shared<RenderTarget> GetTarget(const std::string& name);

private:
	std::vector<Shared<Pass>> m_Passes;

	std::vector<std::string> m_TargetNames;
	int m_CurrentTarget;
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_Targets;

	std::vector<Shared<Model>> m_Models;
	Unique<TLAS> m_TLAS;
	Shared<Camera> m_Camera;
};