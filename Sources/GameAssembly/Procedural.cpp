#include "Procedural.hpp"


void Procedural::Start()
{
	TerminaScript::ScriptableComponent::Start();
	// Génération initiale des objets pour remplir le niveau
}

void Procedural::Update(float dt)
{
	TerminaScript::ScriptableComponent::Update(dt);
	ObjectsUpdate(dt);
	DestroyObjects();
	// Régénération procédurale : maintient le nombre d'objets sur la carte
	procéduralGeneration();
	DestroyObjectsUpdate();
	UpdateObjectCount(m_Objects.size());
}

void Procedural::ObjectsUpdate(float dt)
{
	// Fait avancer chaque objet de +1 par dt 
	for (auto& obj : m_Objects)
	{
		if (obj && obj->HasComponent<Termina::Transform>())
		{
			Termina::Transform& transform = obj->GetComponent<Termina::Transform>();
			transform.SetPosition(transform.GetPosition() + glm::vec3(0.0f, 1.0f * dt, 0.0f));
		}
	}
}

void Procedural::procéduralGeneration()
{
	// Génère de nouveaux objets seulement si le nombre actuel est inférieur au maximum
	int currentCount = m_Objects.size();
	if (currentCount < m_MaxObjects)
	{
		int objectsToSpawn = m_MaxObjects - currentCount;
		for (int i = 0; i < objectsToSpawn; ++i)
		{
			// Instantiate() sans paramètre crée un acteur vide
			Termina::Actor* newObj = Instantiate(Wall);
			
			// Récupère le composant Transform et configure la position
			if (newObj && newObj->HasComponent<Termina::Transform>())
			{
				Termina::Transform& transform = newObj->GetComponent<Termina::Transform>();
				transform.SetLocalPosition(glm::vec3(0.0f, -5.0f, 0.0f));
			}
			
			m_Objects.push_back(newObj);
		}
	}
}

void Procedural::DestroyObjects()
{
	// Identifie les objets sortis de la carte (au-delà de y > 30.0f)
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
	// Détruit chaque objet marqué pour destruction
	for (auto& obj : m_ObjectsToDestroy)
	{
		Destroy(obj);
		m_Objects.erase(std::remove(m_Objects.begin(), m_Objects.end(), obj), m_Objects.end());
	}
	m_ObjectsToDestroy.clear();
}

