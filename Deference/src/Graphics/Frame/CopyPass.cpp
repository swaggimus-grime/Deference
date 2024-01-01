#include "CopyPass.h"
#include "Bindable/Pipeline/VertexBuffer.h"
#include "Bindable/Pipeline/IndexBuffer.h"
#include "Scene/Camera.h"
#include "FrameGraph.h"
#include "VecOps.h"

CopyPass::CopyPass(Graphics& g, const std::string& name, FrameGraph* parent)
	:ScreenPass(g, std::move(name), parent)
{
	AddInTarget("Target");
	AddOutTarget(g, "Target");
}

void CopyPass::Run(Graphics& g)
{
	auto src = GetInTarget("Target");
	auto dest = GetOutTarget("Target");
	dest->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	src->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

	g.CL().CopyResource(**dest, **src);

	dest->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	src->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	g.Flush();
}