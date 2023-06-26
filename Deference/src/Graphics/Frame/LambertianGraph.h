#pragma once

#include "FrameGraph.h"

class LambertianGraph : public FrameGraph
{
public:
	LambertianGraph(Graphics& g, const SceneData& scene);

protected:
	Shared<Camera> m_Cam;
};