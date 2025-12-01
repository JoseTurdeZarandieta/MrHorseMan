#pragma once

#include "Entity.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

enum enemyState {
	PATROL,
	CHASE
};

class Enemy : public Entity
{

public:

	Enemy();
	virtual ~Enemy();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();
	Vector2D GetPosition();
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void ResetToSpawn();
	std::shared_ptr<Pathfinding> pathfinding;

private:
	void PerformPathFinding();

public:
	SDL_Texture* texture = NULL;
	float speed = 2.0f;

	int texW, texH;

	int health = 50;
	int maxHealth = 50;
	Vector2D spawnPos = { 96,96 };

	bool playerOnRange;
	int detectRange = 6;
	enemyState state = PATROL;

private:

	b2Vec2 velocity;

	PhysBody* pbody = nullptr;
	Vector2D spawn;

	/*TODO: enemy patrols*/
	float patrolLeft = 0.0f;
	float patrolRight = 0.0f;
	int direction = 1; //1 right, -1 left

	AnimationSet anims;
	SDL_FlipMode flip = SDL_FLIP_NONE;

};