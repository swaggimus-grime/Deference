#pragma once

#include "RasterPass.h"

class GeometryPass : public RasterPass
{
public:
	GeometryPass(Graphics& g, const std::string& name, FrameGraph* parent);
	virtual void Run(Graphics& g) override;

private:
	SamplerHeap m_SamplerHeap;
	Shared<ConstantBuffer> m_Camera;
	Shared<ConstantBuffer> m_Transform;
};