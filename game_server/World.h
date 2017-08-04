#pragma once
#include <vector>
#include <random>

#define MIN_MAP_WIDTH 16
#define MIN_MAP_HEIGHT 32

enum class Command : unsigned short
{
	NONE, UP, DOWN, LEFT, RIGHT
};

struct Position
{
	float x;
	float y;
};

struct Object
{
	Position pos;
};

struct Player : public Object
{};

struct Obstacle : public Object
{};

enum class Direction : unsigned short
{
	LEFT, RIGHT
};
struct Enemy : public Object
{
	Direction direction_;
	unsigned short velocity_;
};

class World
{
public:
	World() { random_.seed(std::mt19937::default_seed); };
	~World() {};
	void Init();
	void SetMapSize(unsigned short width, unsigned short height);

private:
	Position boundary_;
	Player player_;
	std::vector<Object> objects_;
	std::mt19937 random_;
};