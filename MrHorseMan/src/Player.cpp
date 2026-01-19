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
#include "Enemy.h"


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

	//Imagenes
	controlsPNG = Engine::GetInstance().textures->Load("Assets/Textures/Controls.png");

	//Audios
	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/coin-collision-sound-342335.wav");
	jump1FX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/1st jump.wav");
	jump2FX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/2nd jump.wav");
	walkingFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/correr.wav");
	gameOverFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/game over.wav");
	hitedFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/hited.wav");
	horseNeighFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/horse-neigh.wav");
	lvlFinishedFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/level finished.wav");
	dashFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/dash.wav");
	enemiDiedFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/enemy_dead.wav");

	return true;
}

bool Player::Update(float dt)
{
	Physics* physics = Engine::GetInstance().physics.get();

	if (pendingRespawn) {
		pendingRespawn = false;
		Respawn();
	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN && dashed == false && (isJumping == true || isGrounded == true)) {
		Respawn();
	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_H) == KEY_DOWN)
	{
		showControlsImage = !showControlsImage;   // Cambia estado ON/OFF
		LOG("Toggling help image: %s", showControlsImage ? "ON" : "OFF");
	}

	// Read current velocity
	b2Vec2 velocity = physics->GetLinearVelocity(pbody);
	if(!dashed)velocity = { 0, velocity.y }; // Reset horizontal velocity
	bool moving = false;

	//GodMode
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		LOG("GodMode_Switched");

		godMode = !godMode;
	}

	// Move left/right
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && dashing == false) {
		velocity.x = -speed;
		anims.SetCurrent("move");
		flip = SDL_FLIP_HORIZONTAL; //flips the player's character when moving left
		moving = true;
		isRight = -1;
	}
	else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && dashing == false) {
		velocity.x = speed;
		anims.SetCurrent("move");
		flip = SDL_FLIP_NONE;
		moving = true;
		isRight = 1;
	}

	// Features godMode
	if (godMode == true) {

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT) {
			velocity.y = -speed *2;
			moving = true;
		}

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT) {
			velocity.y = +speed*2;
			moving = true;
		}

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT) {
			velocity.x = -speed*2;
			moving = true;
		}

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT) {
			velocity.x = +speed*2;
			moving = true;
		}

		b2Body_SetGravityScale(pbody->body, 0.0f); //desactiva gravedad

	}

	//Dash															
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN && dashed == false && (isJumping == true || isGrounded == true)) {
		LOG("dash");
		dashed = true;
		dashing = true;
		currentTime = 0.0f;

		Engine::GetInstance().audio->PlayFx(dashFX);
		b2Body_SetGravityScale(pbody->body, 0.0f); //desactiva gravedad
		velocity.y = 0;
		physics->SetLinearVelocity(pbody, { 0, 0 });
		physics->ApplyLinearImpulseToCenter(pbody, 3.0f * isRight,0.0f, true);
	}

	if (dashed == true) {
		currentTime += dt; // vas contando
		LOG("dashing %f", currentTime);
		if (currentTime >= maxTime) {

			b2Body_SetGravityScale(pbody->body, 1.0f); //activas gravedad
			dashing = false;

			// El dash ha terminado completamente
			b2Vec2 endVel = physics->GetLinearVelocity(pbody);

			// Si el jugador está pulsando A o D → retomar velocidad normal
			if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			{
				endVel.x = -speed;
			}
			else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			{
				endVel.x = speed;
			}
			else
			{
				// Si no pulsa nada -> quedas quieto horizontalmente
				velocity.x = 0;
			}

			physics->SetLinearVelocity(pbody, endVel);

			
		}
	}

	// Jump (impulse once)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && jumpCount < maxJumps) {
		if (jumpCount < 1) {
			Engine::GetInstance().audio->PlayFx(jump1FX);
		}
		else if(jumpCount > 0){
			Engine::GetInstance().audio->PlayFx(jump2FX);
		}
		
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
if (!dashing) {
	physics->SetLinearVelocity(pbody, velocity);
}

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
Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - 1.5 * texH, &animFrame, 1.0f, 0.0, INT_MAX, INT_MAX, flip);

//health screen display TODO NEXT TIME. THIS TIME, NO UI REQUIRED
char hpText[32];
snprintf(hpText, sizeof(hpText), "HP: %d", health);

int screenW = Engine::GetInstance().render->camera.w;
int margin = 12;
int posTextX = screenW - 120;
int posTextY = margin;


return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	Engine::GetInstance().textures->UnLoad(controlsPNG);
	controlsPNG = nullptr;
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

		if (dashed == true) {
			b2Body_SetGravityScale(pbody->body, 1.0f);
		}

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
		dashed = false;
		dashing = false;
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
	{
		int px, py;
		int ex, ey;
		pbody->GetPosition(px, py);
		physB->GetPosition(ex, ey);

		// Si la Y del player es MÁS ALTA que la del enemigo (player por encima)
		bool playerAbove = py < ey - (texH / 4);  // margen para evitar colisiones laterales

		if (playerAbove)
		{
			LOG("PLAYER HA PISADO AL ENEMIGO COMO UN CAMIÓN");
			Engine::GetInstance().audio->PlayFx(enemiDiedFx);

			// Rebote
			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0, -1.0f, true);

			// Destruir el enemigo
			auto enemy = dynamic_cast<Enemy*>(physB->listener);
			enemy->pendingToDelete = true;
			
			dashed == false;
		}
		else
		{

			if (!godMode) {
				TakeDamage(10);
				LOG("Collision Enemy. Health %d", health);
			}
		}
		break;
	}

	case ColliderType::DEATHZONE:
		if (!godMode) {
			LOG("DeathZone hit. Respawning");
			pendingRespawn = true;
		}
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

Vector2D Player::GetPosition() {
	int x, y;
	pbody->GetPosition(x, y);
	// Adjust for center
	return Vector2D((float)x - texW / 2, (float)y - texH / 2);
}

void Player::SetPosition(Vector2D pos) {
	pbody->SetPosition((int)(pos.getX() + texW / 2), (int)(pos.getY() + texH / 2));
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
	Engine::GetInstance().audio->PlayFx(horseNeighFX);

	Physics* physics = Engine::GetInstance().physics.get();
	physics->SetLinearVelocity(pbody, { 0,0 });
	physics->SetTransform(pbody, spawnPos.getX(), spawnPos.getY());

	position = spawnPos;
	isJumping = false;
	isGrounded = false;
	dashed = false;
	b2Body_SetGravityScale(pbody->body, 1.0f); //activas gravedad
	jumpCount = 0;
	maxDownwardSpeed = 0.0f;
	anims.SetCurrent("idle");
	
	Engine::GetInstance().render->camera.x = position.getX();
	Engine::GetInstance().render->camera.y = position.getY();
	Engine::GetInstance().entityManager->resetEnemiesToSpwan();
	LOG("Player respawned at (%.1f, %.1f)", spawnPos.getX(), spawnPos.getY());
}
