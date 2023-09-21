#pragma once

#include "Pipeline.h"

class GeometryPipeline : public Pipeline
{
public:
	GeometryPipeline(Graphics& g, std::vector<DXGI_FORMAT> rtFormats);
};