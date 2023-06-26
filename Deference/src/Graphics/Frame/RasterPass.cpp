#include "RasterPass.h"
#include "Resource/RenderTarget.h"
#include "Resource/DepthStencil.h"
#include "Entity/Step.h"


RasterPass::RasterPass(const std::string& name)
	:BindablePass(std::move(name))
{
	AddTarget("rt");
	AddTarget("ds");
}

void RasterPass::Run(Graphics& g)
{
	m_RT->BindWithOther(g, m_DS.get());
	BindablePass::Run(g);
	for (auto& step : m_Steps)
	{
		step.Run(g);
	}
}

void RasterPass::OnAdd(Graphics& g)
{
	m_RT = GetTarget("rt");
	m_DS = GetTarget("ds");
}
