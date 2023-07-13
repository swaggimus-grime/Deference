#pragma once

#include "Bindable/Bindable.h"

class SubObject;

class RaytracingPipeline : public Bindable
{
public:
	virtual void Bind(Graphics& g) override;
	void UpdateTable(Graphics& g,
		const std::vector<std::pair<LPCWSTR, const std::vector<void*>&>>& raygen,
		const std::vector<std::pair<LPCWSTR, const std::vector<void*>&>>& miss,
		const std::vector<std::pair<LPCWSTR, const std::vector<void*>&>>& hit);
	void Dispatch(Graphics& g);
protected:
	void Create(Graphics& g, CD3DX12_STATE_OBJECT_DESC& desc, 
		const std::vector<UINT>& rgNumArgsPerEntry, 
		const std::vector<UINT>& missNumArgsPerEntry, 
		const std::vector<UINT>& hitNumArgsPerEntry);
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

	D3D12_DISPATCH_RAYS_DESC m_Dispatch;
	UINT m_RayGenSize;
	UINT m_MissSize;
	UINT m_HitSize;
};