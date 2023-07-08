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

	Shared<RenderTarget> Run(Graphics& g);

	inline auto& Drawables() const { return m_Drawables; }

private:
	void ConnectWithPreviousTarget(Shared<Pass> pass);
	
private:
	std::vector<Shared<Pass>> m_Passes;
	std::vector<Shared<Drawable>> m_Drawables;
};