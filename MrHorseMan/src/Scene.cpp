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
    auto input = Engine::GetInstance().input.get();

    // -------- SAVE GAME (F5) ----------
    if (input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
    {
        LOG("Saving game...");
        SaveGame();
    }

    // -------- LOAD GAME (F6) ----------
    if (input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
    {
        LOG("Loading game...");
        LoadGame();
    }

    return true;
}

bool Scene::SaveGame()
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("game_state");

    // Player
    pugi::xml_node playerNode = root.append_child("player");
    Vector2D pos = player->GetPosition();
    playerNode.append_attribute("x") = pos.getX();
    playerNode.append_attribute("y") = pos.getY();

    // Enemies
    pugi::xml_node enemiesNode = root.append_child("enemies");

    int id = 0;
    for (auto& e : Engine::GetInstance().entityManager->entities)
    {
        if (e->type != EntityType::ENEMY) continue;

        Enemy* enemy = dynamic_cast<Enemy*>(e.get());
        if (!enemy) continue;

        pugi::xml_node enemyNode = enemiesNode.append_child("enemy");
        enemyNode.append_attribute("id") = id++;
        enemyNode.append_attribute("x") = enemy->position.getX();
        enemyNode.append_attribute("y") = enemy->position.getY();
        enemyNode.append_attribute("dead") = enemy->pendingToDelete;
    }

    doc.save_file("savegame.xml");
    LOG("Game saved!");
    return true;
}


bool Scene::LoadGame()
{
    pugi::xml_document doc;
    if (!doc.load_file("savegame.xml"))
    {
        LOG("Cannot load savegame.xml");
        return false;
    }

    pugi::xml_node root = doc.child("game_state");
    if (!root) return false;

    // Player
    pugi::xml_node playerNode = root.child("player");
    if (playerNode)
    {
        float x = playerNode.attribute("x").as_float();
        float y = playerNode.attribute("y").as_float();
        player->SetPosition({ x, y });
    }

    // Enemies
    pugi::xml_node enemiesNode = root.child("enemies");
    if (enemiesNode)
    {
        auto entityIt = Engine::GetInstance().entityManager->entities.begin();
        for (pugi::xml_node e = enemiesNode.child("enemy"); e && entityIt != Engine::GetInstance().entityManager->entities.end(); e = e.next_sibling("enemy"))
        {
            // Skip until we find the next enemy in the list
            while (entityIt != Engine::GetInstance().entityManager->entities.end() && (*entityIt)->type != EntityType::ENEMY)
            {
                ++entityIt;
            }
            if (entityIt == Engine::GetInstance().entityManager->entities.end()) break;

            Enemy* en = dynamic_cast<Enemy*>(entityIt->get());
            if (!en) { ++entityIt; continue; }

            en->position.setX(e.attribute("x").as_float());
            en->position.setY(e.attribute("y").as_float());
            en->pendingToDelete = e.attribute("dead").as_bool();

            ++entityIt; // move to the next entity
        }
    }

    LOG("Game loaded!");
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
