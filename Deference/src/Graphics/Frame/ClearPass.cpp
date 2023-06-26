#include "ClearPass.h"
#include "Resource/Target.h"

ClearPass::ClearPass(const std::string& name)
	:TargetPass(std::move(name))
{
}

void ClearPass::Run(Graphics& g)
{
	//D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessClear{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, { clearValue } };
	//D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessPreserve{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };
	//D3D12_RENDER_PASS_RENDER_TARGET_DESC renderPassRenderTargetDesc{ m_Target->Handle(), renderPassBeginningAccessClear, renderPassEndingAccessPreserve};

	//g.CL().BeginRenderPass(1, &renderPassRenderTargetDesc, nullptr, D3D12_RENDER_PASS_FLAG_NONE);
	m_Target->Bind(g);
	m_Target->Clear(g);
	//g.CL().EndRenderPass();
}
