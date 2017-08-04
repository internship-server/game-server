#include "World.h"
#include <chrono>
#include <thread>

int main()
{
	using namespace std::chrono_literals;

	World world;
	world.SetMapSize(64, 8);
	world.Init();
	while (!world.IsEnd()) {
		std::this_thread::sleep_for(10ms);
		unsigned int command = rand() % 5;
		world.SpawnEnemy();
		world.ProcessCommand(static_cast<Command>(command));
	}
}