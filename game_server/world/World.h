#pragma once
#include <vector>
#include <random>
#include <time.h>
#include <deque>
#include <queue>
#include <memory>

#define MIN_MAP_WIDTH 8
#define MIN_MAP_HEIGHT 8
#define MAX_ENEMY_VELOCITY 3
#define MAX_ENEMY_COUNT 300
#define EXPECTED_ERROR 0.05
#define OBJECT_RADIUS 0.4

#pragma pack(push, 1)
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
struct SnapshotHeader
{
	unsigned short total_size_;
	bool is_end_;
	unsigned short enemy_number_;
	unsigned short obstacle_number_;
	unsigned short current_score_;
	unsigned short highest_score_;
};
struct Snapshot
{
	SnapshotHeader header_;
	std::shared_ptr<std::vector<char>> data_;
};
#pragma pack(pop)
bool IsCollision(Object &a, Object &b);
class World
{
public:
	World() : ips_(1), snapshot_storage_size_(1), highest_score_(0) { random_.seed(time(NULL)); };
	~World() {};
	void Init();
	void SetMapSize(unsigned short width, unsigned short height);
	void SetIps(unsigned short interval);
	void SetSnapshotStorageSize(unsigned int size);

	void ProcessCommand(Command command);
	bool IsEnd() { return is_end_; }
	bool IsClear() { return is_end_ & !player_.is_dead_; }
	void SpawnEnemy();
	void MakeSnapshot();
    Snapshot& GetSnapshot(unsigned int last);
	void Print();

private:
	bool is_end_;
	float ips_;
	Position boundary_;
	Player player_;
	std::vector<Enemy> enemies_;
	std::vector<Obstacle> obstacles_;
	std::mt19937 random_;
	unsigned int snapshot_storage_size_;
	std::deque<Snapshot> snapshots_;
	std::queue<unsigned short> enemy_id_queue_;
	unsigned short current_score_;
	unsigned short highest_score_;

	void ProcessEnemies();
	void DetectCollision();
	bool DetectClear();
	bool DetectCollisionWithBoundary();
	bool DetectCollisionWithObstacle();
	bool DetectCollisionWithEnemy();
	void UpdateScore();
};