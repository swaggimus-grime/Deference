#include "HybridOutPass.h"
#include "Bindable/Pipeline/HybridOutPipeline.h"

HybridOutPass::HybridOutPass(Graphics& g)
	:ScreenPass(g), m_DepthHeap(g)
{
	m_Depth = m_DepthHeap.Add(g);

	m_ResHeap = MakeUnique<CSUHeap>(g, 3);

	AddInTarget("Position");
	AddInTarget("Diffuse");
	AddInTarget("Accumulation");

	AddOutTarget("Hybrid");

	AddBindable(MakeShared<Viewport>(g));
	AddBindable(MakeShared<HybridOutPipeline>(g));
}

void HybridOutPass::OnAdd(Graphics& g, GeometryGraph* parent)
{
	Pass::OnAdd(g, parent);

	auto& ins = GetInTargets();
	for (auto& in : ins)
		m_ResHeap->Add<RTV>(g, in.second);
}

void HybridOutPass::Run(Graphics& g, GeometryGraph* parent)
{
	auto target = GetOutTarget("Hybrid");
	target->BindWithOther(g, m_Depth.get());
	target->Clear(g);
	m_Depth->Clear(g);

	auto& inTargets = GetInTargets();
	{
		D3D12_RESOURCE_BARRIER barriers[3];
		for (UINT i = 0; i < 3; i++)
		{
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(inTargets[i].second->Res(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		g.CL().ResourceBarrier(3, barriers);
	}
	

	BindBindables(g);
	m_ResHeap->Bind(g);
	g.CL().SetGraphicsRootDescriptorTable(0, m_ResHeap->GPUStart());

	Rasterize(g);

	{
		D3D12_RESOURCE_BARRIER barriers[3];
		for (UINT i = 0; i < 3; i++)
		{
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(inTargets[i].second->Res(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		g.CL().ResourceBarrier(3, barriers);
	}

	g.Flush();
}
