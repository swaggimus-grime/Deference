#pragma once

#include "Resource.h"
#include "Bindable/Bindable.h"
#include <d3d12.h>
#include "Graphics.h"
#include <variant>

#define TYPE_LIST \
ACT_ON_TYPE(FLOAT) \
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
    void Add(const std::string& name)
    {
        m_Constants.insert({ name, { T, m_Size } });
        m_Size += sizeof(ConstType<T>::CPUType);
    }

private:
    std::unordered_map<std::string, std::pair<CONSTANT_TYPE, SIZE_T>> m_Constants;
    SIZE_T m_Size = 0;
};

constexpr CONSTANT_TYPE GetType(CONSTANT_TYPE T)
{
    return T;
}

class ConstantBufferElement
{
public:
    ConstantBufferElement(uint8_t* ptr)
        :m_Ptr(ptr)
    {}

    template<typename T>
    operator T&() const
    {
        return *reinterpret_cast<T*>(m_Ptr);
    }

    template<typename T>
    operator T*() const
    {
        return reinterpret_cast<T*>(m_Ptr);
    }

    template<typename T>
    void operator=(T* v) const
    {
        static_cast<T*>(m_Ptr) = v;
    }

    template<typename T>
    void operator=(const T& v) const
    {
        //static_assert<RealType<std::remove_const_t<T>>
        static_cast<T&>(*this) = v;
    }

private:
    uint8_t* m_Ptr;
};

class ConstantBuffer : public Resource
{
public:
    ConstantBuffer(Graphics& g, const ConstantBufferLayout& layout);

    inline ConstantBufferElement operator[](const std::string& constantName)
    {
        auto offset = m_Constants[constantName];
        CONSTANT_TYPE type = offset.first;
        return ConstantBufferElement(m_Data + offset.second);
    }

    virtual void CreateView(Graphics& g, HDESC h) override;

    inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_Res->GetGPUVirtualAddress(); }

    void BeginUpdate();
    void EndUpdate();

private:
    std::unordered_map<std::string, std::pair<CONSTANT_TYPE, SIZE_T>> m_Constants;
    uint8_t* m_Data;
};