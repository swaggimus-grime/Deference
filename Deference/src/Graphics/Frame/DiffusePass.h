#pragma once

#include "BindablePass.h"

class DiffusePipeline;
class UnorderedAccess;
class m_Light;

class DiffusePass : public BindablePass
{
public:
	DiffusePass(Graphics& g);
	virtual void OnAdd(Graphics& g, GeometryGraph* parent) override;
	virtual void Run(Graphics& g, GeometryGraph* parent) override;

private:
	CSUHeap m_Heap;
	Shared<UnorderedAccess> m_Output;
	Shared<DiffusePipeline> m_Pipeline;
	ComPtr<ID3D12Resource> m_BottomLevelAS;
	Shared<PointLight> m_Light;
};