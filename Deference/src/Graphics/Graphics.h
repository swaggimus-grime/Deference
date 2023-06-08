#pragma once

#include <chrono>
#include <unordered_map>

class RootSig;
class Pipeline;
class VertexBuffer;
class IndexBuffer;
class Texture2D;

struct Transform {
	XMMATRIX Model;
	XMMATRIX ModelView;
	XMMATRIX MVP;
};

class Graphics {
public:
	Graphics(HWND hWnd, UINT width, UINT height);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();

	inline IDXGISwapChain4& SC() const { return *m_SC.Get(); }
	inline ID3D12Device2& Device() const { return *m_Device.Get(); }
	inline ID3D12GraphicsCommandList& CL() const { return *m_CmdList.Get(); }

	inline UINT Width() const { return m_Width; }
	inline UINT Height() const { return m_Height; }
	void OnWindowResize(UINT width, UINT height);

	inline auto& BackBuffs() const { return m_BackBuffs; }

	void Render();

	void CreateBuffer(ComPtr<ID3D12Resource>& buffer, SIZE_T size, const void* data, D3D12_RESOURCE_STATES state);

	static D3D_ROOT_SIGNATURE_VERSION ROOT_SIG_VERSION;

private:
	struct AccelerationStructureBuffers
	{
		ComPtr<ID3D12Resource> pScratch;
		ComPtr<ID3D12Resource> pResult;
		ComPtr<ID3D12Resource> pInstanceDesc;
	};

	AccelerationStructureBuffers CreateBottomLevelAS(const std::vector<std::pair<VertexBuffer*, uint32_t>>& vVertexBuffers);

	void CreateTopLevelAS(
		const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances // pair of bottom level AS and matrix of the instance
	);


private:

	AccelerationStructureBuffers m_topLevelASBuffers;
	ComPtr<ID3D12Resource> m_BottomLevelAS;
	std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> m_Instances;
	ComPtr<ID3D12RootSignature> m_RayGenSig;
	ComPtr<ID3D12RootSignature> m_HitSig;
	ComPtr<ID3D12RootSignature> m_MissSig;
	ComPtr<IDxcBlob> m_RayGenLib;
	ComPtr<IDxcBlob> m_HitLib;
	ComPtr<IDxcBlob> m_MissLib;
	
	ComPtr<ID3D12Resource> m_SbtStorage;
	// Ray tracing pipeline state
	ComPtr<ID3D12StateObject> m_DXRState;
	// Ray tracing pipeline state properties, retaining the shader identifiers
	// to use in the Shader Binding Table
	ComPtr<ID3D12StateObjectProperties> m_DXRStateProps;
	ComPtr<ID3D12Resource> m_OutputRes;
	ComPtr<ID3D12DescriptorHeap> m_SrvUavHeap;

	UINT m_Width;
	UINT m_Height;

	static constexpr UINT s_NumBuffs = 2;
	std::array<ComPtr<ID3D12Resource>, s_NumBuffs > m_BackBuffs;

	ComPtr<ID3D12Device5> m_Device;
	bool m_AllowTearing;
	ComPtr<IDXGISwapChain4> m_SC;

	ComPtr<ID3D12CommandQueue> m_CQ;
	ComPtr<ID3D12CommandAllocator> m_Alloc;
	ComPtr<ID3D12GraphicsCommandList4> m_CmdList;

	ComPtr<ID3D12Fence> m_Fence;
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue = 0;

	ComPtr<ID3D12DescriptorHeap> m_RTHeap;
	UINT m_RtvSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_RtvHandle;

	ComPtr<ID3D12Resource> m_DS;
	ComPtr<ID3D12DescriptorHeap> m_DSHeap;

	ComPtr<ID3D12PipelineState> m_Pipeline;
	//std::unique_ptr<RootSig> m_Sig;
	//std::unique_ptr<Texture2D> m_Tex;
	ComPtr<ID3D12RootSignature> m_Sig;

	//ComPtr<ID3D12Resource> m_CB;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	std::unique_ptr<VertexBuffer> m_VB;
	std::unique_ptr<IndexBuffer> m_IB;
};