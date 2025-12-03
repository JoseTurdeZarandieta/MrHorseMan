#include "Enemy.h"
#include "Engine.h"
#include "Physics.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "Scene.h"
#include "Log.h"
#include "EntityManager.h"
#include "Map.h"
#include "Player.h"




Enemy::Enemy() : Entity(EntityType::ENEMY) {
	name = "Enemy";
}

Enemy::~Enemy() {

}


bool Enemy::Awake() {
	spawnPos = position;
	health = maxHealth;
	return true;
}


bool Enemy::Start() {
	spawnPos = position;
	texture = Engine::GetInstance().textures->Load("Assets/Textures/rino_sprites.png");

	std::unordered_map<int, std::string> animNames = { {1, "move"}/*, {5, "move"} */};
	anims.LoadFromTSX("Assets/Textures/rino.tsx", animNames);
	anims.SetCurrent("move");

	texW = 64;
	texH = 32;
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texH / 2, bodyType::DYNAMIC);

	pbody->listener = this;
	pbody->ctype = ColliderType::ENEMY;

	pathfinding = std::make_shared<Pathfinding>();
	Vector2D pos = GetPosition();	
	Vector2D tilePos = Engine::GetInstance().map->WorldToMap((int)pos.getX(), (int)pos.getY() + 1);
	pathfinding->ResetPath(tilePos);

	if (patrolLeft == 0.0f && patrolRight == 0.0f) {
		patrolLeft = position.getX() - 64.0f;
		patrolRight = position.getX() + 64.0f;
	}

	return true;
}

bool Enemy::Update(float dt) {

	void PerformPathFinding();
	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	Physics* physics = Engine::GetInstance().physics.get();
	
	b2Vec2 velocity = physics->GetLinearVelocity(pbody);
	velocity = { 0, velocity.y };

	velocity.x = direction * speed; 
	physics->SetLinearVelocity(pbody, velocity);

	int x, y;
	pbody->GetPosition(x, y);

	if (x < patrolLeft)		direction = 1;
	if (x > patrolRight)	direction = -1;

	Vector2D pos = { (float)(x - texW / 2), (float)(y - texH / 2) };

	//Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame);

	SDL_Rect src = animFrame; // copia
	if (direction == 1) {
		// crear src flip horizontal invirtiendo la anchura (algunos motores caseros hacen flip con w negativo)
		src.x = animFrame.x + animFrame.w - 1; // empezar por el extremo derecho del frame
		src.w = -animFrame.w;                  // ancho negativo -> flip horizontal (si render lo soporta)
	}
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &src);


	return true;
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	if (physB->ctype == ColliderType::ENEMY) {
		LOG("Enemy collided with player"); 
		//player in charge of self damage reduction
	}
}

void Enemy::ResetToSpawn() {
	Physics* physics = Engine::GetInstance().physics.get();
	physics->SetLinearVelocity(pbody, { 0,0 });
	physics->SetTransform(pbody, spawnPos.getX(), spawnPos.getY());
	direction = 1;
}

bool Enemy::CleanUp() {

	LOG("Cleanup enemy");
	Engine::GetInstance().textures->UnLoad(texture);
	Engine::GetInstance().physics->DeletePhysBody(pbody);
	return true;
}

void Enemy::PerformPathFinding() {

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_DOWN) {
		pathfinding->PropagateAStar(SQUARED);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(SQUARED);
	}
}

Vector2D Enemy::GetPosition() {
	int x, y;
	pbody->GetPosition(x, y);
	// Adjust for center
	return Vector2D((float)x - texW / 2, (float)y - texH / 2);
}




