#pragma once

#ifdef RD_PLATFORM_WINDOWS
	#ifdef RD_BUILD_DLL
		#define RYUDAR_API __declspec(dllexport)
	#else
		#define RYUDAR_API __declspec(dllimport)
	#endif
#else
	#error Ryudar only supports windows!
#endif