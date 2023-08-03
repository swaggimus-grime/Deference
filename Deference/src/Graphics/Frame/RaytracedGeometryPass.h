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
	struct HitArguments
	{
		HGPU m_VertexBuffer;
		HGPU m_IndexBuffer;
		HGPU m_DiffuseMap;
		HGPU m_NormalMap;
	};

private:
	std::vector<Shared<Model>> m_Models;
	Shared<EnvironmentMap> m_Environment;
	Shared<ConstantBuffer> m_Transform;
	Shared<Camera> m_Cam;
};