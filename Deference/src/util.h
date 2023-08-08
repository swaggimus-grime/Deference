#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <concepts>
#include <type_traits>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxcapi.h>
#include <wrl.h>

using HCPU = D3D12_CPU_DESCRIPTOR_HANDLE;
using HGPU = D3D12_GPU_DESCRIPTOR_HANDLE;

struct HDESC
{
	HCPU m_HCPU;
	HGPU m_HGPU;
};

#define ALIGN(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

using Microsoft::WRL::ComPtr;
using namespace DirectX;

template<typename T>
static auto ToVector(std::vector<T> v, std::function<void(UINT)> f)
{
	return std::views::iota(0u, (UINT)v.size()) |
		std::views::transform(f) |
		std::ranges::to<std::vector>();
}

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

template<typename U, typename T>
concept SameClass = std::is_same<U, T>::value;

template<typename T>
concept IsEnum = std::is_enum_v<T>;