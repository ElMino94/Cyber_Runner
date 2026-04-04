#include "CollideCheck.hpp"
#include <Termina/Physics/Components/BoxCollider.hpp>
#include <Termina/Physics/Components/Rigidbody.hpp>
#include <Termina/Core/Logger.hpp>

void CollideStatus::Start()
{
	m_Player = m_Owner->GetParentWorld()->GetActorByName("Player");

	// Ajoute un collider si absent
	if (!m_Owner->HasComponent<Termina::BoxCollider>())
	{
		auto& collider = m_Owner->AddComponent<Termina::BoxCollider>();
		collider.HalfExtents = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	// Ajoute un rigidbody si absent
	if (!m_Owner->HasComponent<Termina::Rigidbody>())
	{
		auto& rigidbody = m_Owner->AddComponent<Termina::Rigidbody>();
		rigidbody.Type = Termina::Rigidbody::BodyType::Kinematic;
	}
}

void CollideStatus::Update(float dt)
{
	// Le système physique gère automatiquement les callbacks
}

void CollideStatus::OnTriggerEnter(Termina::Actor* other)
{
	// Alternative si vous voulez utiliser des triggers
	if (!other || other != m_Player)
		return;

	TN_INFO("FROM wall trigger TRIGGER avec le joueur ! Game Over - '%s'", m_Name.c_str());
}