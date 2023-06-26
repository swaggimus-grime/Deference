#include "Drawable.h"
#include "Entity/Step.h"
#include "Frame/FrameGraph.h"

void Drawable::CreateBottomLevelAS(Graphics& g, const std::vector<Shared<VertexBuffer>>& vVertexBuffers,
    const std::vector<Shared<IndexBuffer>>& indexBuffers)
{
    nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS;

    // Adding all vertex buffers and not transforming their position.
    for (UINT i = 0; i < vVertexBuffers.size(); i++)
    {
        bottomLevelAS.AddVertexBuffer(vVertexBuffers[i]->Res(), 0, vVertexBuffers[i]->NumVertices(),
            vVertexBuffers[i]->Stride(), indexBuffers[i]->Res(), 0, indexBuffers[i]->NumIndices(), nullptr, 0, true);
    }

    // The AS build requires some scratch space to store temporary information.
    // The amount of scratch memory is dependent on the scene complexity.
    UINT64 scratchSizeInBytes = 0;
    // The final AS also needs to be stored in addition to the existing vertex
    // buffers. It size is also dependent on the scene complexity.
    UINT64 resultSizeInBytes = 0;

    bottomLevelAS.ComputeASBufferSizes(&g.Device(), false, &scratchSizeInBytes,
        &resultSizeInBytes);

    // Once the sizes are obtained, the application is responsible for allocating
    // the necessary buffers. Since the entire generation will be done on the GPU,
    // we can directly allocate those on the default heap
    AccelerationStructureBuffers buffers;
    buffers.pScratch = nv_helpers_dx12::CreateBuffer(
        &g.Device(), scratchSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
        nv_helpers_dx12::kDefaultHeapProps);
    buffers.pResult = nv_helpers_dx12::CreateBuffer(
        &g.Device(), resultSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nv_helpers_dx12::kDefaultHeapProps);

    // Build the acceleration structure. Note that this call integrates a barrier
    // on the generated AS, so that it can be used to compute a top-level AS right
    // after this method.
    bottomLevelAS.Generate(&g.CL(), buffers.pScratch.Get(),
        buffers.pResult.Get(), false, nullptr);

    m_Instance = { buffers.pResult, XMMatrixIdentity() };
}

void Drawable::AddStep(const Step& step)
{
	m_Steps.push_back(std::move(step));
}

void DrawableCollection::AddDrawable(const Drawable& d)
{
    m_Instances.push_back(std::move(d.GetInstance()));
    std::move(d.GetSteps().begin(), d.GetSteps().end(), std::back_inserter(m_Steps));
}