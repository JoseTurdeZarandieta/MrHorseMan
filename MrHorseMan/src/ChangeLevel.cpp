#include "ChangeLevel.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

ChangeLevel::ChangeLevel() : Entity(EntityType::ChangeLevel)
{
	name = "changelevel";
}

ChangeLevel::~ChangeLevel() {}

bool ChangeLevel::Awake() {
	spawnPos = position;
	return true;
}

bool ChangeLevel::Start() {

	spawnPos = position;

	//initilize textures
	texture = Engine::GetInstance().textures->Load("Assets/Textures/Doors.png");
	
	Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::STATIC);
	pbody->ctype = ColliderType::CHANGELEVEL;

	pbody->listener = this;   

	return true;
}

bool ChangeLevel::Update(float dt)
{
	if (!active) return true;

	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);

	Vector2D pos = { (float)(x - texW / 2), (float)(y - texH / 2) };
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2);

	if (isPicked) {
		active = false;

		if (pbody) {
			pbody->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
		}
		Engine::GetInstance().entityManager->DestroyEntity(shared_from_this());

		if (Engine::GetInstance().scene->Level1 == true) {
			Engine::GetInstance().scene->ChangeScene(SceneID::LEVEL2);
			LOG("Loading LEVEL 2");
		}
		else if (Engine::GetInstance().scene->Level2 == true) {
			LOG("LEVEL FINISHED");
			Engine::GetInstance().scene->ChangeScene(SceneID::END_MENU);
			LOG("Loading End menu");
		}

		return true;
	}

	return true;
}

bool ChangeLevel::CleanUp()
{
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody) {
		pbody->listener = nullptr;
	}
	Engine::GetInstance().physics->DeletePhysBody(pbody);
	pbody = nullptr;

	return true;
}

void ChangeLevel::ResetToSpawn() {
	Physics* physics = Engine::GetInstance().physics.get();
	physics->SetTransform(pbody, spawnPos.getX(), spawnPos.getY());
}

bool ChangeLevel::Destroy()
{
	LOG("Destroying item");
	active = false;
	Engine::GetInstance().entityManager->DestroyEntity(shared_from_this());
	return true;
}

