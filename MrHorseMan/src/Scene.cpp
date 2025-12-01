#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Enemy.h"
#include "Item.h"

Scene::Scene() : Module()
{
	name = "scene";
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

bool Scene::Start()
{
    //Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/level-iv-339695.wav");
	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/backAgain.wav");
    Engine::GetInstance().map->Load("Assets/Maps/", "MapTemplate.tmx");

    auto spawnObjects = Engine::GetInstance().map->GetObjects("Spawns");

    for (const auto& object : spawnObjects) {
        
        std::string type = object.type;
        if (type.empty()) {
            auto it = object.properties.find("type");
            if (it == object.properties.end()) it = object.properties.find("Type");
            if (it != object.properties.end()) type = it->second;
        }
        if (type.empty()) type = object.name;

        for (auto& c : type) c = (char)std::tolower((unsigned char)c); //omg la deep web

        if (type == "player") {
            player = std::dynamic_pointer_cast<Player>(Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER));
            if (!player) { LOG("ERROR: Player creation failed"); continue; }
            player->position = { object.x + 16.0f, object.y + 16.0f };
            player->spawnPos = player->position;
        }
        else if (type == "enemy") {
            auto enemy = std::dynamic_pointer_cast<Enemy>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY));
            enemy->position = { object.x + 16.0f, object.y + 16.0f };
            enemy->spawnPos = enemy->position;
        }
        else if (type == "item") {
            auto item = std::dynamic_pointer_cast<Item>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM));
            item->position = { object.x, object.y };
        }
        else {
            LOG("Spawn skipped: unknown type='%s' (name='%s') at (%.1f,%.1f)", type.c_str(), object.name.c_str(), object.x, object.y);
        }
    }

    return true;
}



bool Scene::PreUpdate()
{
	return true;
}

bool Scene::Update(float dt)
{
	
	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

// Return the player position
Vector2D Scene::GetPlayerPosition()
{
    return player->GetPosition();
}
