#pragma once

#include "Bindable/Bindable.h"

class SubObject;

class RaytracingPipeline : public Bindable
{
public:
	virtual void Bind(Graphics& g) override;
	void ComputeShaderTableAndDispatch(Graphics& g, 
		const std::vector<std::pair<const std::wstring&, const std::vector<void*>&>>& raygen,
		const std::vector<std::pair<const std::wstring&, const std::vector<void*>&>>& miss,
		const std::vector<std::pair<const std::wstring&, const std::vector<void*>&>>& hit);
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

private:
	ComPtr<ID3D12StateObject> m_State;
	ComPtr<ID3D12StateObjectProperties> m_Props;
	ComPtr<ID3D12Resource> m_Table;

	Shared<RootSig> m_GlobalSig;
};