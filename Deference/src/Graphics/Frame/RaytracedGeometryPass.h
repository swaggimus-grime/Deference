#pragma once

#include <array>
#include "RaytracePass.h"
#include "Bindable/Heap/Texture.h"

class Model;

class RaytracedGeometryPass : public RaytracePass
{
public:
	RaytracedGeometryPass(Graphics& g, FrameGraph* parent);
	virtual void Run(Graphics& g) override;
	virtual void OnAdd(Graphics& g) override;
	virtual void ShowGUI() override;

private:
	Shared<ConstantBuffer> m_Transform;
};