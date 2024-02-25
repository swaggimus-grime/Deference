#pragma once

#include "View.h"

namespace Def
{
	class Sampler : public SamplerView
	{
	public:
		Sampler()
			:SamplerView(nullptr)
		{
			m_Desc = {};
			m_Desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			m_Desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			m_Desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			m_Desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			m_Desc.MinLOD = 0;
			m_Desc.MaxLOD = D3D12_FLOAT32_MAX;
			m_Desc.MipLODBias = 0.0f;
			m_Desc.MaxAnisotropy = 1;
			m_Desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		}

		inline void SetAddressU(D3D12_TEXTURE_ADDRESS_MODE mode)
		{
			m_Desc.AddressU = mode;
		}

		inline void SetAddressV(D3D12_TEXTURE_ADDRESS_MODE mode)
		{
			m_Desc.AddressV = mode;
		}

		virtual const D3D12_SAMPLER_DESC& SamplerViewDesc() const override
		{
			return m_Desc;
		}

	private:
		D3D12_SAMPLER_DESC m_Desc;
	};
}