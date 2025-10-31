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



Enemy::Enemy() : Entity(EntityType::ENEMY) {
	name = "Enemy";
}

Enemy::~Enemy() {

}


bool Enemy::Awake() {
	position = Vector2D(96, 96);
	spawnPos = position;
	health = maxHealth;
	return true;
}


bool Enemy::Start() {
	texture = Engine::GetInstance().textures->Load("Assets/Textures/BudEnem_spritesheet.png");
	std::unordered_map<int, std::string> animNames = { {0, "move"} };

	anims.LoadFromTSX("Assets/Textures/MrHorseMan_spritesheet.tsx", animNames);
	anims.SetCurrent("move");

	texW = 32;
	texH = 32;
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texH / 2, bodyType::DYNAMIC);


	pbody->listener = this;
	pbody->ctype = ColliderType::ENEMY;

	return true;
}

bool Enemy::Update(float dt) {

	Physics* physics = Engine::GetInstance().physics.get();

	b2Vec2 velocity = physics->GetLinearVelocity(pbody);
	velocity = { 0, velocity.y };

	velocity.x = direction * speed; 
	physics->SetLinearVelocity(pbody, velocity);

	int x, y;
	pbody->GetPosition(x, y);

	if (x < patrolLeft)		direction = 1;
	if (x > patrolRight)	direction = -1;

	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2);

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
	return true;
}






