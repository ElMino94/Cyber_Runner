#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <vector>
#include <random>

using namespace TerminaScript;

class Procedural : public TerminaScript::ScriptableComponent
{
private:
	// === PARAMÈTRES DE GÉNÉRATION ===
	float m_LaneWidth = 2.0f;
	float m_SpacingBetweenPatterns = 4.5f;
	float m_DestroyDistance = 20.0f;
	int m_LastSafeLane = 1;

	// === PREFABS ===
	TerminaScript::Prefab m_WallPrefab;
	TerminaScript::Prefab m_BarricadePrefab;
	TerminaScript::Prefab m_Coin;

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
		PATTERN_COINS_LEFT,      // Pièces à gauche
		PATTERN_COINS_RIGHT,     // Pièces à droite
		PATTERN_COINS_CENTER,    // Pièces au centre
		PATTERN_COINS_ALL,       // Pièces partout
		PATTERN_COUNT
	};

	// === STRUCTURE PATTERN ===
	struct PatternLine
	{
		std::vector<int> lanes;  // 0=vide, 1=mur, 2=barricade, 3=coin
		float spawnZ;
	};

	struct WeightedPattern
	{
		PatternType type;
		float weight;
	};

protected:
	std::vector<Termina::Actor*> m_Objects;
	std::vector<Termina::Actor*> m_ObjectsToDestroy;
	std::vector<WeightedPattern> m_WeightedPatterns;

public:
	Procedural() = default;
	Procedural(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

	void Start() override;
	void Update(float dt) override;
	void OnPostUpdate(float dt) override;

private:
	void procéduralGeneration();
	void FObjectsDestroy();
	void DestroyObjects();
	void findPlayerActor();
	bool hasFreeLane(const std::vector<int>& lanes);
	int findSafeLane(const std::vector<int>& lanes);
	bool isReachable(const std::vector<int>& lanes);

	// === PATTERNS ===
	PatternType selectNextPattern();
	void generatePattern(PatternType type, std::vector<int>& lanes);
	void spawnObstaclesForLine(const PatternLine& line);
	float getPlayerZPosition() const;
};
