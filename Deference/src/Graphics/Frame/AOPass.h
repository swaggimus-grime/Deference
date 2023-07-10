#pragma once

#include "BindablePass.h"

class AOPipeline;
class AOConstants;
class UnorderedAccess;
class m_Light;

class AOPass : public BindablePass
{
public:
	AOPass(Graphics& g);
	virtual void OnAdd(Graphics& g, GeometryGraph* parent) override;
	virtual void Run(Graphics& g, GeometryGraph* parent) override;

private:
	CSUHeap m_Heap;
	Shared<UnorderedAccess> m_Output;
	Shared<AOPipeline> m_Pipeline;
	ComPtr<ID3D12Resource> m_BottomLevelAS;
	Shared<AOConstants> m_AOConstants;
	UINT m_FrameCount;
};