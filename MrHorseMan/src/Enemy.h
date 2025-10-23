#pragma once

#include "Entity.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "Animation.h"

struct SDL_Texture;

class Enemy : public Entity
{

public:

	Enemy();
	virtual ~Enemy();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);
	//void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void ResetToSpawn();

public:
	SDL_Texture* texture = NULL;
	float speed = 2.0f;

	int texW, texH;

	int health = 50;
	int maxHealth = 50;
	Vector2D spawnPos = { 96,96 };



private:

	b2Vec2 velocity;

	PhysBody* pbody = nullptr;
	Vector2D spawn;
	float patrolLeft = 0.0f;
	float patrolRight = 0.0f;
	int direction = 1; //1 right, -1 left

	AnimationSet anims;
	SDL_FlipMode flip = SDL_FLIP_NONE;

};