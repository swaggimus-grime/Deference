#include "pch.h"
#include "Graphics.h"
#include <ranges>
#include "Debug/Exception.h"
#include "DXR/DXRHelper.h"
#include "DXR/nv_helpers_dx12/TopLevelASGenerator.h"
#include "DXR/nv_helpers_dx12/ShaderBindingTableGenerator.h"
#include "DXR/nv_helpers_dx12/RootSignatureGenerator.h"
#include "DXR/nv_helpers_dx12/RaytracingPipelineGenerator.h"
#include "DXR/nv_helpers_dx12/BottomLevelASGenerator.h"
#include "Shader.h"
#include "Buffer.h"
#include "RootSig.h"
#include "Pipeline.h"
#include "Texture.h"

D3D_ROOT_SIGNATURE_VERSION Graphics::ROOT_SIG_VERSION = D3D_ROOT_SIGNATURE_VERSION_1_1;

nv_helpers_dx12::ShaderBindingTableGenerator m_SbtHelper;
nv_helpers_dx12::TopLevelASGenerator m_topLevelASGenerator;

Graphics::Graphics(HWND hWnd, UINT width, UINT height)
    :m_Width(width), m_Height(height)
{
    {
        ComPtr<ID3D12Debug> debug;
        HR D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
        debug->EnableDebugLayer();
    }

    ComPtr<IDXGIFactory4> factory;
    UINT factFlags = 0;
    factFlags = DXGI_CREATE_FACTORY_DEBUG;
    HR CreateDXGIFactory2(factFlags, IID_PPV_ARGS(&factory));

    SIZE_T vmemsize = 0;
    ComPtr<IDXGIAdapter1> adapter1;
    ComPtr<IDXGIAdapter4> adapter4;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter1->GetDesc1(&desc);
        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
            SUCCEEDED(D3D12CreateDevice(adapter1.Get(),
                D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) &&
            desc.DedicatedVideoMemory > vmemsize)
        {
            vmemsize = desc.DedicatedVideoMemory;
            HR adapter1.As(&adapter4);
        }
    }

    HR D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device));

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
    HR m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
    BR (options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

    ComPtr<ID3D12InfoQueue> debugInfo;
    if (m_Device.As(&debugInfo) == S_OK)
    {
        debugInfo->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        debugInfo->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        debugInfo->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        HR debugInfo->PushStorageFilter(&NewFilter);
    }

    m_AllowTearing = false;
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &m_AllowTearing, sizeof(m_AllowTearing))))
            {
                m_AllowTearing = false;
            }
        }
    }

    HR D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device));

    D3D12_COMMAND_QUEUE_DESC cqd = {};
    cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqd.NodeMask = 0;
    HR m_Device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&m_CQ));

    ComPtr<IDXGISwapChain1> sc1;
    DXGI_SWAP_CHAIN_DESC1 scd;
    SecureZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));
    scd.Width = width;
    scd.Height = height;
    scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.Stereo = FALSE;
    scd.SampleDesc = { 1, 0 };
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = s_NumBuffs;
    scd.Scaling = DXGI_SCALING_STRETCH;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scd.Flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    //scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    HR factory4->CreateSwapChainForHwnd(m_CQ.Get(), hWnd, &scd, nullptr, nullptr, &sc1);
    HR factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
    HR sc1.As(&m_SC);

    D3D12_DESCRIPTOR_HEAP_DESC dd = {};
    dd.NumDescriptors = s_NumBuffs;
    dd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HR m_Device->CreateDescriptorHeap(&dd, IID_PPV_ARGS(&m_RTHeap));

    m_RtvSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_RtvHandle = m_RTHeap->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < s_NumBuffs; ++i)
    {
        HR m_SC->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffs[i]));
        m_Device->CreateRenderTargetView(m_BackBuffs[i].Get(), nullptr, m_RtvHandle);
        m_RtvHandle.Offset(m_RtvSize);
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HR m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSHeap));

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    {
        const CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
        const auto res = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_Width, m_Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        m_Device->CreateCommittedResource(
            &heap,
            D3D12_HEAP_FLAG_NONE,
            &res,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&m_DS)
        );
    }
    m_DSHeap->SetName(L"Depth/Stencil Resource Heap");
    m_Device->CreateDepthStencilView(m_DS.Get(), &depthStencilDesc, m_DSHeap->GetCPUDescriptorHandleForHeapStart());

    HR m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Alloc));

    HR m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_Alloc.Get(), nullptr, IID_PPV_ARGS(&m_CmdList));

    HR m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
    m_FenceValue = 0;
    m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    BR(m_FenceEvent != nullptr);

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = ROOT_SIG_VERSION;
    if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        ROOT_SIG_VERSION = D3D_ROOT_SIGNATURE_VERSION_1_0;

    {
        //RootParams params;
        ////// Constant Buffer
        ////params.AddInline(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
        ////// Texture
        ////DescTable table;
        ////table.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
        ////params.AddTable(std::move(table), D3D12_SHADER_VISIBILITY_PIXEL);
        ////params.AddSampler(SAMPLER_MODE::BORDER);

        //m_Sig = MakeUnique<RootSig>(*this, std::move(params));
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(
            0, nullptr, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HR D3D12SerializeRootSignature(
            &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        HR m_Device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&m_Sig));
    }

    /*UINT16 idata[] = {
        0, 1, 2, 
        2, 3, 0,
        4, 5, 6,
        6, 7, 4
    };*/

    using enum VERTEX_ATTRIBUTES;
    VertexStream stream(POS | COLOR /*| TEX | NORM */ , 3);
    stream.Pos(0) = { -0.5f, -0.5f, 0.f };
    stream.Pos(1) = {-0.5f,  0.5f, 0.f};
    stream.Pos(2) = { 0.5f,  0.5f, 0.f};
    stream.Color(0) = { 1.f, 0.f, 0.f, 1.f };
    stream.Color(1) = { 0.f, 1.f, 0.f, 1.f };
    stream.Color(2) = { 0.f, 0.f, 1.f, 1.f };
    /*stream.Pos(3) = { 0.5f, -0.5f, 0.f};
    stream.Tex(0) = { 0.f, 0.f };
    stream.Tex(1) = { 0.f, 1.f };
    stream.Tex(2) = { 1.f, 1.f };
    stream.Tex(3) = { 1.f, 0.f };
    stream.Norm(0) = { 1.f, 0.f, 0.f };
    stream.Norm(1) = {0.f, 1.f, 0.f};
    stream.Norm(2) = {0.f, 0.f, 1.f};
    stream.Norm(3) = { 1.f, 1.f, 0.f };

    stream.Pos(4) = { 0.0f, -1.5f, 0.5f };
    stream.Pos(5) = { 0.0f,  1.5f, 0.5f };
    stream.Pos(6) = { 1.5f,  1.5f, 0.5f };
    stream.Pos(7) = { 1.5f, -1.5f, 0.5f };
    stream.Tex(4) = { 0.f, 0.f };
    stream.Tex(5) = { 0.f, 1.f };
    stream.Tex(6) = { 1.f, 1.f };
    stream.Tex(7) = { 1.f, 0.f };
    stream.Norm(4) = { 1.f, 0.f, 0.f };
    stream.Norm(5) = { 0.f, 1.f, 0.f };
    stream.Norm(6) = { 0.f, 0.f, 1.f };
    stream.Norm(7) = { 1.f, 1.f, 0.f };*/

    m_VB = MakeUnique<VertexBuffer>(*this, stream);
    //m_IB = MakeUnique<IndexBuffer>(*this, _countof(idata), idata);

    VertexShader vs(L"shaders\\PosColorVS.hlsl");
    PixelShader ps(L"shaders\\PosColorPS.hlsl");

    //{
    //    const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    //    const auto res = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64);
    //    HR m_Device->CreateCommittedResource(
    //        &heap, // this heap will be used to upload the constant buffer data
    //        D3D12_HEAP_FLAG_NONE, // no flags
    //        &res, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
    //        D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
    //        nullptr, // we do not have use an optimized clear value for constant buffers
    //        IID_PPV_ARGS(&m_CB));
    //    m_CB->SetName(L"Constant Buffer Upload Resource Heap");
    //}
    //{
    //    XMMATRIX model = XMMatrixRotationRollPitchYaw(0.f, PI / 2.f, PI / 2.f);
    //    XMMATRIX modelView = model * XMMatrixLookAtLH(XMVectorSet(-.1f, 0.f, -2.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 0.f), XMVectorSet(0.f, 1.f, 0.f, 0.f));
    //    XMMATRIX projection = XMMatrixTranspose(modelView * XMMatrixPerspectiveFovLH(PI / 2, m_Width / m_Height, 0.01, 10.f));
    //    modelView = XMMatrixTranspose(modelView);
    //    model = XMMatrixTranspose(model);
    //    Transform t{
    //        model, modelView, projection
    //    };
    //    Transform* mapped;
    //    HR m_CB->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    //    std::memcpy(mapped, &t, sizeof(t));
    //    //m_CB->Unmap(0, nullptr);
    //}

    // m_Tex = MakeUnique<Texture2D>(*this, L"textures/fargoth_ur.png");

    //m_Pipeline = MakeUnique<Pipeline>(*this, *m_Sig, vs, ps, m_VB->Layout());
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = m_VB->Layout();
        psoDesc.pRootSignature = m_Sig.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.ByteCode());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.ByteCode());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        HR m_Device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&m_Pipeline));
    }

    {
        // Build the bottom AS from the Triangle vertex buffer
        AccelerationStructureBuffers bottomLevelBuffers =
            CreateBottomLevelAS({ {m_VB.get(), 3 }});

        // Just one instance for now
        m_Instances = { {bottomLevelBuffers.pResult, XMMatrixIdentity()} };
        CreateTopLevelAS(m_Instances);

        // Flush the command list and wait for it to finish
        m_CmdList->Close();
        ID3D12CommandList* ppCommandLists[] = { m_CmdList.Get() };
        m_CQ->ExecuteCommandLists(1, ppCommandLists);
        m_FenceValue++;
        m_CQ->Signal(m_Fence.Get(), m_FenceValue);

        m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
        WaitForSingleObject(m_FenceEvent, INFINITE);

        // Once the command list is finished executing, reset it to be reused for
        // rendering
        HR m_CmdList->Reset(m_Alloc.Get(), m_Pipeline.Get());

        // Store the AS buffers. The rest of the buffers will be released once we exit
        // the function
        m_BottomLevelAS = bottomLevelBuffers.pResult;
    }

    HR m_CmdList->Close();

    {
        nv_helpers_dx12::RayTracingPipelineGenerator pipeline(m_Device.Get());

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
            rsc.AddHeapRangesParameter(
                { {0 /*u0*/, 1 /*1 descriptor */, 0 /*use the implicit register space 0*/,
                  D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/,
                  0 /*heap slot where the UAV is defined*/},
                 {0 /*t0*/, 1, 0,
                  D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/,
                  1} });

            m_RayGenSig = rsc.Generate(m_Device.Get(), true);
        }
        {
            nv_helpers_dx12::RootSignatureGenerator rsc;
            m_MissSig = rsc.Generate(m_Device.Get(), true);
        }
        {
            nv_helpers_dx12::RootSignatureGenerator rsc;
            rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV);
            m_HitSig = rsc.Generate(m_Device.Get(), true);
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
        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.DepthOrArraySize = 1;
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        resDesc.Width = m_Width;
        resDesc.Height = m_Height;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resDesc.MipLevels = 1;
        resDesc.SampleDesc.Count = 1;
        m_Device->CreateCommittedResource(
            &nv_helpers_dx12::kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr,
            IID_PPV_ARGS(&m_OutputRes));
    }
    {
        // Create a SRV/UAV/CBV descriptor heap. We need 2 entries - 1 UAV for the
        // raytracing output and 1 SRV for the TLAS
        m_SrvUavHeap = nv_helpers_dx12::CreateDescriptorHeap(
            m_Device.Get(), 2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

        // Get a handle to the heap memory on the CPU side, to be able to write the
        // descriptors directly
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle =
            m_SrvUavHeap->GetCPUDescriptorHandleForHeapStart();

        // Create the UAV. Based on the root signature we created it is the first
        // entry. The Create*View methods write the view information directly into
        // srvHandle
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        m_Device->CreateUnorderedAccessView(m_OutputRes.Get(), nullptr, &uavDesc,
            srvHandle);

        // Add the Top Level AS SRV right after the raytracing output buffer
        srvHandle.ptr += m_Device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.RaytracingAccelerationStructure.Location =
            m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
        // Write the acceleration structure view in the heap
        m_Device->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
    }
    {
        // The SBT helper class collects calls to Add*Program.  If called several
        // times, the helper must be emptied before re-adding shaders.
        m_SbtHelper.Reset();

        // The pointer to the beginning of the heap is the only parameter required by
        // shaders without root parameters
        const D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle =
            m_SrvUavHeap->GetGPUDescriptorHandleForHeapStart();

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
            { reinterpret_cast<void*>(m_VB->Res()->GetGPUVirtualAddress()) });

        // Compute the size of the SBT given the number of shaders and their
        // parameters
        const uint32_t sbtSize = m_SbtHelper.ComputeSBTSize();

        // Create the SBT on the upload heap. This is required as the helper will use
        // mapping to write the SBT contents. After the SBT compilation it could be
        // copied to the default heap for performance.
        m_SbtStorage = nv_helpers_dx12::CreateBuffer(
            m_Device.Get(), sbtSize, D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);
        if (!m_SbtStorage) {
            throw std::logic_error("Could not allocate the shader binding table");
        }
        // Compile the SBT from the shader and parameters info
        m_SbtHelper.Generate(m_SbtStorage.Get(), m_DXRStateProps.Get());
    }

    // Fill out the Viewport
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width = m_Width;
    m_Viewport.Height = m_Height;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    m_ScissorRect.left = 0;
    m_ScissorRect.top = 0;
    m_ScissorRect.right = m_Width;
    m_ScissorRect.bottom = m_Height;
}

