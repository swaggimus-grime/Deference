#include "RaytracePass.h"
#include "RaytracePass.h"
#include "RaytracePass.h"

void RaytracePass::Run(Graphics& g)
{
	for (auto& out : GetOutTargets())
		out.second->Clear(g);

	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);

	m_Pipeline->Dispatch(g);

	auto& outs = GetOutTargets();
	const auto& targets =
		std::views::iota(0u, (UINT)outs.size()) |
		std::views::transform([&](UINT i) {
		return outs.at(i).second;
			}) |
		std::ranges::to<std::vector>();

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	Resource::Transition(g, m_Outputs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	for(UINT i = 0; i < targets.size(); i++)
		g.CL().CopyResource(**targets[i], **m_Outputs[i]);
	
	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Resource::Transition(g, m_Outputs, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	g.Flush();
}

RaytracePass::RaytracePass(FrameGraph* parent)
	:Pass(parent)
{
	AddGlobalResource(parent->GetTLAS());
}

void RaytracePass::OnAdd(Graphics& g)
{
	SIZE_T numOuts = GetOutTargets().size();
	for (; numOuts > 0; numOuts--)
	{
		auto out = MakeShared<UnorderedAccess>(g);
		AddResource(out);
		m_Outputs.push_back(std::move(out));
	}

	Pass::OnAdd(g);
}

void RaytracePass::OnResize(Graphics& g, UINT w, UINT h)
{
	Pass::OnResize(g, w, h);
	for(auto& out : m_Outputs)
		out->Resize(g, w, h);
}

Shared<UnorderedAccess> RaytracePass::GetOutput(const std::string& name)
{
	return m_Outputs.at(GetOutTargetIndex(name));
}

void RaytracePass::AddModelDescriptors()
{

}
