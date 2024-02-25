#pragma once

#include "Resource.h"
#include "Bindable.h"
#include "BufferElement.h"
#include <d3d12.h>
#include "Graphics.h"
#include <variant>

namespace Def
{
    #define TYPE_LIST \
        ACT_ON_TYPE(FLOAT) \
        ACT_ON_TYPE(XMFLOAT2) \
        ACT_ON_TYPE(XMFLOAT3) \
        ACT_ON_TYPE(XMFLOAT4) \
        ACT_ON_TYPE(INT) \
        ACT_ON_TYPE(UINT) \
        ACT_ON_TYPE(XMMATRIX) \
        ACT_ON_TYPE(XMFLOAT3X3) \

    enum class CONSTANT_TYPE
    {
        #define ACT_ON_TYPE(type) type,
            TYPE_LIST
        #undef ACT_ON_TYPE
    };


    template<CONSTANT_TYPE> struct ConstType;

    #define MapEnumToType(type) \
    template<> struct ConstType<CONSTANT_TYPE::type> \
    { \
        using CPUType = type; \
    }; \

    #define ACT_ON_TYPE(type) MapEnumToType(type)
        TYPE_LIST
    #undef ACT_ON_TYPE

    template<typename T>
    struct RealType {};

    #define ACT_ON_TYPE(type) \
    template<> struct RealType<typename ConstType<CONSTANT_TYPE::type>::CPUType> \
    { \
        static constexpr CONSTANT_TYPE EnumType = CONSTANT_TYPE::type; \
    }; \

    TYPE_LIST
    #undef ACT_ON_TYPE

    class ConstantBufferLayout
    {
    public:
        friend class ConstantBuffer;

        template<CONSTANT_TYPE T>
        void Add(const std::string& name, std::optional<size_t> size = {})
        {
            m_Constants.insert({ name, { T, m_Size } });
            m_Size += size.has_value() ? *size : sizeof(ConstType<T>::CPUType);
        }

    private:
        std::unordered_map<std::string, std::pair<CONSTANT_TYPE, SIZE_T>> m_Constants;
        SIZE_T m_Size = 0;
    };

    constexpr CONSTANT_TYPE GetType(CONSTANT_TYPE T)
    {
        return T;
    }

    class ConstantBuffer : public Resource, public CBV
    {
    public:
        ConstantBuffer(Graphics& g, const ConstantBufferLayout& layout);

        inline BufferElement operator[](const std::string& constantName)
        {
            auto offset = m_Constants[constantName];
            CONSTANT_TYPE type = offset.first;
            return BufferElement(m_Data + offset.second);
        }

        virtual const D3D12_CONSTANT_BUFFER_VIEW_DESC& CBVDesc() const override;

        void BeginUpdate();
        void EndUpdate();

    private:
        std::unordered_map<std::string, std::pair<CONSTANT_TYPE, SIZE_T>> m_Constants;
        uint8_t* m_Data;
    };
}