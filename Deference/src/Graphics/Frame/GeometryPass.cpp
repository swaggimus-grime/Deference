#include "GeometryPass.h"
#include "Entity/Drawable.h"
#include "Bindable/Heap/RenderTarget.h"
#include "Bindable/Pipeline/GeometryPipeline.h"
#include "Bindable/Viewport.h"
#include "GeometryGraph.h"
#include "Bindable/Heap/Sampler.h"

GeometryPass::GeometryPass(Graphics& g)
	:m_DepthHeap(g)
{
	m_Depth = m_DepthHeap.Add(g);

	m_SamplerHeap = MakeUnique<SamplerHeap>(g, 1);
	m_SamplerHeap->Add<Sampler>(g);
	m_TextureHeap = MakeUnique<CSUHeap>(g, 100);

	AddOutTarget("Position");
	AddOutTarget("Normal");
	AddOutTarget("Albedo");
}

void GeometryPass::OnAdd(Graphics& g, GeometryGraph* parent)
{
	BindablePass::OnAdd(g, parent);
	AddBindable(MakeShared<Viewport>(g));
	AddBindable(MakeShared<GeometryPipeline>(g));

	auto& drawables = parent->Drawables();
	for (auto& d : drawables)
		m_TextureHeap->CopyToHeap(g, d->GetTextureHeap());
	m_CBVHandle = m_TextureHeap->GPUHandle();
	for (auto& d : drawables)
		m_TextureHeap->CopyToHeap(g, d->GetCBVHeap());
}

void GeometryPass::Run(Graphics& g, GeometryGraph* parent)
{
	auto& outs = GetOutTargets();
	m_RTs->BindWithDepth(g, m_Depth);
	for (auto& out : outs)
	{
		out.second->Clear(g);
	}
	m_Depth->Clear(g);

	Bind(g);

	ID3D12DescriptorHeap* heaps[] = { m_TextureHeap->GetHeap(), m_SamplerHeap->GetHeap() };
	g.CL().SetDescriptorHeaps(2, heaps);
	g.CL().SetGraphicsRootDescriptorTable(0, m_TextureHeap->GPUStart());
	g.CL().SetGraphicsRootDescriptorTable(1, m_SamplerHeap->GPUStart());

	g.CL().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g.SetCamera(parent->GetCamera());

	auto& drawables = parent->Drawables();
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle = m_CBVHandle;
	for (auto& d : drawables)
	{
		g.CL().SetGraphicsRoot32BitConstant(3, d->DiffuseIndex(), 0);
		g.CL().SetGraphicsRootDescriptorTable(2, cbvHandle);
		cbvHandle.ptr += m_TextureHeap->IncSize();

		d->Rasterize(g);
	}
}