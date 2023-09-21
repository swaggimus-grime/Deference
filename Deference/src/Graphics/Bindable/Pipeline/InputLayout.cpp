#include "InputLayout.h"

#define CHECK_ATTRIB(x) if ((attributes & x) == x) { \
                            m_Elements.push_back({Map<x>::name, 0, Map<x>::format, 0, m_Stride, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}); \
                            m_Offsets.insert({ x, m_Stride }); \
                            m_Stride += sizeof(Map<x>::type); \
                        } \

InputLayout::InputLayout(const VERTEX_ATTRIBUTES& attributes)
    :m_Attributes(attributes), m_Stride(0u)
{
    using enum VERTEX_ATTRIBUTES;
    CHECK_ATTRIB(POS)
    CHECK_ATTRIB(TEX)
    CHECK_ATTRIB(NORM)
    CHECK_ATTRIB(TAN)
    CHECK_ATTRIB(BITAN)
    CHECK_ATTRIB(COLOR)

    m_Layout.NumElements = m_Elements.size();
    m_Layout.pInputElementDescs = m_Elements.data();
}

VERTEX_ATTRIBUTES InputLayout::MapConfigToAttribs(INPUT_LAYOUT_CONFIG config)
{
    using enum INPUT_LAYOUT_CONFIG;
    using enum VERTEX_ATTRIBUTES;

    switch (config)
    {
    case GEOMETRY_PIPELINE:
        return POS | TEX | NORM | TAN | BITAN | COLOR;
    case SCREEN:
        return POS | TEX;
    default:
        return POS;
    }
}

InputLayout::InputLayout(INPUT_LAYOUT_CONFIG config)
    :InputLayout(MapConfigToAttribs(config))
{
}