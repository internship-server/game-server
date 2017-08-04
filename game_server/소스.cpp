#include "World.h"
#include <chrono>
#include <thread>

int main()
{
	using namespace std::chrono_literals;

	World world;
	world.SetMapSize(32, 64);
	world.Init();
	while (true) {
		std::this_thread::sleep_for(100ms);
		unsigned int command = rand() % 5;
		world.ProcessCommand(static_cast<Command>(command));
	}
}