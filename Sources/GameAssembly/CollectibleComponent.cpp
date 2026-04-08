#include "CollectibleComponent.hpp"
#include <Termina/Physics/Components/SphereCollider.hpp>
#include <Termina/Physics/Components/Rigidbody.hpp>
#include <Termina/Renderer/Components/MeshComponent.hpp>
#include "RunnerGameManager.hpp"

void Collectibles::Start()
{
	m_Player = m_Owner->GetParentWorld()->GetActorByName("Player");

	// Collider (SphereCollider)
	auto& collider = m_Owner->AddComponent<Termina::SphereCollider>();
	collider.Radius = 0.5f;

	// Rigidbody avec IsSensor = true
	auto& rb = m_Owner->AddComponent<Termina::Rigidbody>();
	rb.Type = Termina::Rigidbody::BodyType::Static;
	rb.IsSensor = true;

	// Chercher le RunnerGameManager UNE SEULE FOIS au démarrage
	Termina::World* world = m_Owner->GetParentWorld();
	if (world)
	{
		const auto& actors = world->GetActors();
		for (const auto& actorPtr : actors)
		{
			Termina::Actor* actor = actorPtr.get();
			if (actor && actor->HasComponent<RunnerGameManager>())
			{
				m_GameManager = &actor->GetComponent<RunnerGameManager>();
				break;
			}
		}

		if (!m_GameManager)
		{
			TN_WARN("Collectible: RunnerGameManager not found in world!");
		}
	}
}

void Collectibles::Update(float deltaTime)
{
}

void Collectibles::OnTriggerEnter(Termina::Actor* other)
{
	if (!other || other != m_Player)
		return;

	if (m_isCollected)
		return;

	m_isCollected = true;

	// Ajouter le score
	if (m_GameManager)
	{
		m_GameManager->AddScore(value);
		TN_INFO("Collectible '%s' collected! Score +%d", m_Owner->GetName().c_str(), value);
	}

	// Supprimer le composant mesh pour masquer l'objet
	if (m_Owner->HasComponent<Termina::MeshComponent>())
		m_Owner->RemoveComponent<Termina::MeshComponent>();

	// Désactiver tous les composants restants
	for (auto* comp : m_Owner->GetAllComponents())
		comp->SetActive(false);

	// Désactiver l'actor lui-même
	m_Owner->SetActive(false);
}