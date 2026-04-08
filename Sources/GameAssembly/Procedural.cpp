#include "Procedural.hpp"
#include <Termina/Physics/Components/BoxCollider.hpp>
#include <Termina/Physics/Components/Rigidbody.hpp>


// ============= LIFECYCLE =============

void Procedural::Start()
{
	TerminaScript::ScriptableComponent::Start();

	// Initialiser les prefabs
	m_WallPrefab = TerminaScript::Prefab("Assets/Prefabs/Wall.trp");
	m_BarricadePrefab = TerminaScript::Prefab("Assets/Prefabs/Barricade.trp");
	m_Coin = TerminaScript::Prefab("Assets/Prefabs/Coin.trp");

	// Chercher le joueur
	findPlayerActor();

	// Initialiser le générateur aléatoire
	m_RandomEngine.seed(std::random_device{}());
	m_NextSpawnZ = 25.0f;

	// Init pattern weight
	m_WeightedPatterns =
	{
		{PATTERN_EMPTY, 3.0f},
		{PATTERN_SINGLE_WALL, 2.0f},
		{PATTERN_SIDE_WALLS, 2.0f},
		{PATTERN_ZIGZAG, 1.5f},
		{PATTERN_GAP_LEFT, 1.5f},
		{PATTERN_GAP_RIGHT, 1.5f},
		{PATTERN_GAP_CENTER, 1.5f},
		{PATTERN_NARROW_GAP, 1.0f},
		{PATTERN_BARRICADE_WALL, 1.5f},
		{PATTERN_COINS_LEFT, 1.2f},
		{PATTERN_COINS_RIGHT, 1.2f},
		{PATTERN_COINS_CENTER, 1.0f},
		{PATTERN_COINS_ALL, 0.8f}
	};

	// Génération initiale
	procéduralGeneration();
}

void Procedural::Update(float dt)
{
	TerminaScript::ScriptableComponent::Update(dt);

	// Génération de nouveaux obstacles
	procéduralGeneration();

	// swap vecteurs pour éviter de supprimer pendant l'itération
	FObjectsDestroy();
}

void Procedural::OnPostUpdate(float dt)
{
	DestroyObjects();
}

// ============= RECHERCHE DU JOUEUR =============

void Procedural::findPlayerActor()
{
	// Chercher l'acteur nommé "Player"
	m_PlayerActor = m_Owner->GetParentWorld()->GetActorByName("Player");
	
	if (m_PlayerActor)
	{
		TN_INFO("Procedural: Joueur trouvé par nom 'Player'");
		return;
	}

	// Si non trouvé, chercher le premier acteur avec Transform
	const auto& actors = m_Owner->GetParentWorld()->GetActors();
	for (const auto& actorPtr : actors)
	{
		Termina::Actor* actor = actorPtr.get();
		if (actor && actor->HasComponent<Termina::Transform>())
		{
			// Vérifier que ce n'est pas le manager lui-même
			if (actor->GetName() != m_Owner->GetName())
			{
				m_PlayerActor = actor;
				TN_INFO("Procedural: Joueur trouvé: %s", actor->GetName().c_str());
				return;
			}
		}
	}

	TN_ERROR("Procedural: Impossible de trouver le joueur!");
}

// ============= POSITION DU JOUEUR =============

float Procedural::getPlayerZPosition() const
{
	if (!m_PlayerActor || !m_PlayerActor->HasComponent<Termina::Transform>())
	{
		return 0.0f;
	}

	return m_PlayerActor->GetComponent<Termina::Transform>().GetPosition().z;
}

// ============= GÉNÉRATION PROCÉDURALE =============

void Procedural::procéduralGeneration()
{
	float playerZ = getPlayerZPosition();

	while (m_NextSpawnZ < playerZ + 50.0f)
	{
		std::vector<int> lanes(3, 0);
		PatternType pattern;

		int attempts = 0;

		do
		{
			pattern = selectNextPattern();
			generatePattern(pattern, lanes);
			attempts++;
		} while ((!hasFreeLane(lanes) || !isReachable(lanes)) && attempts < 10);

		// fallback si aucun pattern valide
		if (attempts >= 10)
		{
			lanes = { 0, 1, 0 };
		}

		PatternLine line;
		line.lanes = lanes;
		line.spawnZ = m_NextSpawnZ;

		spawnObstaclesForLine(line);
		m_LastSafeLane = findSafeLane(lanes);
		m_NextSpawnZ += m_SpacingBetweenPatterns;
	}
}

// ============= DESTRUCTION =============

void Procedural::FObjectsDestroy()
{
	float playerZ = getPlayerZPosition();
	float threshold = playerZ - m_DestroyDistance;

	auto it = m_Objects.begin();

	while (it != m_Objects.end())
	{
		auto obj = *it;

		if (!obj || !obj->HasComponent<Termina::Transform>())
		{
			it = m_Objects.erase(it);
			continue;
		}

		float z = obj->GetComponent<Termina::Transform>().GetPosition().z;

		if (z < threshold)
		{
			m_ObjectsToDestroy.push_back(obj);
			it = m_Objects.erase(it);
		}
		else
		{
			++it;
		}

	}
}

