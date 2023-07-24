#pragma once

#include "Pass.h"

class AOPipeline;
class UnorderedAccess;

class AOPass : public Pass
{
public:
	AOPass(Graphics& g);
	virtual void OnAdd(Graphics& g, FrameGraph* parent) override;
	virtual void Run(Graphics& g, FrameGraph* parent) override;
	virtual void ShowGUI() override;

private:
	Unique<GPUShaderHeap> m_GPUHeap;
	Unique<UnorderedAccess> m_Output;
	Unique<ConstantBuffer> m_Constants;
	Shared<AOPipeline> m_Pipeline;
	UINT m_FrameCount;
};