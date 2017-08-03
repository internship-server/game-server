#pragma once
#include <vector>

struct Object
{
	struct pos
	{
		float x;
		float y;
	};
};

struct Player : Object
{};

struct Obstacle : Object
{};

enum Direction : unsigned short
{
	UP, DOWN, LEFT, RIGHT
};
struct Enemy : Object
{
	Direction direction_;
	unsigned short velocity_;
};

class World
{
private:
	Player player;
	std::vector<Object> objects;
};