#include "Graphics.h"
#include <ranges>
#include "Debug/Exception.h"
#include "Entity/Camera.h"
#include "Swapchain.h"
#include <imgui_impl_dx12.h>

D3D_ROOT_SIGNATURE_VERSION Graphics::ROOT_SIG_VERSION = D3D_ROOT_SIGNATURE_VERSION_1_1;

Graphics::Graphics(HWND hWnd, UINT width, UINT height)
    :m_Width(width), m_Height(height), m_Resized(false)
{
#ifdef _DEBUG
    {
        ComPtr<ID3D12Debug> debug;
        ComPtr<ID3D12Debug1> debug1;
        HR D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
        HR debug->QueryInterface(IID_PPV_ARGS(&debug1));
        debug1->SetEnableGPUBasedValidation(true);
        debug1->EnableDebugLayer();
    }
#endif

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
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
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

    HR D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device));

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
    HR m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
    BR(options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

    D3D12_COMMAND_QUEUE_DESC cqd = {};
    cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqd.NodeMask = 0;
    HR m_Device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&m_CQ));
    HR m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Alloc));
    HR m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_Alloc.Get(), nullptr, IID_PPV_ARGS(&m_CmdList));

    m_SC = MakeUnique<Swapchain>(*this, hWnd, s_NumInFlightFrames);

    HR m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
    m_FenceValue = 0;
    m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    BR(m_FenceEvent != nullptr);

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = ROOT_SIG_VERSION;
    if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        ROOT_SIG_VERSION = D3D_ROOT_SIGNATURE_VERSION_1_0;

    m_CmdList->Close();

    HR m_Alloc->Reset();
    HR m_CmdList->Reset(m_Alloc.Get(), nullptr);
    
    UI::InitGraphics(*this);
}

Graphics::~Graphics()
{
    HR m_CQ->Signal(m_Fence.Get(), ++m_FenceValue);
    HR m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
    if (::WaitForSingleObject(m_FenceEvent, 3000) == WAIT_FAILED)
        HR GetLastError();

    ImGui_ImplDX12_Shutdown();
}

void Graphics::OnResize(UINT w, UINT h)
{
    m_Width = w;
    m_Height = h;
    m_SC->OnResize(*this, m_Width, m_Height);
}

void Graphics::Flush()
{
    m_CmdList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_CmdList.Get()};
    m_CQ->ExecuteCommandLists(1, ppCommandLists);
    m_FenceValue++;
    m_CQ->Signal(m_Fence.Get(), m_FenceValue);

    m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
    WaitForSingleObject(m_FenceEvent, INFINITE);

    HR m_Alloc->Reset();
    m_CmdList->Reset(m_Alloc.Get(), nullptr);
}

void Graphics::Wait()
{
    if (m_Fence->GetCompletedValue() < m_FenceValue)
    {
        HR m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
        ::WaitForSingleObject(m_FenceEvent, INFINITE);
    }
}

void Graphics::BeginFrame()
{   
    auto bb = m_SC->CurrentBB();
    bb->Transition(*this, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    bb->Clear(*this);
}

void Graphics::EndFrame()
{
    auto bb = m_SC->CurrentBB();
    bb->Transition(*this, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    Flush();
    m_SC->Present();
}

void Graphics::CopyToCurrentBB(Shared<RenderTarget> src)
{
    auto bb = m_SC->CurrentBB();
    src->Transition(*this, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
    bb->Transition(*this, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    m_CmdList->CopyResource(**bb, **src);

    src->Transition(*this, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    bb->Transition(*this, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
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

void Graphics::CreateBuffer(ComPtr<ID3D12Resource>& buffer, SIZE_T size, D3D12_HEAP_TYPE heap, D3D12_RESOURCE_STATES state)
{
    const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(heap);
    const auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    HR m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &rdesc, state, nullptr, IID_PPV_ARGS(&buffer));
}
