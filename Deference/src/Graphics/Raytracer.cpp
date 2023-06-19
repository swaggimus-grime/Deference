#include "Raytracer.h"
#include "DXR/DXRHelper.h"
#include "DXR/nv_helpers_dx12/TopLevelASGenerator.h"
#include "DXR/nv_helpers_dx12/ShaderBindingTableGenerator.h"
#include "DXR/nv_helpers_dx12/RootSignatureGenerator.h"
#include "DXR/nv_helpers_dx12/RaytracingPipelineGenerator.h"
#include "DXR/nv_helpers_dx12/BottomLevelASGenerator.h"
#include "UnorderedAccess.h"

Raytracer::Raytracer(Graphics& g)
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
    HR g.Device().CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
    BR(options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

    auto bottomAS = CreateBottomLevelAS(g, g.Scene().m_VBs, g.Scene().m_IBs);
    m_Instances = { { bottomAS.pResult, XMMatrixIdentity() } };
    CreateTopLevelAS(g, m_Instances);

    g.Flush();
    HR g.CL().Reset(&g.CA(), g.GetPipeline());
    m_BottomLevelAS = bottomAS;

    g.CL().Close();

    {
        nv_helpers_dx12::RayTracingPipelineGenerator pipeline(&g.Device());

        // The pipeline contains the DXIL code of all the shaders potentially executed
        // during the raytracing process. This section compiles the HLSL code into a
        // set of DXIL libraries. We chose to separate the code in several libraries
        // by semantic (ray generation, hit, miss) for clarity. Any code layout can be
        // used.
        m_RayGenLib = nv_helpers_dx12::CompileShaderLibrary(L"shaders\\RayGen.hlsl");
        m_MissLib = nv_helpers_dx12::CompileShaderLibrary(L"shaders\\Miss.hlsl");
        m_HitLib = nv_helpers_dx12::CompileShaderLibrary(L"shaders\\Hit.hlsl");

        // In a way similar to DLLs, each library is associated with a number of
        // exported symbols. This
        // has to be done explicitly in the lines below. Note that a single library
        // can contain an arbitrary number of symbols, whose semantic is given in HLSL
        // using the [shader("xxx")] syntax
        pipeline.AddLibrary(m_RayGenLib.Get(), { L"RayGen" });
        pipeline.AddLibrary(m_MissLib.Get(), { L"Miss" });
        pipeline.AddLibrary(m_HitLib.Get(), { L"ClosestHit" });

        // To be used, each DX12 shader needs a root signature defining which
        // parameters and buffers will be accessed.
        {
            nv_helpers_dx12::RootSignatureGenerator rsc;
            rsc.AddHeapRangesParameter({
                { 0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0 },
                { 0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1 },
                { 0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2 }
                });
            m_RayGenSig = rsc.Generate(&g.Device(), true);
        }
        {
            nv_helpers_dx12::RootSignatureGenerator rsc;
            m_MissSig = rsc.Generate(&g.Device(), true);
        }
        {
            nv_helpers_dx12::RootSignatureGenerator rsc;
            rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV);
            rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 1);
            m_HitSig = rsc.Generate(&g.Device(), true);
        }

        // 3 different shaders can be invoked to obtain an intersection: an
        // intersection shader is called
        // when hitting the bounding box of non-triangular geometry. This is beyond
        // the scope of this tutorial. An any-hit shader is called on potential
        // intersections. This shader can, for example, perform alpha-testing and
        // discard some intersections. Finally, the closest-hit program is invoked on
        // the intersection point closest to the ray origin. Those 3 shaders are bound
        // together into a hit group.

        // Note that for triangular geometry the intersection shader is built-in. An
        // empty any-hit shader is also defined by default, so in our simple case each
        // hit group contains only the closest hit shader. Note that since the
        // exported symbols are defined above the shaders can be simply referred to by
        // name.

        // Hit group for the triangles, with a shader simply interpolating vertex
        // colors
        pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");

        // The following section associates the root signature to each shader. Note
        // that we can explicitly show that some shaders share the same root signature
        // (eg. Miss and ShadowMiss). Note that the hit shaders are now only referred
        // to as hit groups, meaning that the underlying intersection, any-hit and
        // closest-hit shaders share the same root signature.
        pipeline.AddRootSignatureAssociation(m_RayGenSig.Get(), { L"RayGen" });
        pipeline.AddRootSignatureAssociation(m_MissSig.Get(), { L"Miss" });
        pipeline.AddRootSignatureAssociation(m_HitSig.Get(), { L"HitGroup" });

        // The payload size defines the maximum size of the data carried by the rays,
        // ie. the the data
        // exchanged between shaders, such as the HitInfo structure in the HLSL code.
        // It is important to keep this value as low as possible as a too high value
        // would result in unnecessary memory consumption and cache trashing.
        pipeline.SetMaxPayloadSize(4 * sizeof(float)); // RGB + distance

        // Upon hitting a surface, DXR can provide several attributes to the hit. In
        // our sample we just use the barycentric coordinates defined by the weights
        // u,v of the last two vertices of the triangle. The actual barycentrics can
        // be obtained using float3 barycentrics = float3(1.f-u-v, u, v);
        pipeline.SetMaxAttributeSize(2 * sizeof(float)); // barycentric coordinates

        // The raytracing process can shoot rays from existing hit points, resulting
        // in nested TraceRay calls. Our sample code traces only primary rays, which
        // then requires a trace depth of 1. Note that this recursion depth should be
        // kept to a minimum for best performance. Path tracing algorithms can be
        // easily flattened into a simple loop in the ray generation.
        pipeline.SetMaxRecursionDepth(1);

        // Compile the pipeline for execution on the GPU
        m_DXRState = pipeline.Generate();

        // Cast the state object into a properties object, allowing to later access
        // the shader pointers by name
        HR m_DXRState->QueryInterface(IID_PPV_ARGS(&m_DXRStateProps));
    }
    {
        m_Heap = MakeUnique<Heap<ShaderAccessible>>(g, 3, true);
        m_Output = m_Heap->AddResource<UnorderedAccess>(g);
        m_Heap->AddResource<TopLevelAS>(g, m_topLevelASBuffers.pResult);
        m_Heap->AddResource<ConstantBuffer>(g, g.GetCamera().Res(), sizeof(Camera::CamData));
    }
    {
        // The SBT helper class collects calls to Add*Program.  If called several
        // times, the helper must be emptied before re-adding shaders.
        m_SbtHelper.Reset();

        // The pointer to the beginning of the heap is the only parameter required by
        // shaders without root parameters
        const D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle =
           m_Heap->GetHeap()->GetGPUDescriptorHandleForHeapStart();

        // The helper treats both root parameter pointers and heap pointers as void*,
        // while DX12 uses the
        // D3D12_GPU_DESCRIPTOR_HANDLE to define heap pointers. The pointer in this
        // struct is a UINT64, which then has to be reinterpreted as a pointer.
        UINT64* heapPointer = reinterpret_cast<UINT64*>(srvUavHeapHandle.ptr);

        // The ray generation only uses heap data
        m_SbtHelper.AddRayGenerationProgram(L"RayGen", { heapPointer });

        // The miss and hit shaders do not access any external resources: instead they
        // communicate their results through the ray payload
        m_SbtHelper.AddMissProgram(L"Miss", {});

        // Adding the triangle hit shader
        m_SbtHelper.AddHitGroup(L"HitGroup",
            {
                reinterpret_cast<void*>(g.Scene().m_VBs[1]->Res()->GetGPUVirtualAddress()), reinterpret_cast<void*>(g.Scene().m_IBs[1]->Res()->GetGPUVirtualAddress())
            });

        // Compute the size of the SBT given the number of shaders and their
        // parameters
        const uint32_t sbtSize = m_SbtHelper.ComputeSBTSize();

        // Create the SBT on the upload heap. This is required as the helper will use
        // mapping to write the SBT contents. After the SBT compilation it could be
        // copied to the default heap for performance.
        m_SbtStorage = nv_helpers_dx12::CreateBuffer(
            &g.Device(), sbtSize, D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);
        if (!m_SbtStorage) {
            throw std::logic_error("Could not allocate the shader binding table");
        }
        // Compile the SBT from the shader and parameters info
        m_SbtHelper.Generate(m_SbtStorage.Get(), m_DXRStateProps.Get());
    }
}

