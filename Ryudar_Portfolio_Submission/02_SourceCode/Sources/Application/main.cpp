#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <windows.h>

#include "Graphics/D3D11Exception.h"
#include "Application/Ryudar.h"

using namespace std;

int main(void)
{
	try
	{
		Ryudar::Ryudar ryudarApp;

		if (!ryudarApp.Initialize())
		{
			std::cerr << "Initialization failed.\n";
			return EXIT_FAILURE;
		}

		return ryudarApp.Run();
	}
	catch (const Ryudar::D3D11Exception &exception)
	{
		std::cerr << "Direct3D error:\n" << exception.what() << '\n';

		return EXIT_FAILURE;
	}
	catch (const std::exception &exception)
	{
		std::cerr << "Fatal error:\n" << exception.what() << '\n';

		return EXIT_FAILURE;
	}
}
