#pragma once

#include "FrameGraph.h"
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

	inline auto& Drawables() const { return m_Drawables; }

private:
	void ConnectWithPreviousTarget(Shared<Pass> pass);
	
private:
	std::vector<Shared<Pass>> m_Passes;
	std::vector<Shared<Drawable>> m_Drawables;
	Shared<Camera> m_Camera;
};