void Raytracer::Render(Graphics& g, Shared<RenderTarget> bb)
{
    bb->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
    bb->Bind(g);
    m_Heap->Bind(g);
    m_Output->Transition(g, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    D3D12_DISPATCH_RAYS_DESC desc = {};
    uint32_t rayGenerationSectionSizeInBytes =
        m_SbtHelper.GetRayGenSectionSize();
    desc.RayGenerationShaderRecord.StartAddress =
        m_SbtStorage->GetGPUVirtualAddress();
    desc.RayGenerationShaderRecord.SizeInBytes =
        rayGenerationSectionSizeInBytes;
    uint32_t missSectionSizeInBytes = m_SbtHelper.GetMissSectionSize();
    desc.MissShaderTable.StartAddress =
        m_SbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes;
    desc.MissShaderTable.SizeInBytes = missSectionSizeInBytes;
    desc.MissShaderTable.StrideInBytes = m_SbtHelper.GetMissEntrySize();
    uint32_t hitGroupsSectionSize = m_SbtHelper.GetHitGroupSectionSize();
    desc.HitGroupTable.StartAddress = m_SbtStorage->GetGPUVirtualAddress() +
        rayGenerationSectionSizeInBytes +
        missSectionSizeInBytes;
    desc.HitGroupTable.SizeInBytes = hitGroupsSectionSize;
    desc.HitGroupTable.StrideInBytes = m_SbtHelper.GetHitGroupEntrySize();
    desc.Width = g.Width();
    desc.Height = g.Height();
    desc.Depth = 1;
    // Bind the raytracing pipeline
    g.CL().SetPipelineState1(m_DXRState.Get());
    // Dispatch the rays and write to the raytracing output
    g.CL().DispatchRays(&desc);
    
    m_Output->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE);
    bb->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST);
    g.CL().CopyResource(bb->Res(), m_Output->Res());
    bb->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
    bb->Transition(g, D3D12_RESOURCE_STATE_PRESENT);
}

