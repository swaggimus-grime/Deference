#include "LambertianGraph.h"
#include "Graphics.h"
#include "LambertianPass.h"
#include "ClearPass.h"
#include "Bindable/Viewport.h"
#include "Shader/RootSig.h"
#include "Pipeline/LambertianPipeline.h"
#include "ShadowTracePass.h"

LambertianGraph::LambertianGraph(Graphics& g, const SceneData& scene)
	:FrameGraph(g, scene), m_Cam(std::move(scene.m_Camera))
{
	auto topLevelAS = scene.CreateTopLevelAS(g);

	AddBindable(MakeShared<Viewport>(g));
	RootParams params;
	params.AddInline(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
	DescTable table;
	table.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	params.AddTable(std::move(table), D3D12_SHADER_VISIBILITY_PIXEL);
	params.AddSampler(SAMPLER_FILTER_MODE::BILINEAR, SAMPLER_ADDRESS_MODE::BORDER);
	auto sig = MakeShared<RootSig>(g, std::move(params));
	AddBindable(MakeShared<LambertianPipeline>(g, *sig));
	AddBindable(std::move(sig));
	AddBindable(m_Cam);

	{
		auto pass = MakeShared<ClearPass>("clearBB");
		Connect("#.rt", pass, "target");
		AddPass(g, std::move(pass));
	}
	{
		auto pass = MakeShared<ClearPass>("clearDS");
		Connect("#.ds", pass, "target");
		AddPass(g, std::move(pass));
	}
	{
		auto pass = MakeShared<LambertianPass>(g, "lambertian");
		Connect("clearBB.target", pass, "rt");
		Connect("clearDS.target", pass, "ds");
		AddPass(g, std::move(pass));
	}
	{
		auto pass = MakeShared<ShadowTracePass>(g, "shadowTrace", std::move(topLevelAS), m_Cam);
		Connect("lambertian.rt", pass, "rt");
		AddPass(g, std::move(pass));
	}

	AddSteps(std::move(scene.m_Steps));
}