Graphics::~Graphics()
{
    HR m_CQ->Signal(m_Fence.Get(), ++m_FenceValue);
    HR m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
    if (::WaitForSingleObject(m_FenceEvent, 3000) == WAIT_FAILED)
        HR GetLastError();

    
}

void Graphics::OnWindowResize(UINT width, UINT height)
{
    m_Width = width;
    m_Height = height;

    /*Flush(m_CQ, m_Fence, m_FenceValue, m_FenceEvent);
    for(UINT i = 0; i < s_NumBuffs; i++)
        m_BackBuffs[i].Reset();

    DXGI_SWAP_CHAIN_DESC scd = {};
    HR m_SC->GetDesc(&scd);
    HR m_SC->ResizeBuffers(s_NumBuffs, m_Width, m_Height, scd.BufferDesc.Format, scd.Flags);
    m_RtvHandle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < s_NumBuffs; i++) {
        HR m_SC->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffs[i]));
        m_Device->CreateRenderTargetView(m_BackBuffs[i].Get(), nullptr, m_RtvHandle);
        m_RtvHandle.Offset(m_RtvSize);
    }*/
}

void Graphics::Render()
{
    UINT currentBuffIdx = m_SC->GetCurrentBackBufferIndex();
    auto& bb = m_BackBuffs[currentBuffIdx];
    HR m_Alloc->Reset();
    HR m_CmdList->Reset(m_Alloc.Get(), m_Pipeline.Get());

    m_CmdList->SetGraphicsRootSignature(m_Sig.Get());
    m_CmdList->RSSetViewports(1, &m_Viewport); // set the viewports
    m_CmdList->RSSetScissorRects(1, &m_ScissorRect); // set the scissor rects

    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(bb.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_CmdList->ResourceBarrier(1, &barrier);
        FLOAT col[] = { 0.f, 0.f, 0.f, 1.f };
        const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{
            m_RTHeap->GetCPUDescriptorHandleForHeapStart(), (INT)currentBuffIdx, m_RtvSize
        };
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(m_DSHeap->GetCPUDescriptorHandleForHeapStart());
        m_CmdList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        //m_CmdList->ClearRenderTargetView(rtv, col, 0, nullptr);
        //m_CmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }
    {
        //m_Tex->Bind(*this);

        // set the root descriptor table 0 to the constant buffer descriptor heap
        //m_CmdList->SetGraphicsRootConstantBufferView(0, m_CB->GetGPUVirtualAddress());
        //m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
        /*m_VB->Bind(*this);
        m_IB->Bind(*this);
        m_CmdList->DrawIndexedInstanced(12, 1, 0, 0, 0);*/

        std::vector<ID3D12DescriptorHeap*> heaps = { m_SrvUavHeap.Get() };
        m_CmdList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()),
            heaps.data());

        {
            // On the last frame, the raytracing output was used as a copy source, to
            // copy its contents into the render target. Now we need to transition it to
            // a UAV so that the shaders can write in it.
            CD3DX12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(
                m_OutputRes.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            m_CmdList->ResourceBarrier(1, &transition);
        }
        {
            // Setup the raytracing task
            D3D12_DISPATCH_RAYS_DESC desc = {};
            // The layout of the SBT is as follows: ray generation shader, miss
            // shaders, hit groups. As described in the CreateShaderBindingTable method,
            // all SBT entries of a given type have the same size to allow a fixed
            // stride.

            // The ray generation shaders are always at the beginning of the SBT.
            uint32_t rayGenerationSectionSizeInBytes =
                m_SbtHelper.GetRayGenSectionSize();
            desc.RayGenerationShaderRecord.StartAddress =
                m_SbtStorage->GetGPUVirtualAddress();
            desc.RayGenerationShaderRecord.SizeInBytes =
                rayGenerationSectionSizeInBytes;
            // The miss shaders are in the second SBT section, right after the ray
            // generation shader. We have one miss shader for the camera rays and one
            // for the shadow rays, so this section has a size of 2*m_sbtEntrySize. We
            // also indicate the stride between the two miss shaders, which is the size
            // of a SBT entry
            uint32_t missSectionSizeInBytes = m_SbtHelper.GetMissSectionSize();
            desc.MissShaderTable.StartAddress =
                m_SbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes;
            desc.MissShaderTable.SizeInBytes = missSectionSizeInBytes;
            desc.MissShaderTable.StrideInBytes = m_SbtHelper.GetMissEntrySize();
            // The hit groups section start after the miss shaders. In this sample we
            // have one 1 hit group for the triangle
            uint32_t hitGroupsSectionSize = m_SbtHelper.GetHitGroupSectionSize();
            desc.HitGroupTable.StartAddress = m_SbtStorage->GetGPUVirtualAddress() +
                rayGenerationSectionSizeInBytes +
                missSectionSizeInBytes;
            desc.HitGroupTable.SizeInBytes = hitGroupsSectionSize;
            desc.HitGroupTable.StrideInBytes = m_SbtHelper.GetHitGroupEntrySize();

            // Dimensions of the image to render, identical to a kernel launch dimension
            desc.Width = m_Width;
            desc.Height = m_Height;
            desc.Depth = 1;

            // Bind the raytracing pipeline
            m_CmdList->SetPipelineState1(m_DXRState.Get());
            // Dispatch the rays and write to the raytracing output
            m_CmdList->DispatchRays(&desc);
        }
        // The raytracing output needs to be copied to the actual render target used
        // for display. For this, we need to transition the raytracing output from a
        // UAV to a copy source, and the render target buffer to a copy destination.
        // We can then do the actual copy, before transitioning the render target
        // buffer into a render target, that will be then used to display the image
        {
            auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
                m_OutputRes.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_COPY_SOURCE);
            m_CmdList->ResourceBarrier(1, &transition);
        }
        {
            auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
                bb.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_COPY_DEST);
            m_CmdList->ResourceBarrier(1, &transition);
        }
        {
            m_CmdList->CopyResource(bb.Get(),
                m_OutputRes.Get());
            auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
                bb.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_RENDER_TARGET);
            m_CmdList->ResourceBarrier(1, &transition);
        }
    }
    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(bb.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_CmdList->ResourceBarrier(1, &barrier);
    }
    {
        m_CmdList->Close();
        ID3D12CommandList* const lists[] = { m_CmdList.Get() };
        m_CQ->ExecuteCommandLists(_countof(lists), lists);
    }
    m_CQ->Signal(m_Fence.Get(), ++m_FenceValue);
    m_SC->Present(0, m_AllowTearing ? DXGI_FEATURE_PRESENT_ALLOW_TEARING : 0);
    HR m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
    if (::WaitForSingleObject(m_FenceEvent, INFINITE) == WAIT_FAILED)
        HR GetLastError();
}

