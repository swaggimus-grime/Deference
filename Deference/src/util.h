#pragma once

#include <memory>
#include <wrl.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

//Rename smart ptrs to shorter name
template<typename T>
using Unique = std::unique_ptr<T>;
template<typename T, typename ... Args>
static constexpr Unique<T> MakeUnique(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Shared = std::shared_ptr<T>;
template<typename T, typename ... Args>
static constexpr Shared<T> MakeShared(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename U, typename T>
concept Derived = std::is_base_of<U, T>::value;