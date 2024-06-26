#pragma once

#ifdef RD_PLATFORM_WINDOWS

extern Ryudar::Application* Ryudar::CreateApplication();

int main(int argc, char** argv)
{
	printf("Ryudar Engine\n");
	auto app = Ryudar::CreateApplication();
	app->Run();
	delete app;
}
#endif
