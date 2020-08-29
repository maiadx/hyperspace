// HyperspaceEngine.cpp : Defines the functions for the static library.
//

#include "hypch.h"
#include "Core/Application.h"

int main()
{
	hyApplication app;

	app.Init();
	app.Run();
	app.Shutdown();

	return 0;
}