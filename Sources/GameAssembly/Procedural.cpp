#include "Procedural.hpp"


void Procedural::Start()
{
	TerminaScript::ScriptableComponent::Start();

	// Initialisation des prefabs
	m_WallPrefab = TerminaScript::Prefab("Assets/Prefabs/Wall.trp");
	m_BarricadePrefab = TerminaScript::Prefab("Assets/Prefabs/Barricade.trp");
	m_CarPrefab = TerminaScript::Prefab("Assets/Prefabs/Car.trp");

	// Initialisation du générateur aléatoire
	m_RandomEngine.seed(std::random_device{}());
	m_NextSpawnHeight = 0.0f;

	// Génération initiale
	procéduralGeneration();
}

void Procedural::Update(float dt)
{
	TerminaScript::ScriptableComponent::Update(dt);
	ObjectsUpdate(dt);
	DestroyObjects();
	procéduralGeneration();
	DestroyObjectsUpdate();
}

void Procedural::ObjectsUpdate(float dt)
{
	// Déplace chaque objet vers le haut
	for (auto& obj : m_Objects)
	{
		if (obj && obj->HasComponent<Termina::Transform>())
		{
			Termina::Transform& transform = obj->GetComponent<Termina::Transform>();
			glm::vec3 pos = transform.GetPosition();
			pos.y += m_ObjectsSpeed * dt;
			transform.SetPosition(pos);
		}
	}
}

void Procedural::procéduralGeneration()
{

	int currentCount = m_Objects.size();
	if (currentCount < m_MaxObjects)
	{
		int objectsToSpawn = m_MaxObjects - currentCount;

		for (int i = 0; i < objectsToSpawn; ++i)
		{
			// Sélectionner un pattern
			std::vector<int> lanes(3, 0); // 3 voies
			PatternType pattern = selectNextPattern();

			// Générer le pattern
			generatePattern(pattern, lanes);

			// Créer la ligne de pattern
			PatternLine line;
			line.lanes = lanes;
			line.height = m_NextSpawnHeight;

			// Spawn les obstacles
			spawnObstaclesForLine(line);

			// Avancer la hauteur de spawn
			m_NextSpawnHeight -= m_SpacingBetweenPatterns;
		}
	}
}

void Procedural::DestroyObjects()
{
	// Détruit les objets sortis de la carte (y > 30.0f)
	for (auto& obj : m_Objects)
	{
		if (obj && obj->HasComponent<Termina::Transform>())
		{
			Termina::Transform& transform = obj->GetComponent<Termina::Transform>();
			if (transform.GetPosition().y > 30.0f)
			{
				m_ObjectsToDestroy.push_back(obj);
			}
		}
	}
}

void Procedural::DestroyObjectsUpdate()
{
	for (auto& obj : m_ObjectsToDestroy)
	{
		Destroy(obj);
		m_Objects.erase(std::remove(m_Objects.begin(), m_Objects.end(), obj), m_Objects.end());
	}
	m_ObjectsToDestroy.clear();
}

// ============= PATTERNS =============

Procedural::PatternType Procedural::selectNextPattern()
{
	std::uniform_int_distribution<> dist(0, PATTERN_COUNT - 1);
	int patternIndex;

	// Éviter de répéter le même pattern
	do
	{
		patternIndex = dist(m_RandomEngine);
	} while (patternIndex == m_LastPatternIndex && m_LastPatternIndex != -1);

	m_LastPatternIndex = patternIndex;
	return static_cast<PatternType>(patternIndex);
}

void Procedural::generatePattern(PatternType type, std::vector<int>& lanes)
{
	switch (type)
	{
		case PATTERN_EMPTY:
			for (int& lane : lanes) lane = 0;
			break;

		case PATTERN_SINGLE_WALL:
			lanes[0] = 0;
			lanes[1] = 1; // Mur
			lanes[2] = 0;
			break;

		case PATTERN_SIDE_WALLS:
			lanes[0] = 1; // Mur
			lanes[1] = 0;
			lanes[2] = 1; // Mur
			break;

		case PATTERN_ZIGZAG:
		{
			std::uniform_int_distribution<> dist(0, 1);
			if (dist(m_RandomEngine))
			{
				lanes[0] = 1;
				lanes[1] = 0;
				lanes[2] = 0;
			}
			else
			{
				lanes[0] = 0;
				lanes[1] = 0;
				lanes[2] = 1;
			}
			break;
		}

		case PATTERN_GAP_LEFT:
			lanes[0] = 0; // Passage
			lanes[1] = 1; // Mur
			lanes[2] = 1; // Mur
			break;

		case PATTERN_GAP_RIGHT:
			lanes[0] = 1; // Mur
			lanes[1] = 1; // Mur
			lanes[2] = 0; // Passage
			break;

		case PATTERN_GAP_CENTER:
			lanes[0] = 1; // Mur
			lanes[1] = 0; // Passage
			lanes[2] = 1; // Mur
			break;

		case PATTERN_NARROW_GAP:
		{
			std::uniform_int_distribution<> dist(0, 2);
			int gapLane = dist(m_RandomEngine);
			for (int i = 0; i < 3; ++i)
				lanes[i] = (i == gapLane) ? 0 : 1;
			break;
		}

		case PATTERN_BARRICADE_WALL:
			for (int& lane : lanes) lane = 2; // Barricades
			break;

		case PATTERN_CAR_OBSTACLE:
		{
			std::uniform_int_distribution<> dist(0, 2);
			int carLane = dist(m_RandomEngine);
			for (int i = 0; i < 3; ++i)
				lanes[i] = (i == carLane) ? 3 : 0; // Voiture
			break;
		}

		default:
			for (int& lane : lanes) lane = 0;
			break;
	}
}

void Procedural::spawnObstaclesForLine(const PatternLine& line)
{
	const float laneSpacing = m_LaneWidth;
	const float baseX = -1.5f; // Position de départ

	for (int i = 0; i < 3; ++i)
	{
		if (line.lanes[i] == 0) continue; // Voie vide

		float xPos = baseX + (i * laneSpacing);
		Termina::Actor* obstacle = nullptr;

		// Spawn l'obstacle approprié
		switch (line.lanes[i])
		{
			case 1: // Mur
				obstacle = Instantiate(m_WallPrefab);
				break;
			case 2: // Barricade
				obstacle = Instantiate(m_BarricadePrefab);
				break;
			case 3: // Voiture
				obstacle = Instantiate(m_CarPrefab);
				break;
			default:
				break;
		}

		if (obstacle && obstacle->HasComponent<Termina::Transform>())
		{
			Termina::Transform& transform = obstacle->GetComponent<Termina::Transform>();
			transform.SetLocalPosition(glm::vec3(xPos, line.height, 0.0f));
			m_Objects.push_back(obstacle);
		}
	}
}

bool Procedural::isPatternPassable(const std::vector<int>& lanes)
{
	// Vérifier qu'il y a au moins une voie libre
	for (int lane : lanes)
	{
		if (lane == 0) return true;
	}
	return false;
}

