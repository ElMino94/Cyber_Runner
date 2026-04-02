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
	m_NextSpawnZ = 35.0f; // Commencer plus loin pour éviter les collisions

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
	
	// Nettoyage effectif
	DestroyObjectsUpdate();
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
	int currentCount = m_Objects.size();
	
	// Générer jusqu'à atteindre le maximum
	if (currentCount < m_MaxObjects)
	{
		int objectsToSpawn = m_MaxObjects - currentCount;

		for (int i = 0; i < objectsToSpawn; ++i)
		{
			// Créer un pattern
			std::vector<int> lanes(3, 0);
			PatternType pattern = selectNextPattern();
			generatePattern(pattern, lanes);

			// Construire la ligne
			PatternLine line;
			line.lanes = lanes;
			line.spawnZ = m_NextSpawnZ;

			// Spawn les obstacles
			spawnObstaclesForLine(line);

			// Préparer le prochain spawn - espacement augmenté pour éviter collisions
			m_NextSpawnZ += m_SpacingBetweenPatterns;
		}
	}
}

// ============= DESTRUCTION =============

void Procedural::DestroyObjects()
{
	float playerZ = getPlayerZPosition();
	float destroyThreshold = playerZ - m_DestroyDistance;

	for (auto& obj : m_Objects)
	{
		if (!obj || !obj->HasComponent<Termina::Transform>())
			continue;

		float objectZ = obj->GetComponent<Termina::Transform>().GetPosition().z;

		// Détruire si trop loin derrière le joueur
		if (objectZ < destroyThreshold)
		{
			m_ObjectsToDestroy.push_back(obj);
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

// ============= SÉLECTION PATTERN =============

Procedural::PatternType Procedural::selectNextPattern()
{
	std::uniform_int_distribution<> dist(0, PATTERN_COUNT - 1);
	int patternIndex;

	// Éviter répétition - favorise les patterns faciles
	do
	{
		patternIndex = dist(m_RandomEngine);
	} while (patternIndex == m_LastPatternIndex && m_LastPatternIndex != -1);

	m_LastPatternIndex = patternIndex;
	return static_cast<PatternType>(patternIndex);
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

		case PATTERN_BARRICADE_WALL:
			// Mur de barricades complet - moins fréquent
			lanes = {2, 2, 2};
			break;

		case PATTERN_CAR_OBSTACLE:
		{
			// Une seule voiture sur une voie
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
	float playerZ = getPlayerZPosition();
	float baseX = -2.0f; // Ajusté pour nouvelle largeur de lane

	for (int i = 0; i < 3; ++i)
	{
		if (line.lanes[i] == 0) continue;

		float xPos = baseX + (i * m_LaneWidth);
		float spawnZ = playerZ + line.spawnZ;

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

