#pragma once

class UnorderedAccess;

class Raytracer : public Context
{
public:
	Raytracer(Graphics& g);
	virtual void Render(Graphics& g, Shared<RenderTarget> bb) override;

private:
	struct AccelerationStructureBuffers
	{
		ComPtr<ID3D12Resource> pScratch;
		ComPtr<ID3D12Resource> pResult;
		ComPtr<ID3D12Resource> pInstanceDesc;
	};

	AccelerationStructureBuffers CreateBottomLevelAS(Graphics& g, const std::vector<Shared<VertexBuffer>>& vVertexBuffers,
		const std::vector<Shared<IndexBuffer>>& indexBuffers) const;

	void CreateTopLevelAS(Graphics& g,
		const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances // pair of bottom level AS and matrix of the instance
	);

private:
	nv_helpers_dx12::ShaderBindingTableGenerator m_SbtHelper;
	nv_helpers_dx12::TopLevelASGenerator m_TopLevelASGenerator;
	AccelerationStructureBuffers m_TopLevelASBuffers;
	std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> m_Instances;
	ComPtr<ID3D12RootSignature> m_RayGenSig;
	ComPtr<ID3D12RootSignature> m_HitSig;
	ComPtr<ID3D12RootSignature> m_MissSig;
	ComPtr<ID3D12RootSignature> m_ShadowSig;
	ComPtr<IDxcBlob> m_RayGenLib;
	ComPtr<IDxcBlob> m_HitLib;
	ComPtr<IDxcBlob> m_MissLib;
	ComPtr<IDxcBlob> m_ShadowLib;

	std::vector<ComPtr<ID3D12Resource>> m_PerInstCBuffs;

	ComPtr<ID3D12Resource> m_SbtStorage;
	// Ray tracing pipeline state
	ComPtr<ID3D12StateObject> m_DXRState;
	// Ray tracing pipeline state properties, retaining the shader identifiers
	// to use in the Shader Binding Table
	ComPtr<ID3D12StateObjectProperties> m_DXRStateProps;
	
	Unique<Heap<ShaderAccessible>> m_Heap;
	Shared<UnorderedAccess> m_Output;
};