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

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(96, 96);
	spawnPos = position;
	health = maxHealth;
	return true;
}

bool Player::Start() {

	//L03: TODO 2: Initialize Player parameters
	//L10: TODO 3; Load the spritesheet of the player
	texture = Engine::GetInstance().textures->Load("Assets/Textures/MrHorseMan_spritesheet.png");

	//L10: TODO 3: Load the spritesheet animations from the TSX file
	std::unordered_map<int, std::string> animNames = { {0, "idle"}, {6, "move"}, {12, "jump"} };
	anims.LoadFromTSX("Assets/Textures/MrHorseMan_spritesheet.tsx", animNames);
	anims.SetCurrent("idle");
	// L08 TODO 5: Add physics to the player - initialize physics body
	texW = 32;
	texH = 32;
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texH / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;
	//Engine::GetInstance().physics->SetTransform(pbody, spawnPos.getX(), spawnPos.getY());

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/coin-collision-sound-342335.wav");

	return true;
}

bool Player::Update(float dt)
{
	Physics* physics = Engine::GetInstance().physics.get();

	// Read current velocity
	b2Vec2 velocity = physics->GetLinearVelocity(pbody);
	velocity = { 0, velocity.y }; // Reset horizontal velocity
	bool moving = false;

	// Move left/right
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -speed;
		//L10: TODO 6: Update the animation based on the player's state
		anims.SetCurrent("move");
		flip = SDL_FLIP_HORIZONTAL; //flips the player's character when moving left
		moving = true;
	}
	else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = speed;
		//L10: TODO 6: Update the animation based on the player's state
		anims.SetCurrent("move");
		flip = SDL_FLIP_NONE;
		moving = true;
	}
	else
		if (isGrounded)
			anims.SetCurrent("idle");

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

// L10: TODO 5: Draw the player using the texture and the current animation frame
Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - 1.5* texH, &animFrame, 1.0f, 0.0, INT_MAX, INT_MAX, flip);

//health screen display
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
		//LOG("Collision PLATFORM");
		if (isJumping) {
			if (maxDownwardSpeed > fallSpeedDamageThreshold) {
				float t = (std::min(maxDownwardSpeed, fallSpeedMax) - fallSpeedDamageThreshold) / std::max(0.001f, (fallSpeedMax - fallSpeedDamageThreshold));
				int dmg = (int)(t * 60.0f);
				TakeDamage(dmg);
			}
			LOG("jump x platform");
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
		physB->listener->Destroy();
		break;
	case ColliderType::ENEMY:
		TakeDamage(10);
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

	isJumping = false;
	anims.SetCurrent("idle");

	Engine::GetInstance().entityManager->resetEnemiesToSpwan();
}
