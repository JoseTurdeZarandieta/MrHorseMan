#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"
#include "Item.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	//position = Vector2D(96, 96);
	spawnPos = position;
	health = maxHealth;
	return true;
}

bool Player::Start() {

	spawnPos = position;

	texture = Engine::GetInstance().textures->Load("Assets/Textures/MrHorseMan_spritesheet.png");

	std::unordered_map<int, std::string> animNames = { {0, "idle"}, {6, "move"}, {12, "jump"} };
	anims.LoadFromTSX("Assets/Textures/MrHorseMan_spritesheet.tsx", animNames);
	anims.SetCurrent("idle");

	texW = 32;
	texH = 32;
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texH / 2, bodyType::DYNAMIC);

	pbody->listener = this;

	pbody->ctype = ColliderType::PLAYER;

	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/coin-collision-sound-342335.wav");

	return true;
}

bool Player::Update(float dt)
{
	Physics* physics = Engine::GetInstance().physics.get();

	if (pendingRespawn) {
		pendingRespawn = false;
		Respawn();
	}



	// Read current velocity
	b2Vec2 velocity = physics->GetLinearVelocity(pbody);
	if(!dashed)velocity = { 0, velocity.y }; // Reset horizontal velocity
	bool moving = false;

	//GodMode
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		LOG("GodMode_Switched");
		if (godMode == false) godMode = true;
		if (godMode == true) godMode = false;
	}

	// Move left/right
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -speed;
		anims.SetCurrent("move");
		flip = SDL_FLIP_HORIZONTAL; //flips the player's character when moving left
		moving = true;
		isRight = -1;
	}
	else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = speed;
		anims.SetCurrent("move");
		flip = SDL_FLIP_NONE;
		moving = true;
		isRight = 1;
	}

	// Move up/down
	if (godMode == true) {

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			velocity.y = -speed;
			moving = true;
		}

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			velocity.y = +speed;
			moving = true;
		}
	}

	//Dash															
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN && dashed == false && (isJumping == true || isGrounded == true)) {
		LOG("funciona");
		dashed == true;
		currentTime = 0.0f;

		b2Body_SetGravityScale(pbody->body, 0.0f); //desactiva gravedad
		Engine::GetInstance().physics->SetLinearVelocity(pbody, {100.0f * isRight, 0.0f});

		
	}

	if (dashed == true) {
		currentTime += deltaTime; // vas contando

		if (currentTime >= maxTime) {
			dashed = false;

			b2Body_SetGravityScale(pbody->body, 1.0f); //activas gravedad
		}
	}

	// Jump (impulse once)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && jumpCount < maxJumps) {
		b2Vec2 vel = physics->GetLinearVelocity(pbody); 
		vel.y = 0;
		physics->SetLinearVelocity(pbody, vel); 
		physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		//L10: TODO 6: Update the animation based on the player's state
		anims.SetCurrent("jump");
		isJumping = true;
		isGrounded = false;
		jumpCount++;
	}
	if (!isJumping && !moving)
		anims.SetCurrent("idle");


// Preserve vertical speed while jumping
if (isJumping == true) {
	velocity.y = physics->GetYVelocity(pbody);

	if (velocity.y > maxDownwardSpeed) {
		maxDownwardSpeed = velocity.y;
	}
}

// Apply velocity via helper
physics->SetLinearVelocity(pbody, velocity);

if (health <= 0) {
	Respawn();
}
// L10: TODO 5: Update the animation based on the player's state (moving, jumping, idle)
anims.Update(dt);
SDL_Rect animFrame = anims.GetCurrentFrame();

// Update render position using your PhysBody helper
int x, y;
pbody->GetPosition(x, y);
position.setX((float)x);
position.setY((float)y);

//L10: TODO 7: Center the camera on the player
float limitLeft = Engine::GetInstance().render->camera.w / 4;
float limitRight = Engine::GetInstance().map->GetMapSizeInPixels().getX() - Engine::GetInstance().render->camera.w * 3 / 4;;
if (position.getX() - limitLeft > 0 && position.getX() < limitRight) {
	Engine::GetInstance().render->camera.x = -position.getX() + Engine::GetInstance().render->camera.w / 4;

}

