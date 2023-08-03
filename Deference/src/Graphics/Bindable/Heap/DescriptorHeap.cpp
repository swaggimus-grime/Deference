#include "DescriptorHeap.h"
#include <d3d12.h>
#include <d3dx12.h>

RenderTargetHeap::RenderTargetHeap(Graphics& g, UINT numTargets)
	:DescriptorHeap(g, numTargets, false), m_NumTargets(numTargets)
{
}

void RenderTargetHeap::Bind(Graphics& g)
{
	const auto& handle = CPUStart();
	g.CL().OMSetRenderTargets(m_NumTargets, &handle, true, nullptr);
}

void RenderTargetHeap::BindWithDepth(Graphics& g, DepthStencil& depth)
{
	const auto& handle = CPUStart();
	auto d = depth.GetHCPU();
	g.CL().OMSetRenderTargets(m_NumTargets, &handle, true, &d);
}

DepthStencilHeap::DepthStencilHeap(Graphics& g, UINT numTargets)
	:DescriptorHeap(g, numTargets, false)
{
}

void DepthStencilHeap::Bind(Graphics& g)
{
	const auto& handle = CPUStart();
	g.CL().OMSetRenderTargets(0, nullptr, false, &handle);
}

CPUShaderHeap::CPUShaderHeap(Graphics& g, UINT numDescs)
	:DescriptorHeap(g, numDescs, false)
{
}

SamplerHeap::SamplerHeap(Graphics& g, UINT numDescs)
	:GPUVisibleHeap(g, numDescs)
{
}

GPUShaderHeap::GPUShaderHeap(Graphics& g, UINT numDescs)
	:GPUVisibleHeap(g, numDescs)
{
}

