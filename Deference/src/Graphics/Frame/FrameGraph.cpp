#include "FrameGraph.h"
#include "FrameGraph.h"
#include "Pass.h"
#include "util.h"
#include "Entity/Step.h"
#include "DXR/nv_helpers_dx12/TopLevelASGenerator.h"

void SceneData::AddDrawableCollection(const DrawableCollection& d)
{
    std::move(d.GetInstances().begin(), d.GetInstances().end(), std::back_inserter(m_Instances));
    std::move(d.GetSteps().begin(), d.GetSteps().end(), std::back_inserter(m_Steps));
}

void SceneData::AddDrawable(const Drawable& d)
{
    m_Instances.push_back(std::move(d.GetInstance()));
    std::move(d.GetSteps().begin(), d.GetSteps().end(), std::back_inserter(m_Steps));
}

ComPtr<ID3D12Resource> SceneData::CreateTopLevelAS(Graphics& g) const
{
    nv_helpers_dx12::TopLevelASGenerator m_TopLevelASGenerator;

    auto& instances = m_Instances;
    for (size_t i = 0; i < instances.size(); i++)
    {
        m_TopLevelASGenerator.AddInstance(instances[i].first.Get(), instances[i].second,
            static_cast<UINT>(i), static_cast<UINT>(2 * i));
    }

    // As for the bottom-level AS, the building the AS requires some scratch space
    // to store temporary data in addition to the actual AS. In the case of the
    // top-level AS, the instance descriptors also need to be stored in GPU
    // memory. This call outputs the memory requirements for each (scratch,
    // results, instance descriptors) so that the application can allocate the
    // corresponding memory
    UINT64 scratchSize, resultSize, instanceDescsSize;

    m_TopLevelASGenerator.ComputeASBufferSizes(&g.Device(), true, &scratchSize,
        &resultSize, &instanceDescsSize);

    // Create the scratch and result buffers. Since the build is all done on GPU,
    // those can be allocated on the default heap
    AccelerationStructureBuffers m_TopLevelASBuffers;
    m_TopLevelASBuffers.pScratch = nv_helpers_dx12::CreateBuffer(
        &g.Device(), scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON,
        nv_helpers_dx12::kDefaultHeapProps);
    m_TopLevelASBuffers.pResult = nv_helpers_dx12::CreateBuffer(
        &g.Device(), resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nv_helpers_dx12::kDefaultHeapProps);

    // The buffer describing the instances: ID, shader binding information,
    // matrices ... Those will be copied into the buffer by the helper through
    // mapping, so the buffer has to be allocated on the upload heap.
    m_TopLevelASBuffers.pInstanceDesc = nv_helpers_dx12::CreateBuffer(
        &g.Device(), instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);

    // After all the buffers are allocated, or if only an update is required, we
    // can build the acceleration structure. Note that in the case of the update
    // we also pass the existing AS as the 'previous' AS, so that it can be
    // refitted in place.
    m_TopLevelASGenerator.Generate(&g.CL(),
        m_TopLevelASBuffers.pScratch.Get(),
        m_TopLevelASBuffers.pResult.Get(),
        m_TopLevelASBuffers.pInstanceDesc.Get());

    return m_TopLevelASBuffers.pResult;
}

FrameGraph::FrameGraph(Graphics& g, const SceneData& scene)
	:m_RTHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1), m_DSHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1)
{
	m_DS = m_DSHeap.AddResource<DepthStencil>(g);
	m_RT = m_RTHeap.AddResource<RenderTarget>(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void FrameGraph::Run(Graphics& g)
{
	for (auto& b : m_Bindables)
		b->Bind(g);

	for (auto& pass : m_Passes)
		pass->Run(g);
}

void FrameGraph::AddPass(Graphics& g, Shared<Pass> pass)
{
	pass->OnAdd(g);
	m_Passes.emplace_back(std::move(pass));
}

void FrameGraph::AddBindable(Shared<Bindable> bindable)
{
	m_Bindables.push_back(std::move(bindable));
}

void FrameGraph::Connect(const std::string& outPassTarget, Shared<Pass> inPass, const std::string& inTarget)
{
	auto passTarget = ParseTokens(outPassTarget, '.');
	if (passTarget[0] == "#")
	{
		if (passTarget[1] == "rt")
			inPass->GetTarget(inTarget) = m_RT;
		else if (passTarget[1] == "ds")
			inPass->GetTarget(inTarget) = m_DS;
		else
			throw DefException(std::format("Failed to find target with name \"{}\" in pass \"{}\"", passTarget[1], passTarget[0]));
	}
	else
		inPass->GetTarget(inTarget) = GetPass(passTarget[0])->GetTarget(passTarget[1]);
}

void FrameGraph::AddSteps(const std::vector<Step>& steps)
{
    for (auto& step : steps)
        GetPass(step.PassName())->AddStep(std::move(step));
}

Shared<Pass> FrameGraph::GetPass(const std::string& name)
{
    return *(std::find_if(m_Passes.begin(), m_Passes.end(), [&](const auto& pass) {return pass->Name() == name; }));
}
