#pragma once

#include "Bindable/Bindable.h"

class SubObject;

class RaytracingPipeline : public Bindable
{
public:
	void Dispatch(Graphics& g);
	virtual void Bind(Graphics& g) override;

protected:
	void Create(Graphics& g, CD3DX12_STATE_OBJECT_DESC& desc, 
		const std::vector<std::pair<const std::wstring&, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>&>>& raygen,
		const std::vector<std::pair<const std::wstring&, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>&>>& miss,
		const std::vector<std::pair<const std::wstring&, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>&>>& hit);
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

private:
	D3D12_DISPATCH_RAYS_DESC m_Dispatch = {};
	ComPtr<ID3D12StateObject> m_State;
	ComPtr<ID3D12StateObjectProperties> m_Props;
	ComPtr<ID3D12Resource> m_Table;

	Shared<RootSig> m_GlobalSig;
};