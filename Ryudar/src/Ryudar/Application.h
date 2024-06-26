#pragma once

#include "Core.h"

namespace Ryudar {
	class RYUDAR_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}

