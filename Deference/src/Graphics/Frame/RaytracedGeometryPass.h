//#pragma once
//
//#include <array>
//#include "Pass.h"
//#include "Bindable/Heap/Texture.h"
//
//class RaytracedGeometryPipeline;
//
//class RaytracedGeometryPass : public Pass
//{
//public:
//	RaytracedGeometryPass(Graphics& g);
//	virtual void OnAdd(Graphics& g, FrameGraph* parent) override;
//	virtual void Run(Graphics& g, FrameGraph* parent) override;
//
//	virtual void ShowGUI() override;
//
//private:
//	Shared<RaytracedGeometryPipeline> m_Pipeline;
//	EnvironmentMap m_Environment;
//	GPUShaderHeap m_GPUHeap;
//	std::vector<Shared<UnorderedAccess>> m_Outputs;
//	Unique<ConstantBuffer> m_Transform;
//};