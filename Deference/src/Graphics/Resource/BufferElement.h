#pragma once

namespace Def
{
    class BufferElement
    {
    public:
        BufferElement(void* ptr)
            :m_Ptr(ptr)
        {}

        template<typename T>
        operator T& () const
        {
            return *reinterpret_cast<T*>(m_Ptr);
        }

        template<typename T>
        operator T* () const
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
        void* m_Ptr;
    };
}