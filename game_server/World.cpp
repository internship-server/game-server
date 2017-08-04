#include "World.h"
#include <iostream>

void World::Init()
{
	std::cout << std::boolalpha;
	is_end_ = false;
	player_.is_dead_ = false;
	player_.pos.x = random_() % static_cast<unsigned int>(boundary_.x);
	player_.pos.y = 0;
	enemies_.clear();
	obstacles_.clear();
}

void World::SetMapSize(unsigned short width, unsigned short height)
{
	if (width < MIN_MAP_WIDTH || height < MIN_MAP_HEIGHT)
		throw "Map Size is Too Small.";
	boundary_.x = width;
	boundary_.y = height;
}

void World::ProcessCommand(Command command)
{
	ProcessEnemies();
	Position before_position = player_.pos;
	switch (command) {
	case Command::NONE:
		break;
	case Command::UP:
		++player_.pos.y;
		break;
	case Command::DOWN:
		--player_.pos.y;
		break;
	case Command::LEFT:
		--player_.pos.x;
		break;
	case Command::RIGHT:
		++player_.pos.x;
		break;
	default:
		throw "Unknown Command.";
	}
	if (DetectClear()) {
		std::cout << "Clear!\n";
		is_end_ = true;
	}
	else {
		if (DetectCollisionWithObstacle() || DetectCollisionWithBoundary())
			player_.pos = before_position;
		if (DetectCollisionWithEnemy())
			player_.is_dead_ = is_end_ = true;
	}
	std::cout << "Alive: " << !player_.is_dead_ << "\tPosition: (" << player_.pos.x << ", " << player_.pos.y << ")\n";
	std::cout << "Enemies: ";
	for (Enemy &enemy : enemies_) {
		std::cout << "(" << enemy.pos.x << ", " << enemy.pos.y << "), ";
	}	std::cout << std::endl;
}

void World::SpawnEnemy()
{
	if (!(random_() % 2)) {
		std::cout << "Enemy Spawned!\n";
		Enemy new_enemy;
		new_enemy.velocity_ = random_() % (MAX_ENEMY_VELOCITY - 1);
		++new_enemy.velocity_;
		new_enemy.direction_ = static_cast<Direction>(random_() % 2);
		new_enemy.pos.x = new_enemy.direction_ == Direction::RIGHT ? 0 : boundary_.x;
		new_enemy.pos.y = random_() % static_cast<unsigned int>(boundary_.y);
		enemies_.push_back(new_enemy);
	}
}

void World::SetSnapshotStorageSize(unsigned int size)
{
	snapshot_storage_size = size;
}

void World::MakeSnapshot()
{
	unsigned int player_area_size, enemies_area_size, obstacles_area_size;
	player_area_size = sizeof(Player);
	enemies_area_size = sizeof(Enemy) * enemies_.size();
	obstacles_area_size = sizeof(Obstacle) * obstacles_.size();
	unsigned int total_size = player_area_size + enemies_area_size + obstacles_area_size;
	std::cout << "Total size is " << total_size << "\n";
	std::shared_ptr<char> new_snapshot(new char[total_size]);
	memset(new_snapshot.get(), 0, total_size);
	memcpy(new_snapshot.get(), &player_, sizeof(Player));
	memcpy(new_snapshot.get() + player_area_size, enemies_.data(), enemies_area_size);
	memcpy(new_snapshot.get() + player_area_size + enemies_area_size, obstacles_.data(), obstacles_area_size);

	snapshots_.push_front(new_snapshot);
	if (snapshots_.size() > snapshot_storage_size)
		snapshots_.pop_back();
}

auto World::GetSnapshot(unsigned int last)
{
	if (last >= snapshots_.size())
		throw "Invalid Snapshot Index.";
	return snapshots_[last];
}

void World::ProcessEnemies()
{
	for (auto enemy = enemies_.begin(); enemy != enemies_.end();) {
		if (enemy->direction_ == Direction::LEFT)
			enemy->pos.x -= enemy->velocity_;
		else if (enemy->direction_ == Direction::RIGHT)
			enemy->pos.x += enemy->velocity_;
		else
			throw "Unknown Direction.";

		if ((enemy->direction_ == Direction::LEFT && enemy->pos.x <= 0) ||
			(enemy->direction_ == Direction::RIGHT && enemy->pos.x >= boundary_.x))
			enemy = enemies_.erase(enemy);
		else
			++enemy;
	}
}

void World::DetectCollision()
{
	DetectCollisionWithObstacle();
	DetectCollisionWithEnemy();
}

bool World::DetectClear()
{
	return player_.pos.y == boundary_.y;
}

bool World::DetectCollisionWithBoundary()
{
	return player_.pos.x < 0 || player_.pos.x > boundary_.x || player_.pos.y < 0 || player_.pos.y > boundary_.y;
}

bool World::DetectCollisionWithObstacle()
{
	for (Obstacle &obstacle : obstacles_) {
		if (player_ == obstacle) {
			return true;
		}
	}
	return false;
}

bool World::DetectCollisionWithEnemy()
{
	for (Enemy &enemy : enemies_) {
		if (player_ == enemy) {
			return true;
		}
	}
	return false;
}