Raytracer::AccelerationStructureBuffers Raytracer::CreateBottomLevelAS(Graphics& g, const std::vector<Shared<VertexBuffer>>& vVertexBuffers,
    const std::vector<Shared<IndexBuffer>>& vIndexBuffers)
{
    nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS;

    for (size_t i = 0; i < vVertexBuffers.size(); i++) {
        if (i < vIndexBuffers.size() && vIndexBuffers[i]->NumIndices() > 0)
            bottomLevelAS.AddVertexBuffer(vVertexBuffers[i]->Res(), 0, vVertexBuffers[i]->NumVertices(), vVertexBuffers[i]->Stride(), vIndexBuffers[i]->Res(), 0, vIndexBuffers[i]->NumIndices(), nullptr, 0, true);
        else
            bottomLevelAS.AddVertexBuffer(vVertexBuffers[i]->Res(), 0, vVertexBuffers[i]->NumVertices(), vVertexBuffers[i]->Stride(), 0, 0);
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

    return buffers;
}

void Raytracer::CreateTopLevelAS(Graphics& g, const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances)
{
    // Gather all the instances into the builder helper
    for (size_t i = 0; i < instances.size(); i++) {
        m_topLevelASGenerator.AddInstance(instances[i].first.Get(),
            instances[i].second, static_cast<UINT>(i),
            static_cast<UINT>(0));
    }

    // As for the bottom-level AS, the building the AS requires some scratch space
    // to store temporary data in addition to the actual AS. In the case of the
    // top-level AS, the instance descriptors also need to be stored in GPU
    // memory. This call outputs the memory requirements for each (scratch,
    // results, instance descriptors) so that the application can allocate the
    // corresponding memory
    UINT64 scratchSize, resultSize, instanceDescsSize;

    m_topLevelASGenerator.ComputeASBufferSizes(&g.Device(), true, &scratchSize,
        &resultSize, &instanceDescsSize);

    // Create the scratch and result buffers. Since the build is all done on GPU,
    // those can be allocated on the default heap
    m_topLevelASBuffers.pScratch = nv_helpers_dx12::CreateBuffer(
        &g.Device(), scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON,
        nv_helpers_dx12::kDefaultHeapProps);
    m_topLevelASBuffers.pResult = nv_helpers_dx12::CreateBuffer(
        &g.Device(), resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nv_helpers_dx12::kDefaultHeapProps);

    // The buffer describing the instances: ID, shader binding information,
    // matrices ... Those will be copied into the buffer by the helper through
    // mapping, so the buffer has to be allocated on the upload heap.
    m_topLevelASBuffers.pInstanceDesc = nv_helpers_dx12::CreateBuffer(
        &g.Device(), instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);

    // After all the buffers are allocated, or if only an update is required, we
    // can build the acceleration structure. Note that in the case of the update
    // we also pass the existing AS as the 'previous' AS, so that it can be
    // refitted in place.
    m_topLevelASGenerator.Generate(&g.CL(),
        m_topLevelASBuffers.pScratch.Get(),
        m_topLevelASBuffers.pResult.Get(),
        m_topLevelASBuffers.pInstanceDesc.Get());
}