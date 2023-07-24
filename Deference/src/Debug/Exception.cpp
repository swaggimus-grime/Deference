#include "Exception.h"
#include <exception>
#include <format>
#include <comdef.h>

HRWrapper::HRWrapper(HRESULT hr, std::source_location loc)
	:m_HR(hr), m_Loc(loc)
{
}

AssertWrapper::AssertWrapper(bool assert, std::source_location loc)
	:m_Assertion(assert), m_Loc(loc)
{
}

void operator <<(AssertChecker, HRWrapper hr)
{
	if (FAILED(hr.m_HR)) {
		_com_error err(hr.m_HR);
		throw DefException(err.ErrorMessage(), hr.m_Loc);
	}
}

void operator >>(AssertChecker, AssertWrapper assert)
{
	if (!assert.m_Assertion)
		throw DefException("Assertion failed!", assert.m_Loc);
}

DefException::DefException(const std::string& msg, std::source_location loc)
	:std::runtime_error(std::format("{}:{}:{}: {}",
		loc.file_name(),
		loc.line(),
		loc.column(),
		msg))
{
}
