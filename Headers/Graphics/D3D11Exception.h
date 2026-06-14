#pragma once

// Direct3D呼び出し失敗時にHRESULTと処理内容をまとめて伝える例外型。
// ThrowIfFailed()により失敗判定を一か所で統一する。

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

	// 元のHRESULTを返し、呼び出し側でエラー種別を追加判定できるようにする。
	HRESULT GetResult() const noexcept;

private:
	HRESULT m_result;
};

// 失敗したHRESULTをD3D11Exceptionへ変換し、成功時は何もしない。
void ThrowIfFailed(HRESULT result, std::string_view operation, std::string_view detail = {});

} // namespace Ryudar
