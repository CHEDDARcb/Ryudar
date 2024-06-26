#include <Ryudar.h>

class Sandbox : public Ryudar::Application
{
public:
	Sandbox()
	{
	}

	~Sandbox()
	{
	}
};

Ryudar::Application* Ryudar::CreateApplication()
{
	return new Sandbox();
}