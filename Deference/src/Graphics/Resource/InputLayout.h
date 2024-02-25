#pragma once

#include <unordered_map>
#include <d3d12.h>
#include "Frame/GeometryPass.h"
#include "Format.h"

namespace Def
{
	enum class VERTEX_ATTRIBUTES
	{
		POS = 0,
		TEX = 0x1,
		NORM = 0x2,
		TAN = 0x4
	};

	template<VERTEX_ATTRIBUTES> struct Map;

	template<> struct Map<VERTEX_ATTRIBUTES::POS> {
		static constexpr auto name = "POSITION";
	};

	template<> struct Map<VERTEX_ATTRIBUTES::TEX> {
		static constexpr auto name = "TEXCOORD";
	};

	template<> struct Map<VERTEX_ATTRIBUTES::NORM> {
		static constexpr auto name = "NORMAL";
	};

	template<> struct Map<VERTEX_ATTRIBUTES::TAN> {
		static constexpr auto name = "TANGENT";
	};

	//template<> struct Map<VERTEX_ATTRIBUTES::BITAN> {
	//	using type = XMFLOAT3;
	//	static constexpr auto name = "BITANGENT";
	//	static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
	//};

	inline VERTEX_ATTRIBUTES operator&(VERTEX_ATTRIBUTES a, VERTEX_ATTRIBUTES b)
	{
		return static_cast<VERTEX_ATTRIBUTES>(static_cast<int>(a) & static_cast<int>(b));
	}

	inline VERTEX_ATTRIBUTES operator|(VERTEX_ATTRIBUTES a, VERTEX_ATTRIBUTES b)
	{
		return static_cast<VERTEX_ATTRIBUTES>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline void operator|=(VERTEX_ATTRIBUTES& a, VERTEX_ATTRIBUTES b)
	{
		a = static_cast<VERTEX_ATTRIBUTES>(static_cast<int>(a) | static_cast<int>(b));
	}

	class InputLayout
	{
	public:
		template<VERTEX_ATTRIBUTES att>
		void AddElement(DXGI_FORMAT fmt)
		{
			D3D12_INPUT_ELEMENT_DESC element{};
			element.Format = fmt;
			element.SemanticName = Map<att>::name;
			element.InputSlot = m_Elements.size();
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			m_Elements.push_back(std::move(element));

			m_Offsets.insert({ Map<att>::name, m_PrevOffset });
			m_PrevOffset += DXGI_FORMAT_Sizeof(fmt);
		}

		inline D3D12_INPUT_LAYOUT_DESC operator*() const { return { m_Elements.data(), (UINT)m_Elements.size() }; }
		inline const auto& GetOffsets() const { return m_Offsets; }
		inline SIZE_T GetStride() const { return m_PrevOffset; }

	private:
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_Elements;
		std::unordered_map<std::string, SIZE_T> m_Offsets;
		SIZE_T m_PrevOffset = 0;
	};

}