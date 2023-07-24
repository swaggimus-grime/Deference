#include "DescriptorHeap.h"
#include <d3d12.h>
#include <d3dx12.h>

RenderTargetHeap::RenderTargetHeap(Graphics& g, UINT numTargets)
	:DescriptorHeap(g, numTargets), m_NumTargets(numTargets)
{
}

void RenderTargetHeap::Bind(Graphics& g)
{
	const auto& handle = CPUStart();
	g.CL().OMSetRenderTargets(m_NumTargets, &handle, true, nullptr);
}

void RenderTargetHeap::BindWithDepth(Graphics& g, Shared<DepthStencil> depth)
{
	const auto& handle = CPUStart();
	auto d = depth->GetView();
	g.CL().OMSetRenderTargets(m_NumTargets, &handle, true, &d);
}

DepthStencilHeap::DepthStencilHeap(Graphics& g, UINT numTargets)
	:DescriptorHeap(g, numTargets)
{
}

void DepthStencilHeap::Bind(Graphics& g)
{
	const auto& handle = CPUStart();
	g.CL().OMSetRenderTargets(0, nullptr, false, &handle);
}

CPUShaderHeap::CPUShaderHeap(Graphics& g, UINT numDescs)
	:DescriptorHeap(g, numDescs)
{
}

SamplerHeap::SamplerHeap(Graphics& g, UINT numDescs)
	:DescriptorHeap(g, numDescs)
{
}

GPUShaderHeap::GPUShaderHeap(Graphics& g, UINT numDescs)
	:DescriptorHeap(g, numDescs)
{
}
