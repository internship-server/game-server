#include "World.h"
#include <iostream>
#include <math.h>

void World::Init()
{
	current_score_ = 0;
	for (int i = 0; i <= MAX_ENEMY_COUNT; ++i)
		enemy_id_queue_.push(i);
	is_end_ = false;
	player_.object_id_ = 0;
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

void World::SetIps(unsigned short ips)
{
	ips_ = ips;
}

void World::ProcessCommand(Command command)
{
	ProcessEnemies();
	Position before_position = player_.pos;
	switch (command) {
	case Command::NONE:
		break;
	case Command::UP:
		player_.pos.y += 1 / ips_;
		break;
	case Command::DOWN:
		player_.pos.y -= 1 / ips_;
		break;
	case Command::LEFT:
		player_.pos.x -= 1 / ips_;
		break;
	case Command::RIGHT:
		player_.pos.x += 1 / ips_;
		break;
	default:
		throw "Unknown Command.";
	}
	if (DetectClear()) {
		is_end_ = true;
	}
	else {
		if (DetectCollisionWithObstacle() || DetectCollisionWithBoundary())
			player_.pos = before_position;
		if (DetectCollisionWithEnemy())
			player_.is_dead_ = is_end_ = true;
	}
	UpdateScore();
}

void World::SpawnEnemy()
{
	if (enemies_.size() < MAX_ENEMY_COUNT) {
		if (!(random_() % 2)) {
			Enemy new_enemy;
			new_enemy.velocity_ = random_() % (MAX_ENEMY_VELOCITY - 1);
			++new_enemy.velocity_;
			new_enemy.direction_ = static_cast<Direction>(random_() % 2);
			new_enemy.pos.x = new_enemy.direction_ == Direction::RIGHT ? 0 : boundary_.x;
			new_enemy.pos.y = random_() % static_cast<unsigned int>(boundary_.y);
			new_enemy.object_id_ = enemy_id_queue_.front();
			enemy_id_queue_.pop();
			enemies_.push_back(new_enemy);
		}
	}
}

void World::SetSnapshotStorageSize(unsigned int size)
{
	snapshot_storage_size_ = size;
}

void World::MakeSnapshot()
{
	Snapshot snapshot;
	SnapshotHeader &header = snapshot.header_;
	header.is_end_ = is_end_;
	header.enemy_number_ = enemies_.size();
	header.obstacle_number_ = obstacles_.size();
	header.current_score_ = current_score_;
	header.highest_score_ = highest_score_;
	
	unsigned int player_area_size, enemies_area_size, obstacles_area_size;
	player_area_size = sizeof(Player);
	enemies_area_size = sizeof(Enemy) * header.enemy_number_;
	obstacles_area_size = sizeof(Obstacle) * header.obstacle_number_;
	unsigned int total_size = player_area_size + enemies_area_size + obstacles_area_size;
	header.total_size_ = total_size;
	snapshot.data_ = std::make_shared<std::vector<char>>(total_size);
	auto snapshot_data = snapshot.data_.get()->data();
	memset(snapshot_data, 0, total_size);
	memcpy(snapshot_data, &player_, sizeof(Player));
	memcpy(snapshot_data + player_area_size, enemies_.data(), enemies_area_size);
	memcpy(snapshot_data + player_area_size + enemies_area_size, obstacles_.data(), obstacles_area_size);

	snapshots_.push_front(snapshot);
	if (snapshots_.size() > snapshot_storage_size_)
		snapshots_.pop_back();
}

Snapshot& World::GetSnapshot(unsigned int last)
{
	if (last >= snapshots_.size())
		throw "Invalid Snapshot Index.";
	return snapshots_[last];
}

void World::Print()
{
	std::cout << "Alive: " << !player_.is_dead_ << "\tPosition: (" << player_.pos.x << ", " << player_.pos.y << ")\n";
	std::cout << "Enemies: ";
	for (Enemy &enemy : enemies_) {
		std::cout << "(" << enemy.pos.x << ", " << enemy.pos.y << "), ";
	}    std::cout << std::endl;
}

void World::ProcessEnemies()
{
	for (auto enemy = enemies_.begin(); enemy != enemies_.end();) {
		if (enemy->direction_ == Direction::LEFT)
			enemy->pos.x -= (enemy->velocity_ / (float)ips_);
		else if (enemy->direction_ == Direction::RIGHT)
			enemy->pos.x += (enemy->velocity_ / (float)ips_);
		else
			throw "Unknown Direction.";

		if ((enemy->direction_ == Direction::LEFT && enemy->pos.x <= 0) ||
			(enemy->direction_ == Direction::RIGHT && enemy->pos.x >= boundary_.x)) {
			enemy_id_queue_.push(enemy->object_id_);
			enemy = enemies_.erase(enemy);
		}
		else {
			++enemy;
		}
	}
}

void World::DetectCollision()
{
	DetectCollisionWithObstacle();
	DetectCollisionWithEnemy();
}

bool World::DetectClear()
{
	return player_.pos.y - EXPECTED_ERROR <= boundary_.y && boundary_.y <= player_.pos.y + EXPECTED_ERROR;
}

bool World::DetectCollisionWithBoundary()
{
	return player_.pos.x < 0 || player_.pos.x > boundary_.x || player_.pos.y < 0 || player_.pos.y > boundary_.y;
}

bool World::DetectCollisionWithObstacle()
{
	for (Obstacle &obstacle : obstacles_)
		if (IsCollision(player_, obstacle))
			return true;
	return false;
}

bool World::DetectCollisionWithEnemy()
{
	for (Enemy &enemy : enemies_)
		if (IsCollision(player_, enemy))
			return true;
	return false;
}

void World::UpdateScore()
{
	if (player_.pos.y > current_score_) {
		current_score_ = player_.pos.y;
		if (current_score_ > highest_score_)
			highest_score_ = current_score_;
	}
}

bool IsCollision(Object & a, Object & b)
{
	if (abs(a.pos.x - b.pos.x) <= 2 * OBJECT_RADIUS && abs(a.pos.y - b.pos.y) <= 2 * OBJECT_RADIUS)
		return true;
	return false;
}
