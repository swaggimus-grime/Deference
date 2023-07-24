#pragma once

#include "Pass.h"

class DiffusePipeline;
class UnorderedAccess;

class DiffusePass : public Pass
{
public:
	DiffusePass(Graphics& g);
	virtual void OnAdd(Graphics& g, FrameGraph* parent) override;
	virtual void Run(Graphics& g, FrameGraph* parent) override;

	virtual void ShowGUI() override;

private:
	Shared<DiffusePipeline> m_Pipeline;

	Unique<GPUShaderHeap> m_GPUHeap;
	Unique<UnorderedAccess> m_Output;
	Unique<ConstantBuffer> m_Light;
};