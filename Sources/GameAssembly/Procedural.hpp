#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <vector>
#include <random>

using namespace TerminaScript;

class Procedural : public TerminaScript::ScriptableComponent
{
private:
	// === PARAMÈTRES DE GÉNÉRATION ===
	int m_MaxObjects = 30;                    // Augmenté pour meilleure densité
	float m_LaneWidth = 2.0f;                 // Augmenté de 1.5 à 2.0 pour plus d'espace
	float m_SpacingBetweenPatterns = 4.5f;    // Augmenté de 2.5 à 3.5 pour éviter collisions
	float m_DestroyDistance = 20.0f;          // Augmenté de 30 à 40 pour plus de visibilité

	// === PREFABS ===
	TerminaScript::Prefab m_WallPrefab;
	TerminaScript::Prefab m_BarricadePrefab;
	TerminaScript::Prefab m_CarPrefab;

	// === RÉFÉRENCES ===
	Termina::Actor* m_PlayerActor = nullptr;

	// === GÉNÉRATEUR ALÉATOIRE ===
	std::mt19937 m_RandomEngine;
	int m_LastPatternIndex = -1;
	float m_NextSpawnZ = 0.0f;

	// === TYPES DE PATTERNS ===
	enum PatternType
	{
		PATTERN_EMPTY,           // Espace vide
		PATTERN_SINGLE_WALL,     // Mur au centre
		PATTERN_SIDE_WALLS,      // Murs sur les côtés
		PATTERN_ZIGZAG,          // Zigzag
		PATTERN_GAP_LEFT,        // Trou à gauche
		PATTERN_GAP_RIGHT,       // Trou à droite
		PATTERN_GAP_CENTER,      // Trou au centre
		PATTERN_NARROW_GAP,      // Passage étroit
		PATTERN_BARRICADE_WALL,  // Mur de barricades
		PATTERN_CAR_OBSTACLE,    // Voiture
		PATTERN_COUNT
	};

	// === STRUCTURE PATTERN ===
	struct PatternLine
	{
		std::vector<int> lanes;  // 0=vide, 1=mur, 2=barricade, 3=voiture
		float spawnZ;
	};

protected:
	std::vector<Termina::Actor*> m_Objects;
	std::vector<Termina::Actor*> m_ObjectsToDestroy;

public:
	Procedural() = default;
	Procedural(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

	void Start() override;
	void Update(float dt) override;

private:
	void procéduralGeneration();
	void DestroyObjects();
	void DestroyObjectsUpdate();
	void findPlayerActor();
	bool hasFreeLane(const std::vector<int>& lanes);

	// === PATTERNS ===
	PatternType selectNextPattern();
	void generatePattern(PatternType type, std::vector<int>& lanes);
	void spawnObstaclesForLine(const PatternLine& line);
	float getPlayerZPosition() const;
};