void Procedural::DestroyObjects()
{
	for (auto* obj : m_ObjectsToDestroy)
	{
		if (!obj) continue;

		obj->SetActive(false);

		if (obj->HasComponent<Termina::Rigidbody>())
		{
			obj->GetComponent<Termina::Rigidbody>().SetActive(false);
		}

		if (obj->HasComponent<Termina::BoxCollider>())
		{
			obj->GetComponent<Termina::BoxCollider>().SetActive(false);
		}
		m_Owner->GetParentWorld()->DestroyActor(obj);
	}

	m_ObjectsToDestroy.clear();
}

// ============= SÉLECTION PATTERN =============

Procedural::PatternType Procedural::selectNextPattern()
{
	float totalWeight = 0.0f;

	for (auto& p : m_WeightedPatterns)
		totalWeight += p.weight;

	std::uniform_real_distribution<float> dist(0.0f, totalWeight);
	float r = dist(m_RandomEngine);

	float cumulative = 0.0f;

	for (auto& p : m_WeightedPatterns)
	{
		cumulative += p.weight;
		if (r <= cumulative)
			return p.type;
	}

	return PATTERN_EMPTY;
}

// ============= GÉNÉRATION PATTERNS =============

void Procedural::generatePattern(PatternType type, std::vector<int>& lanes)
{
	switch (type)
	{
		case PATTERN_EMPTY:
			lanes = {0, 0, 0};
			break;

		case PATTERN_SINGLE_WALL:
			lanes = {0, 1, 0};
			break;

		case PATTERN_SIDE_WALLS:
			lanes = {1, 0, 1};
			break;

		case PATTERN_ZIGZAG:
		{
			std::uniform_int_distribution<> dist(0, 1);
			lanes = dist(m_RandomEngine) ? std::vector<int>{1, 0, 0} : std::vector<int>{0, 0, 1};
			break;
		}

		case PATTERN_GAP_LEFT:
			lanes = {0, 1, 1};
			break;

		case PATTERN_GAP_RIGHT:
			lanes = {1, 1, 0};
			break;

		case PATTERN_GAP_CENTER:
			lanes = {1, 0, 1};
			break;

		case PATTERN_NARROW_GAP:
		{
			std::uniform_int_distribution<> dist(0, 2);
			int gap = dist(m_RandomEngine);
			lanes = {1, 1, 1};
			lanes[gap] = 0;
			break;
		}

		case PATTERN_BARRICADE_WALL:
		{
			std::uniform_int_distribution<> dist(0, 3);
			int pattern = dist(m_RandomEngine);

			switch (pattern)
			{
			case 0:
				lanes = { 2, 0, 0 };
				break;

			case 1:
				lanes = { 0, 0, 2 };
				break;

			case 2:
				lanes = { 2, 0, 2 };
				break;

			case 3:
				lanes = { 2, 3, 2 }; 
				break;
			}
			break;
		}

		case PATTERN_COINS_LEFT:
			lanes = {3, 0, 0};
			break;

		case PATTERN_COINS_RIGHT:
			lanes = {0, 0, 3};
			break;

		case PATTERN_COINS_CENTER:
			lanes = {0, 3, 0};
			break;

		case PATTERN_COINS_ALL:
			lanes = { 3, 3, 3 };


			m_NextSpawnZ += m_SpacingBetweenPatterns * 0.5f;
			break;

		default:
			lanes = {0, 0, 0};
			break;
	}
}

// ============= SPAWN OBSTACLES =============
void Procedural::spawnObstaclesForLine(const PatternLine& line)
{
	float baseX = -((3 - 1) * m_LaneWidth) / 2.0f;

	for (int i = 0; i < 3; ++i)
	{
		if (line.lanes[i] == 0) continue;

		float xPos = baseX + (i * m_LaneWidth);
		float spawnZ = line.spawnZ;

		Termina::Actor* obstacle = nullptr;
		float yPos = 2.0f;

		switch (line.lanes[i])
		{
		case 1: // WALL
			obstacle = Instantiate(m_WallPrefab);
			yPos = 2.0f;
			break;

		case 2: // BARRICADE
			obstacle = Instantiate(m_BarricadePrefab);
			yPos = 1.5f;
			break;

		case 3: // COIN
			obstacle = Instantiate(m_Coin);
			yPos = 2.0f;
			break;
		}

		if (!obstacle)
			continue;

		if (obstacle->HasComponent<Termina::Transform>())
		{
			auto& transform = obstacle->GetComponent<Termina::Transform>();
			transform.SetPosition(glm::vec3(xPos, yPos, spawnZ));

			m_Objects.push_back(obstacle);
		}
	}
}

bool Procedural::hasFreeLane(const std::vector<int>& lanes)
{
	for (int lane : lanes)
	{
		if (lane == 0)
			return true;
	}
	return false;
}

int Procedural::findSafeLane(const std::vector<int>& lanes)
{
	if (lanes.empty())
	{
		return -1;
	}

	int bestLane = -1;
	int bestDistance = 999;

	for (size_t i = 0; i < lanes.size(); ++i)
	{
		if (lanes[i] == 0)
		{
			int dist = abs((int)i - m_LastSafeLane);

			if (dist < bestDistance)
			{
				bestDistance = dist;
				bestLane = (int)i;
			}
		}
	}

	return bestLane;
}

bool Procedural::isReachable(const std::vector<int>& lanes)
{
	if (m_LastSafeLane < 0) return true;

	for (size_t i = 0; i < lanes.size(); ++i)
	{
		if (lanes[i] == 0)
		{
			if (abs((int)i - m_LastSafeLane) <= 1)
				return true;
		}
	}
	return false;
}