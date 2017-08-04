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
	friend bool operator==(const Object& l, const Object& r)
	{
		return l.pos.x == r.pos.x && l.pos.y == r.pos.y;
	}
};

struct Player : public Object
{
	bool is_dead_;
};

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
	
	void ProcessCommand(Command command);

private:
	bool is_cleared_;
	Position boundary_;
	Player player_;
	std::vector<Enemy> enemies_;
	std::vector<Obstacle> obstacles_;
	std::mt19937 random_;

	void ProcessEnemies();
	void DetectCollision();
	bool DetectClear();
	bool DetectCollisionWithBoundary();
	bool DetectCollisionWithObstacle();
	bool DetectCollisionWithEnemy();
};