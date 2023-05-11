#include "pch.h"
#include "Graphics.h"

Graphics::Graphics(HWND hWnd, UINT width, UINT height)
    :m_Width(width), m_Height(height)
{
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
                D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
            desc.DedicatedVideoMemory > vmemsize)
        {
            vmemsize = desc.DedicatedVideoMemory;
            HR adapter1.As(&adapter4);
        }
    }

    HR D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));

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

    bool allowTearing = false;
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = false;
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
    scd.BufferCount = numBuffs;
    scd.Scaling = DXGI_SCALING_STRETCH;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scd.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    //scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    HR factory4->CreateSwapChainForHwnd(m_CQ.Get(), hWnd, &scd, nullptr, nullptr, &sc1);
    HR factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
    sc1.As(&m_SC);

    D3D12_DESCRIPTOR_HEAP_DESC dd = {};
    dd.NumDescriptors = numBuffs;
    dd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HR m_Device->CreateDescriptorHeap(&dd, IID_PPV_ARGS(&descriptorHeap));

    rtvSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < numBuffs; ++i)
    {
        HR m_SC->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffs[i]));
        m_Device->CreateRenderTargetView(m_BackBuffs[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(rtvSize);
    }

    HR m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));

    HR m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));
    HR cmdList->Close();

    HR m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    fenceEvent  = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    BR (fenceEvent != nullptr);
}

Graphics::~Graphics()
{
    
}

void Graphics::Render()
{
    UINT currentBuffIdx = m_SC->GetCurrentBackBufferIndex();
    auto& bb = m_BackBuffs[currentBuffIdx];
    allocator->Reset();
    cmdList->Reset(allocator.Get(), nullptr);

    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(bb.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmdList->ResourceBarrier(1, &barrier);
        FLOAT col[] = { 0.f, 0.f, 0.f, 1.f };
        const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{
            descriptorHeap->GetCPUDescriptorHandleForHeapStart(), (INT)currentBuffIdx, rtvSize
        };
        cmdList->ClearRenderTargetView(rtv, col, 0, nullptr);
    }
    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(bb.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        cmdList->ResourceBarrier(1, &barrier);
    }
    {
        cmdList->Close();
        ID3D12CommandList* const lists[] = {cmdList.Get()};
        m_CQ->ExecuteCommandLists(_countof(lists), lists);
    }
    m_CQ->Signal(fence.Get(), fenceValue++);
    m_SC->Present(0, 0);
    fence->SetEventOnCompletion(fenceValue - 1, fenceEvent);
    if (::WaitForSingleObject(fenceEvent, INFINITE) == WAIT_FAILED)
        HR GetLastError();
}
