#include "Procedural.hpp"
#include <glm/glm.hpp>

// ============= LIFECYCLE =============

void Procedural::Start()
{
	TerminaScript::ScriptableComponent::Start();

	// Initialiser les prefabs
	m_WallPrefab = TerminaScript::Prefab("Assets/Prefabs/Wall.trp");
	m_BarricadePrefab = TerminaScript::Prefab("Assets/Prefabs/Barricade.trp");
	m_CarPrefab = TerminaScript::Prefab("Assets/Prefabs/Car.trp");

	// Chercher le joueur
	findPlayerActor();

	// Initialiser le générateur aléatoire
	m_RandomEngine.seed(std::random_device{}());
	m_NextSpawnZ = 25.0f; // Commencer plus loin pour éviter les collisions

	//Init patern weight
	m_WeightedPatterns =
	{
		{PATTERN_EMPTY, 3.0f},
		{PATTERN_SINGLE_WALL, 2.0f},
		{PATTERN_SIDE_WALLS, 2.0f},
		{PATTERN_ZIGZAG, 1.5f},
		{PATTERN_GAP_LEFT, 1.5f},
		{PATTERN_GAP_RIGHT, 1.5f},
		{PATTERN_NARROW_GAP, 1.0f},
		{PATTERN_CAR_OBSTACLE, 1.0f}
	};

	// Génération initiale
	procéduralGeneration();
}

void Procedural::Update(float dt)
{
	TerminaScript::ScriptableComponent::Update(dt);
	
	// Destruction des obstacles trop loin
	DestroyObjects();
	
	// Génération de nouveaux obstacles
	procéduralGeneration();
	
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
			lanes = { 0, 1, 0 }; // centre libre garanti
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


	void Procedural::DestroyObjects()
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
				Destroy(obj);
				it = m_Objects.erase(it);
			}
			else
			{
				++it;
			}
		}
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
			// Plus fréquent pour laisser respirer le joueur
			lanes = {0, 0, 0};
			break;

		case PATTERN_SINGLE_WALL:
			// Mur au centre seulement
			lanes = {0, 1, 0};
			break;

		case PATTERN_SIDE_WALLS:
			// Murs sur les côtés, passage au centre
			lanes = {1, 0, 1};
			break;

		case PATTERN_ZIGZAG:
		{
			// Zigzag alternant
			std::uniform_int_distribution<> dist(0, 1);
			lanes = dist(m_RandomEngine) ? std::vector<int>{1, 0, 0} : std::vector<int>{0, 0, 1};
			break;
		}

		case PATTERN_GAP_LEFT:
			// Trou à gauche seulement
			lanes = {0, 1, 1};
			break;

		case PATTERN_GAP_RIGHT:
			// Trou à droite seulement
			lanes = {1, 1, 0};
			break;

		case PATTERN_GAP_CENTER:
			// Trou au centre
			lanes = {1, 0, 1};
			break;

		case PATTERN_NARROW_GAP:
		{
			// Un seul passage étroit
			std::uniform_int_distribution<> dist(0, 2);
			int gap = dist(m_RandomEngine);
			lanes = {1, 1, 1};
			lanes[gap] = 0;
			break;
		}

		case PATTERN_CAR_OBSTACLE:
		{
			
			std::uniform_int_distribution<> dist(0, 2);
			int car = dist(m_RandomEngine);
			lanes = {0, 0, 0};
			lanes[car] = 3;
			break;
		}

		default:
			lanes = {0, 0, 0};
			break;
	}
}

// ============= SPAWN OBSTACLES =============

void Procedural::spawnObstaclesForLine(const PatternLine& line)
{
	float baseX = -((3 - 1) * m_LaneWidth) / 2.0f; // Ajusté pour nouvelle largeur de lane

	for (int i = 0; i < 3; ++i)
	{
		if (line.lanes[i] == 0) continue;

		float xPos = baseX + (i * m_LaneWidth);
		float spawnZ = line.spawnZ;

		Termina::Actor* obstacle = nullptr;

		// Instantier le bon prefab
		switch (line.lanes[i])
		{
			case 1:
				obstacle = Instantiate(m_WallPrefab);
				break;
			case 2:
				obstacle = Instantiate(m_BarricadePrefab);
				break;
			case 3:
				obstacle = Instantiate(m_CarPrefab);
				break;
		}

		if (obstacle && obstacle->HasComponent<Termina::Transform>())
		{
			// Position absolue : X pour la lane, Y = 0, Z devant le joueur
			Termina::Transform& transform = obstacle->GetComponent<Termina::Transform>();
			transform.SetPosition(glm::vec3(xPos, 2.0f, spawnZ));
			
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
	int bestLane = -1;
	int bestDistance = 999;

	for (int i = 0; i < lanes.size(); ++i)
	{
		if (lanes[i] == 0)
		{
			int dist = abs(i - m_LastSafeLane);

			if (dist < bestDistance)
			{
				bestDistance = dist;
				bestLane = i;
			}
		}
	}

	return bestLane;
}

bool Procedural::isReachable(const std::vector<int>& lanes)
{
	if (m_LastSafeLane < 0) return true;

	for (int i = 0; i < lanes.size(); ++i)
	{
		if (lanes[i] == 0)
		{
			if (abs(i - m_LastSafeLane) <= 1)
				return true;
		}
	}
	return false;
}

