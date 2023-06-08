#pragma once

//C++ stuff
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <memory>
#include <array>
#include <numbers>
static constexpr auto PI = std::numbers::pi;
#include <optional>

//Windows defines
//#define _USE_MATH_DEFINES
#define NOMINMAX

//Windows stuff
#include <Windows.h>
#include <dxcapi.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <comdef.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
using namespace DirectX;

//App common headers
#include "Debug/Exception.h"
#include "Graphics/Graphics.h"

//Rename smart ptrs to shorter name
template<typename T>
using Unique = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Unique<T> MakeUnique(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Shared = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Shared<T> MakeShared(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}