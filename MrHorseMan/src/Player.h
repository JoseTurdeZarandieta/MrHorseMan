#pragma once

#include "Entity.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "Animation.h"

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void TakeDamage(int amount);
	void HealToFull();
	void Respawn();

	int GetHealth() const {
		return health;
	}

public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;

	int texW, texH;

	//Audio fx
	int pickCoinFxId;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 2.0f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping

	bool pendingRespawn = false;

	int health = 100;
	int maxHealth = 100;
	Vector2D spawnPos = { 96,96 };
	float fallSpeedDamageThreshold = 10.0f;
	float fallSpeedMax = 25.0f;

private: 
	b2Vec2 velocity;
	// L10: TODO 4: Declare an AnimationSet to hold all player animations
	AnimationSet anims;
	SDL_FlipMode flip = SDL_FLIP_NONE;

	float maxDownwardSpeed = 0.0f;
	bool isGrounded = false;
	bool godMode = false;
	bool dashed = false;
	int jumpCount = 0;
	int isRight = 1;
	float currentTime = 0.0f;
	float maxTime = 1.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	float dashDuration = 0.5f; // segundos
	float dashTimer = 0.0f;
	float dashSpeed = 100.0f;   // velocidad horizontal
	int dashDirection = 1;      // 1 = derecha, -1 = izquierda
	

	const int maxJumps = 2; // 1 ground + 1 air (double jump)
};