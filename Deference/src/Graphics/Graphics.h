#pragma once

class Graphics {
public:
	Graphics(HWND hWnd, UINT width, UINT height);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();

	inline IDXGISwapChain4& SC() const { return *m_SC.Get(); }
	inline ID3D12Device2& Device() const { return *m_Device.Get(); }

	inline UINT Width() const { return m_Width; }
	inline UINT Height() const { return m_Height; }

	inline auto& BackBuffs() const { return m_BackBuffs; }

	void Render();

private:
	UINT m_Width;
	UINT m_Height;

	static constexpr UINT numBuffs = 2;
	std::array<ComPtr<ID3D12Resource>, numBuffs > m_BackBuffs;

	ComPtr<ID3D12Device2> m_Device;
	ComPtr<IDXGISwapChain4> m_SC;

	ComPtr<ID3D12CommandQueue> m_CQ;
	ComPtr<ID3D12CommandAllocator> allocator;
	ComPtr<ID3D12GraphicsCommandList> cmdList;
	ComPtr<ID3D12Fence> fence;
	HANDLE fenceEvent;
	UINT64 fenceValue = 0;

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	UINT rtvSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
};