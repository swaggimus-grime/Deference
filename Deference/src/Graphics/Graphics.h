#pragma once

#include <wrl.h>
#include "util.h"
#include <chrono>
#include <unordered_map>
#include <functional>

class Context;
class Pipeline;
class RootSig;
class RenderTarget;
class Swapchain;

class Graphics {
public:
	Graphics(HWND hWnd, UINT width, UINT height);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();

	//inline void SetOnResize(std::function<void(UINT, UINT)> callback) { m_OnResize = callback; }
	void OnResize(UINT w, UINT h);

	inline ID3D12Device5& Device() const { return *m_Device.Get(); }
	inline ID3D12GraphicsCommandList4& CL() const { return *m_CmdList.Get(); }
	inline ID3D12CommandAllocator& CA() const { return *m_Alloc.Get(); }
	inline auto& CQ() const { return *m_CQ.Get(); }

	void Flush();
	void Wait();

	inline UINT Width() const { return m_Width; }
	inline UINT Height() const { return m_Height; }

	void BeginFrame();
	void EndFrame();

	void CopyToCurrentBB(Shared<RenderTarget> src);

	void CreateBuffer(ComPtr<ID3D12Resource>& buffer, SIZE_T size, const void* data, D3D12_RESOURCE_STATES state);
	void CreateBuffer(ComPtr<ID3D12Resource>& buffer, SIZE_T size, D3D12_HEAP_TYPE heap, D3D12_RESOURCE_STATES state);

	static D3D_ROOT_SIGNATURE_VERSION ROOT_SIG_VERSION;

	static constexpr UINT s_NumInFlightFrames = 2;

private:
	UINT m_Width;
	UINT m_Height;

	ComPtr<ID3D12Device5> m_Device;
	bool m_AllowTearing;

	Unique<Swapchain> m_SC;
	
	ComPtr<ID3D12CommandQueue> m_CQ;
	ComPtr<ID3D12CommandAllocator> m_Alloc;
	ComPtr<ID3D12GraphicsCommandList4> m_CmdList;

	ComPtr<ID3D12Fence> m_Fence;
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue = 0;

	std::function<void(UINT, UINT)> m_OnResize;
	bool m_Resized;
};