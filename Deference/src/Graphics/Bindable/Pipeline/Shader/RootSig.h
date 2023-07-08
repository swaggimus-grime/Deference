#pragma once

#include "Bindable/Bindable.h"

class DescTable
{
public:
	void AddRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT slot = 0, UINT space = 0, UINT numDescs = 1);
	inline UINT NumRanges() const { return m_Ranges.size(); }
	inline auto* Ranges() const { return m_Ranges.data(); }

private:
	std::vector<D3D12_DESCRIPTOR_RANGE1> m_Ranges;
	D3D12_DESCRIPTOR_RANGE_TYPE m_Type;
	UINT m_OffsetInDescriptors = 0;
};

enum class SAMPLER_FILTER_MODE
{
	NEAREST,
	BILINEAR
};

enum class SAMPLER_ADDRESS_MODE
{
	BORDER
};

class RootParams
{
public:
	void AddTable(const DescTable& table, D3D12_SHADER_VISIBILITY visibility);
	void AddInline(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility, UINT slot = 0);
	void AddSampler(SAMPLER_FILTER_MODE filter, SAMPLER_ADDRESS_MODE address, UINT slot = 0);
	inline D3D12_ROOT_SIGNATURE_FLAGS Flags() const { return m_Flags; }

	inline UINT NumParams() const { return m_Params.size(); }
	inline auto* Params() const { return m_Params.data(); }
	inline UINT NumSamplers() const { return m_Samplers.size(); }
	inline auto* Samplers() const { return m_Samplers.data(); }

private:
	void CheckFlags(D3D12_SHADER_VISIBILITY visibility);

private:
	std::vector<D3D12_ROOT_PARAMETER1> m_Params;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_Samplers;
	D3D12_ROOT_SIGNATURE_FLAGS m_Flags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
};

class RootSig : public Bindable
{
public:
	RootSig(Graphics& g, const RootParams& params, bool local = false);
	RootSig(Graphics& g, SIZE_T numParams, const CD3DX12_ROOT_PARAMETER1* params);
	inline auto* Sig() const { return m_Sig.Get(); }
	virtual void Bind(Graphics& g) override;

private:
	ComPtr<ID3D12RootSignature> m_Sig;
};