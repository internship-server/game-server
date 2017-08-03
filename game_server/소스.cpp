#include "World.h"

int main()
{
	World world;
	while (true) {
		world.SetMapSize(32, 64);
		world.Init();
	}
}