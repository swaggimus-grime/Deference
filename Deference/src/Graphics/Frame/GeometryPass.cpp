#include "GeometryPass.h"
#include "Bindable/Heap/RenderTarget.h"
#include "Bindable/Pipeline/GeometryPipeline.h"
#include "Bindable/Viewport.h"
#include "GeometryGraph.h"
#include "Bindable/Heap/Sampler.h"

GeometryPass::GeometryPass(Graphics& g)
	:m_DepthHeap(g, 1), m_SamplerHeap(g, 1), m_GPUHeap(g, 10)
{
	m_Depth = MakeShared<DepthStencil>(g);
	m_Depth->CreateView(g, m_DepthHeap.Next());

	m_SamplerHeap.Add(g);

	AddOutTarget("Position");
	AddOutTarget("Normal");
	AddOutTarget("Albedo");
}

void GeometryPass::OnAdd(Graphics& g, FrameGraph* parent)
{
	Pass::OnAdd(g, parent);
	AddBindable(MakeShared<Viewport>(g));
	AddBindable(MakeShared<GeometryPipeline>(g));

	const auto& models = parent->GetModels();
	for (const auto& m : models)
	{
		for (const auto& texture : m->GetTextures())
		{
			texture->CreateView(g, m_GPUHeap.Next());
		}
	}

	using enum CONSTANT_TYPE;

	{
		ConstantBufferLayout layout;
		layout.Add<XMMATRIX>("world");
		layout.Add<XMMATRIX>("mvp");
		layout.Add<XMFLOAT3X3>("normMat");
		m_Transform = MakeUnique<ConstantBuffer>(g, std::move(layout));
		m_Transform->CreateView(g, m_GPUHeap.Next());
	}
}

void GeometryPass::Run(Graphics& g, FrameGraph* parent)
{
	auto& outs = GetOutTargets();
	m_RTs->BindWithDepth(g, m_Depth);
	for (auto& out : outs)
	{
		out.second->Clear(g);
	}
	m_Depth->Clear(g);

	BindBindables(g);

	ID3D12DescriptorHeap* heaps[] = { m_GPUHeap.GetHeap(), m_SamplerHeap.GetHeap() };
	g.CL().SetDescriptorHeaps(2, heaps);
	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap.GPUStart());
	g.CL().SetGraphicsRootDescriptorTable(1, m_SamplerHeap.GPUStart());

	g.CL().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const auto& cam = parent->GetCamera();
	for (const auto& m : parent->GetModels())
	{
		XMMATRIX world = m->GetWorldTransform();

		(*m_Transform)["world"] = XMMatrixTranspose(world);
		(*m_Transform)["mvp"] = XMMatrixTranspose(world * cam->View() * cam->Proj());
		(*m_Transform)["normMat"] = XMMatrixInverse(nullptr, world);

		g.CL().SetGraphicsRootConstantBufferView(2, m_Transform->GetGPUAddress());

		const auto& buffers = m->GetBuffers();
		const auto& textureIndexes = m->GetTextureIndexes();
		for (UINT i = 0; i < buffers.size(); i++)
		{
			const auto& buffPair = buffers[i];
			buffPair.first->Bind(g);
			buffPair.second->Bind(g);
			g.CL().SetGraphicsRoot32BitConstants(3, sizeof(TextureIndex) / sizeof(INT), &textureIndexes[i], 0);

			g.CL().DrawIndexedInstanced(buffPair.second->NumIndices(), 1, 0, 0, 0);
		}
	}

	g.Flush();
}