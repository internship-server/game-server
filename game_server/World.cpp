#include "World.h"
#include <iostream>

void World::Init()
{
	is_cleared_ = false;
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
		is_cleared_ = true;
	}
	else {
		if (DetectCollisionWithObstacle() || DetectCollisionWithBoundary())
			player_.pos = before_position;
		if (DetectCollisionWithEnemy())
			player_.is_dead_ = true;
	}
	std::cout << "Alive: " << !player_.is_dead_ << "Position: (" << player_.pos.x << ", " << player_.pos.y << ")\n";
}

void World::ProcessEnemies()
{
	for (Enemy &enemy : enemies_) {
		if (enemy.direction_ == Direction::LEFT)
			enemy.pos.x -= enemy.velocity_;
		else if (enemy.direction_ == Direction::RIGHT)
			enemy.pos.x += enemy.velocity_;
		else
			throw "Unknown Direction.";
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
