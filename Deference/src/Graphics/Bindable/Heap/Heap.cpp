#include "Heap.h"
#include "Heap.h"
#include <d3d12.h>
#include <d3dx12.h>

RenderTargetHeap::RenderTargetHeap(Graphics& g, UINT numTargets)
	:Heap(g, numTargets), m_NumTargets(numTargets)
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
	g.CL().OMSetRenderTargets(m_NumTargets, &handle, true, &depth->Handle());
}

DepthStencilHeap::DepthStencilHeap(Graphics& g, UINT numTargets)
	:Heap(g, numTargets)
{
}

void DepthStencilHeap::Bind(Graphics& g)
{
	const auto& handle = CPUStart();
	g.CL().OMSetRenderTargets(0, nullptr, false, &handle);
}

CSUHeap::CSUHeap(Graphics& g, UINT numDescs)
	:Heap(g, numDescs, true)
{
	m_GPUHandle = m_Heap->GetGPUDescriptorHandleForHeapStart();
}

void CSUHeap::CopyToHeap(Graphics& g, const SucHeap& other)
{
	if (&other == nullptr)
		return;
	g.Device().CopyDescriptorsSimple(other.NumActiveDescs(), m_Handle, other.CPUStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_Handle.ptr += m_IncSize * other.NumActiveDescs();
	m_GPUHandle.ptr += m_IncSize * other.NumActiveDescs();
}

SucHeap::SucHeap(Graphics& g, UINT numDescs)
	:Heap(g, numDescs, false), m_CurrentDescIdx(0)
{
}

void SucHeap::Free(Shared<Resource> r)
{
	m_FreeHandles.push(r->Handle());
	auto it = std::find_if(m_ActiveHandles.begin(), m_ActiveHandles.end(), [&](const auto& h) { return r->Handle().ptr == h.ptr; });
	m_ActiveHandles.erase(it);
}

SamplerHeap::SamplerHeap(Graphics& g, UINT numDescs)
	:Heap(g, numDescs, true)
{
}