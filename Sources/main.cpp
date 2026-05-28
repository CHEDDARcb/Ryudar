#include <iostream>
#include <memory>
#include <windows.h>

#include "Ryudar.h"

using namespace std;

int main(void)
{
	Ryudar::Ryudar ryudarApp;

	if (!ryudarApp.Initialize())
	{
		cout << "Initialization failed." << endl;
		return -1;
	}

	return ryudarApp.Run();
}
