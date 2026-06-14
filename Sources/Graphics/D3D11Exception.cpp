#include "Graphics/D3D11Exception.h"

#include <cstdint>
#include <iomanip>
#include <sstream>

namespace Ryudar
{
namespace
{

std::string MakeErrorMessage(HRESULT result, std::string_view operation, std::string_view detail)
{
	std::ostringstream stream;

	stream << operation << " failed. HRESULT: 0x" << std::hex << std::uppercase << std::setw(8)
	       << std::setfill('0') << static_cast<std::uint32_t>(result);

	if (!detail.empty())
	{
		stream << '\n' << detail;
	}

	return stream.str();
}

} // namespace

D3D11Exception::D3D11Exception(HRESULT result, std::string_view operation, std::string_view detail)
    : std::runtime_error(MakeErrorMessage(result, operation, detail))
    , m_result(result)
{
}

HRESULT D3D11Exception::GetResult() const noexcept { return m_result; }

void ThrowIfFailed(HRESULT result, std::string_view operation, std::string_view detail)
{
	if (FAILED(result))
	{
		throw D3D11Exception(result, operation, detail);
	}
}

} // namespace Ryudar
