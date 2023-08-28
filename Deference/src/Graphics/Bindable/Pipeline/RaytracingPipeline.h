#pragma once

#include "Bindable/Bindable.h"

enum class SHADER_TABLE_TYPE
{
	RAY_GEN,
	MISS,
	HIT
};

class ShaderBindTable;

class RaytracingPipeline : public Bindable
{
public:
	virtual void Bind(Graphics& g) override;
	inline auto* GetProps() const { return m_Props.Get(); }
	void Dispatch(Graphics& g);

	void SubmitTable(SHADER_TABLE_TYPE type, Shared<ShaderBindTable> table);

protected:
	void Create(Graphics& g, CD3DX12_STATE_OBJECT_DESC& desc);
	ComPtr<IDxcBlob> CreateLibrary(const std::wstring& path);

private:
	struct DXC
	{
		IDxcCompiler* m_Compiler;
		IDxcLibrary* m_Lib;
		IDxcIncludeHandler* m_Includer;

		DXC()
		{
			HR DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<void**>(&m_Compiler));
			HR DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<void**>(&m_Lib));
			HR m_Lib->CreateIncludeHandler(&m_Includer);
		}
	};

	static DXC s_DXC;


protected:
	inline void SetGlobalSig(Unique<RootSig>&& sig) { m_GlobalSig = std::move(sig); }
	inline void AddLocalSig(Unique<RootSig>&& sig) { m_Sigs.push_back(std::move(sig)); }

private:
	Shared<ShaderBindTable> m_RayGenTable;
	Shared<ShaderBindTable> m_MissTable;
	Shared<ShaderBindTable> m_HitTable;

	ComPtr<ID3D12StateObject> m_State;
	ComPtr<ID3D12StateObjectProperties> m_Props;

	D3D12_DISPATCH_RAYS_DESC m_Dispatch;
	Unique<RootSig> m_GlobalSig;
	std::vector<Unique<RootSig>> m_Sigs;
};

class ShaderBindTable
{
public:
	ShaderBindTable(Graphics& g, RaytracingPipeline* parent, UINT numRecords, SIZE_T recordSize);
	void Add(LPCWSTR shaderName, void* localArgs = nullptr, SIZE_T localArgSize = 0);
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const;
	inline auto GetSize() const { return m_Size; }
	inline auto GetStride() const { return m_RecordSize; }
	void Finish();

private:
	UINT64 m_Size;
	RaytracingPipeline* m_Parent;
	ComPtr<ID3D12Resource> m_Buffer;
	const SIZE_T m_RecordSize;
	uint8_t* m_Mapped;
};