void Graphics::CreateBuffer(ComPtr<ID3D12Resource>& buffer, SIZE_T size, const void* data, D3D12_RESOURCE_STATES state)
{
    {
        const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(size);
        // COPY_DEST produces an exception; possible driver issue
        HR m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &rdesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
    }
    ComPtr<ID3D12Resource> upload;
    {
        const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
        const auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(size);
        HR m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &rdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload));
    }
    {
        void* mapped = nullptr;
        HR upload->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
        std::memcpy(mapped, data, size);
        upload->Unmap(0, nullptr);
    }

    m_CmdList->CopyResource(buffer.Get(), upload.Get());
    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, state);
        m_CmdList->ResourceBarrier(1, &barrier);
    }

    HR m_CmdList->Close();
    ID3D12CommandList* const lists[] = { m_CmdList.Get() };
    m_CQ->ExecuteCommandLists(_countof(lists), lists);
    m_CQ->Signal(m_Fence.Get(), ++m_FenceValue);
    m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
    if (::WaitForSingleObject(m_FenceEvent, INFINITE) == WAIT_FAILED)
        HR GetLastError();

    HR m_Alloc->Reset();
    HR m_CmdList->Reset(m_Alloc.Get(), nullptr);
}

Graphics::AccelerationStructureBuffers Graphics::CreateBottomLevelAS(const std::vector<std::pair<VertexBuffer*, uint32_t>>& vVertexBuffers)
{
    nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS;

    // Adding all vertex buffers and not transforming their position.
    for (const auto& buffer : vVertexBuffers) {
        bottomLevelAS.AddVertexBuffer(buffer.first->Res(), 0, buffer.second,
            buffer.first->Stride(), nullptr, 0);
    }

    // The AS build requires some scratch space to store temporary information.
    // The amount of scratch memory is dependent on the scene complexity.
    UINT64 scratchSizeInBytes = 0;
    // The final AS also needs to be stored in addition to the existing vertex
    // buffers. It size is also dependent on the scene complexity.
    UINT64 resultSizeInBytes = 0;

    bottomLevelAS.ComputeASBufferSizes(m_Device.Get(), false, &scratchSizeInBytes,
        &resultSizeInBytes);

    // Once the sizes are obtained, the application is responsible for allocating
    // the necessary buffers. Since the entire generation will be done on the GPU,
    // we can directly allocate those on the default heap
    AccelerationStructureBuffers buffers;
    buffers.pScratch = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), scratchSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
        nv_helpers_dx12::kDefaultHeapProps);
    buffers.pResult = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), resultSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nv_helpers_dx12::kDefaultHeapProps);

    // Build the acceleration structure. Note that this call integrates a barrier
    // on the generated AS, so that it can be used to compute a top-level AS right
    // after this method.
    bottomLevelAS.Generate(m_CmdList.Get(), buffers.pScratch.Get(),
        buffers.pResult.Get(), false, nullptr);

    return buffers;
}

void Graphics::CreateTopLevelAS(const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances)
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

    m_topLevelASGenerator.ComputeASBufferSizes(m_Device.Get(), true, &scratchSize,
        &resultSize, &instanceDescsSize);

    // Create the scratch and result buffers. Since the build is all done on GPU,
    // those can be allocated on the default heap
    m_topLevelASBuffers.pScratch = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON,
        nv_helpers_dx12::kDefaultHeapProps);
    m_topLevelASBuffers.pResult = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nv_helpers_dx12::kDefaultHeapProps);

    // The buffer describing the instances: ID, shader binding information,
    // matrices ... Those will be copied into the buffer by the helper through
    // mapping, so the buffer has to be allocated on the upload heap.
    m_topLevelASBuffers.pInstanceDesc = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);

    // After all the buffers are allocated, or if only an update is required, we
    // can build the acceleration structure. Note that in the case of the update
    // we also pass the existing AS as the 'previous' AS, so that it can be
    // refitted in place.
    m_topLevelASGenerator.Generate(m_CmdList.Get(),
        m_topLevelASBuffers.pScratch.Get(),
        m_topLevelASBuffers.pResult.Get(),
        m_topLevelASBuffers.pInstanceDesc.Get());
}
