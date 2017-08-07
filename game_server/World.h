#pragma once
#include <vector>
#include <random>
#include <time.h>
#include <deque>
#include <memory>

#define MIN_MAP_WIDTH 16
#define MIN_MAP_HEIGHT 16
#define MAX_ENEMY_VELOCITY 3

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
	unsigned short object_id_;
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
enum PacketType : unsigned short
{
	// To Do: 지수와 상의할 것
	SNAPSHOT
};
struct SnapshotPacketHeader
{
	unsigned short size_;
	PacketType type_;
	bool is_end_;
	unsigned short enemy_number_;
	unsigned short obstacle_number_;

};
class World
{
public:
	World() { snapshot_storage_size = 1; random_.seed(time(NULL)); };
	~World() {};
	void Init();
	void SetMapSize(unsigned short width, unsigned short height);
	
	void ProcessCommand(Command command);
	bool IsEnd() { return is_end_; }
	bool IsClear() { return is_end_ & !player_.is_dead_; }
	void SpawnEnemy();
	void SetSnapshotStorageSize(unsigned int size);
	void MakeSnapshot();
	auto GetSnapshot(unsigned int last);

private:
	bool is_end_;
	unsigned short current_object_id_;
	Position boundary_;
	Player player_;
	std::vector<Enemy> enemies_;
	std::vector<Obstacle> obstacles_;
	std::mt19937 random_;
	unsigned int snapshot_storage_size;
	std::deque<std::shared_ptr<char>> snapshots_;

	void ProcessEnemies();
	void DetectCollision();
	bool DetectClear();
	bool DetectCollisionWithBoundary();
	bool DetectCollisionWithObstacle();
	bool DetectCollisionWithEnemy();
};