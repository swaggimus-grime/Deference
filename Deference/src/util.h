#pragma once

#include <memory>
#include <vector>
#include <string>
#include <concepts>
#include <type_traits>
#include <wrl.h>
#include <DirectXMath.h>

#define ALIGN(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

static std::vector<std::string> ParseTokens(const std::string& s, const char del)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    while (ss.good()) {
        std::string sub;
        std::getline(ss, sub, del);
        tokens.push_back(std::move(sub));
    }

    return tokens;
}

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

template<typename U, typename T>
concept SameClass = std::is_same<U, T>::value;

template<typename T>
concept IsEnum = std::is_enum_v<T>;