float limitUp = Engine::GetInstance().render->camera.h / 4;
float limitDown = Engine::GetInstance().map->GetMapSizeInPixels().getY() - Engine::GetInstance().render->camera.h * 3 / 4;
if (position.getY() - limitUp > 0 && position.getY() < limitDown) {
	Engine::GetInstance().render->camera.y = -position.getY() + Engine::GetInstance().render->camera.h / 4;

}

// L10: TODO 5: Draw the player using the texture and the current animation frame
Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - 1.5* texH, &animFrame, 1.0f, 0.0, INT_MAX, INT_MAX, flip);

//health screen display TODO NEXT TIME. THIS TIME, NO UI REQUIRED
char hpText[32];
snprintf(hpText, sizeof(hpText), "HP: %d", health);

int screenW = Engine::GetInstance().render->camera.w;
int margin = 12;
int posTextX = -Engine::GetInstance().render->camera.w + (screenW - 100);
int posTextY = Engine::GetInstance().render->camera.y + margin;

Engine::GetInstance().render->DrawText(hpText, posTextX, posTextY);


return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	
	switch (physB->ctype)
	{	
	case ColliderType::PLATFORM:
	{
		int px, py;
		int bx, by;
		pbody->GetPosition(px, py);
		physB->GetPosition(bx, by);

		// Compute vertical and horizontal distance
		float dy = py - by;
		float dx = abs(px - bx);

		// Only count as landing if the platform is below the player and horizontally aligned
		if (dy < -texH / 2 && dx < texW) // platform below within tolerance
		{
			jumpCount = 0;
			anims.SetCurrent("idle");
		}
		LOG("Collision PLATFORM");
		if (isJumping) {
			int dmg = 0;
			if (maxDownwardSpeed > fallSpeedDamageThreshold) {
				float t = (std::min(maxDownwardSpeed, fallSpeedMax) - fallSpeedDamageThreshold) / std::max(0.07f, (fallSpeedMax - fallSpeedDamageThreshold));
				dmg = (int)(t * 120.0f);
				TakeDamage(dmg);
			}
			LOG("jump x platform. dmg %d\Current Health %d", dmg, health);
			maxDownwardSpeed = 0.0f;
		}
		//reset the jump flag when touching the ground
		isJumping = false;
		isGrounded = true;
		//L10: TODO 6: Update the animation based on the player's state
		LOG("Collision PLATFORM");
		break;
	}
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);

		if (physB && physB->listener) {

			if (auto* item = dynamic_cast<Item*>(physB->listener)) {
				item->isPicked = true;
			}

			physB->listener->Destroy();
		}
		break;
	case ColliderType::ENEMY:
		TakeDamage(10);
		LOG("Collision Enemy. Health %d", health);
		break;
	case ColliderType::DEATHZONE:
		LOG("DeathZone hit. Respawning");
		pendingRespawn = true;
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::ENEMY:
		LOG("End Collision ENEMY");
		break;
	case ColliderType::DEATHZONE:
		LOG("End Collision DEATHZONE");
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::TakeDamage(int amount) {
	if (amount <= 0){
		return;
	}
	health -= amount;
	if (health <= 0) {
		health = 0;
	}
}

void Player::HealToFull() {
	health = maxHealth;
}

void Player::Respawn() {
	HealToFull();

	Physics* physics = Engine::GetInstance().physics.get();
	physics->SetLinearVelocity(pbody, { 0,0 });
	physics->SetTransform(pbody, spawnPos.getX(), spawnPos.getY());

	position = spawnPos;
	isJumping = false;
	isGrounded = false;
	jumpCount = 0;
	maxDownwardSpeed = 0.0f;
	anims.SetCurrent("idle");
	
	Engine::GetInstance().render->camera.x = position.getX();
	Engine::GetInstance().render->camera.y = position.getY();
	Engine::GetInstance().entityManager->resetEnemiesToSpwan();
	LOG("Player respawned at (%.1f, %.1f)", spawnPos.getX(), spawnPos.getY());
}
