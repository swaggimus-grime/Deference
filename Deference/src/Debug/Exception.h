#pragma once

#include <Windows.h>
#include <source_location>

class DefException : public std::runtime_error
{
public:
	DefException(const std::string& msg, std::source_location loc = std::source_location::current());
};

struct HRWrapper
{
	HRWrapper(HRESULT hr, std::source_location loc = std::source_location::current());

	HRESULT m_HR;
	std::source_location m_Loc;
};

struct AssertWrapper
{
	AssertWrapper(bool assert, std::source_location loc = std::source_location::current());

	bool m_Assertion;
	std::source_location m_Loc;
};

static struct AssertChecker {} ASS;

void operator <<(AssertChecker, HRWrapper hr);
void operator >>(AssertChecker, AssertWrapper assert);

#define HR ASS << 
#define BR ASS >> 