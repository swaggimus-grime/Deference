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

HGPU SamplerHeap::Add(Graphics& g)
{
	HGPU hgpu{ m_GPUStart.ptr + (m_HCPU.ptr - m_CPUStart.ptr) };
	
	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	g.Device().CreateSampler(&samplerDesc, m_HCPU);

	m_HCPU.ptr += m_IncSize;
	return hgpu;
}

GPUShaderHeap::GPUShaderHeap(Graphics& g, UINT numDescs)
	:GPUVisibleHeap(g, numDescs)
{
}

std::vector<std::vector<HGPU>> GPUShaderHeap::Copy(Graphics& g, HCPU src, UINT num, UINT stride)
{
	g.Device().CopyDescriptorsSimple(num, m_HCPU, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	HGPU hgpu{ m_GPUStart.ptr + (m_HCPU.ptr - m_CPUStart.ptr) };

	const UINT numEntries = num / stride;
	std::vector<std::vector<HGPU>> entries(numEntries, std::vector<HGPU>(stride));
	for (UINT i = 0; i < numEntries; i++)
		for (UINT j = 0; j < stride; j++)
		{
			entries[i][j] = hgpu;
			hgpu.ptr += m_IncSize;
		}

	m_HCPU.ptr += m_IncSize * num;
	return std::move(entries);
}
