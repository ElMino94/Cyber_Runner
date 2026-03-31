#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <vector>
#include <random>

using namespace TerminaScript;

class Procedural : public TerminaScript::ScriptableComponent
{
private:
	// Paramètres de génération
	int m_MaxObjects = 15;
	float m_ObjectsSpeed = 5.0f;
	float m_LaneWidth = 1.5f;
	float m_SpacingBetweenPatterns = 2.5f;

	// Prefabs
	TerminaScript::Prefab m_WallPrefab;
	TerminaScript::Prefab m_BarricadePrefab;
	TerminaScript::Prefab m_CarPrefab;

	// Générateur aléatoire
	std::mt19937 m_RandomEngine;
	int m_LastPatternIndex = -1;
	float m_NextSpawnHeight = 0.0f;

	// Types de patterns
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
		PATTERN_BARRICADE_WALL,  // Barricades
		PATTERN_CAR_OBSTACLE,    // Voiture
		PATTERN_COUNT
	};

	// Structure pour un pattern
	struct PatternLine
	{
		std::vector<int> lanes;  // 0 = vide, 1 = mur, 2 = barricade, 3 = voiture
		float height;
	};

protected:
	std::vector<Termina::Actor*> m_Objects;
	std::vector<Termina::Actor*> m_ObjectsToDestroy;

public:
	Procedural() = default;
	Procedural(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

	void Start() override;
	void Update(float dt) override;

	void ObjectsUpdate(float dt);
	void procéduralGeneration();
	void DestroyObjects();
	void DestroyObjectsUpdate();

	int GetMaxObjects() const { return m_MaxObjects; }
	float GetObjectsSpeed() const { return m_ObjectsSpeed; }

	void SetMaxObjects(int maxObjects) { m_MaxObjects = maxObjects; }
	void SetObjectsSpeed(float speed) { m_ObjectsSpeed = speed; }

private:
	// Génération de patterns
	PatternType selectNextPattern();
	void generatePattern(PatternType type, std::vector<int>& lanes);
	void spawnObstaclesForLine(const PatternLine& line);
	bool isPatternPassable(const std::vector<int>& lanes);
};
