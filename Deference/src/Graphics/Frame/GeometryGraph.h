#pragma once

#include "GeometryPass.h"

class Drawable;
class DrawableCollection;

class GeometryGraph
{
public:
	GeometryGraph();
	void AddGeometry(Shared<Drawable> d);
	void AddGeometry(DrawableCollection& d);
	void AddPass(Graphics& g, Shared<Pass> pass);
	inline void SetCamera(Shared<Camera> cam) { m_Camera = cam; }
	inline auto GetCamera() const { return m_Camera; }

	Shared<RenderTarget> Run(Graphics& g);
	void ShowUI(Graphics& g);

	inline auto& Drawables() const { return m_Drawables; }

private:
	void ConnectTargets(Shared<Pass> pass);
	Shared<RenderTarget> GetTarget(const std::string& name);
private:
	std::vector<Shared<Pass>> m_Passes;
	std::vector<std::string> m_TargetNames;
	int m_CurrentTarget;
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_Targets;
	std::vector<Shared<Drawable>> m_Drawables;
	Shared<Camera> m_Camera;
};