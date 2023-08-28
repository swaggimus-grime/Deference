#pragma once

#include <array>
#include "RaytracePass.h"
#include "Bindable/Heap/Texture.h"

class Model;

class RaytracedGeometryPass : public RaytracePass
{
public:
	RaytracedGeometryPass(Graphics& g, const std::string& name, FrameGraph* parent);
	virtual void Run(Graphics& g) override;
	virtual void ShowGUI() override;
	virtual void Finish(Graphics& g) override;

private:

private:
	Shared<ConstantBuffer> m_Transform;
};