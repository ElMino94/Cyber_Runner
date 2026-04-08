#include "CollectibleComponent.hpp"
#include <Termina/Physics/Components/SphereCollider.hpp>
#include <Termina/Physics/Components/Rigidbody.hpp>
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

	if (m_GameManager)
	{
		m_GameManager->AddScore(value);
	}

	m_Owner->SetActive(false);
	m_isCollected = true;
}