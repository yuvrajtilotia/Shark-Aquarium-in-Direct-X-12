#include "pch.h"
#include "Engine.h"

int CALLBACK wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	INIT_MEMORY_LEAK_DETECTION;

	Engine engine;
	engine.Initialize(APP_NAME);
	engine.Run();
	return 0;
}