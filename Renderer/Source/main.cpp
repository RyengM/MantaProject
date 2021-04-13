#include "Camera.h"
#include "Renderer.h"
#include "SmokeSolver.h"
#include "ThreadManager.h"
#include <queue>

int main()
{
	Renderer renderer;

	renderer.Init();
	
	ThreadGuard phyx(std::thread(SmokeSolver::Run, std::ref(renderer.smokes["smoke"]->density), std::ref(renderer.phyxMutex), std::ref(renderer.exit)), ThreadGuard::DesAction::JOIN);

	renderer.Draw();

	return 0;
}