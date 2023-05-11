#include "pch.h"
#include "Exception.h"
#include <exception>
#include <format>

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
		throw std::runtime_error(std::format("{}:{}:{}: {}",
			hr.m_Loc.file_name(),
			hr.m_Loc.line(),
			hr.m_Loc.column(),
			err.ErrorMessage()));
	}
}

void operator >>(AssertChecker, AssertWrapper assert)
{
	if (!assert.m_Assertion)
		throw std::runtime_error(std::format("{}:{}:{}: {}",
			assert.m_Loc.file_name(),
			assert.m_Loc.line(),
			assert.m_Loc.column(),
			"Assertion failed!"));
}
