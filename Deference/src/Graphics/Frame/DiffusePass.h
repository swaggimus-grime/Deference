#pragma once

#include "BindablePass.h"

class DiffusePipeline;
class UnorderedAccess;

class DiffusePass : public BindablePass
{
public:
	DiffusePass(Graphics& g);
	virtual void OnAdd(Graphics& g, GeometryGraph* parent) override;
	virtual void Run(Graphics& g, GeometryGraph* parent) override;

private:
	SucHeap m_SucHeap;
	Shared<UnorderedAccess> m_Output;
	Shared<DiffusePipeline> m_Pipeline;
	ComPtr<ID3D12Resource> m_BottomLevelAS;
};