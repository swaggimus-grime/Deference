#pragma once

#include <array>
#include "RaytracePass.h"
#include "Bindable/Heap/Texture.h"
#include <random>

class Model;

class RaytracedGeometryPass : public RaytracePass
{
public:
	RaytracedGeometryPass(Graphics& g, const std::string& name, FrameGraph* parent);
	virtual void Run(Graphics& g) override;
	virtual void ShowGUI() override;
	virtual void OnSceneLoad(Graphics& g) override;

private:
	Shared<ConstantBuffer> m_Transform;
	std::uniform_real_distribution<float> m_RngDist;
	std::mt19937 m_Rng;
	UINT m_FrameCount = 0;
	float m_FStop = 32.0f;
	float m_FocalLength = 32.0f;
	float m_LensRadius;
};