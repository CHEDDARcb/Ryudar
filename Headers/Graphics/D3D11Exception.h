#pragma once

// Direct3D 호출 실패 시 HRESULT와 작업 정보를 함께 전달하는 예외 형식이다.
// ThrowIfFailed()를 통해 실패 검사를 한곳에서 일관되게 처리한다.

#include <stdexcept>
#include <string>
#include <string_view>

#include <windows.h>

namespace Ryudar
{

class D3D11Exception : public std::runtime_error
{
public:
	D3D11Exception(HRESULT result, std::string_view operation, std::string_view detail = {});

	// 원본 HRESULT를 반환해 호출자가 오류 종류를 추가로 판별할 수 있게 한다.
	HRESULT GetResult() const noexcept;

private:
	HRESULT m_result;
};

// 실패한 HRESULT를 D3D11Exception으로 변환하고, 성공한 경우에는 아무 작업도 하지 않는다.
void ThrowIfFailed(HRESULT result, std::string_view operation, std::string_view detail = {});

} // namespace Ryudar
