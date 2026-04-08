#include "SideScrolling.hpp"
void SideScrolling::Start()
{
	m_Player = m_Owner->GetParentWorld()->GetActorByName("Player");

	// Get all actors named Corridor in the world and add them to m_Segments vector.
	// These are the segments that will be recycled to create the infinite scrolling effect.
	const auto& actors = m_Owner->GetParentWorld()->GetActors();
	for (const auto& actorPtr : actors)
	{
		Termina::Actor* actor = actorPtr.get();
		if (actor && actor->GetName().find("Corridor") != std::string::npos)
		{
			m_Segments.push_back(actor);
		}
	}

	TN_INFO("SideScrolling: Found %zu corridor segments", m_Segments.size());
}

void SideScrolling::Update(float deltaTime)
{
	if (!m_Player)
		return;

	CheckIfNeedReplace();
}

void SideScrolling::CheckIfNeedReplace()
{
	// If any segment is more than 50 units behind the player, move it to be right behind the last segment of the vector,
	// which is the one farthest ahead of the player. This creates an infinite scrolling effect.
	for (auto* segment : m_Segments)
	{
		if (!segment || !segment->HasComponent<Termina::Transform>())
			continue;

		const Termina::Transform& segmentTransform = segment->GetComponent<Termina::Transform>();
		const Termina::Transform& playerTransform = m_Player->GetComponent<Termina::Transform>();

		glm::vec3 segmentPos = segmentTransform.GetPosition();
		glm::vec3 playerPos = playerTransform.GetPosition();

		if (segmentPos.z + 40.0f < playerPos.z)
		{
			// Find the farthest segment ahead of the player
			float farthestZ = playerPos.z;
			for (auto* otherSegment : m_Segments)
			{
				if (otherSegment && otherSegment->HasComponent<Termina::Transform>())
				{
					glm::vec3 otherPos = otherSegment->GetComponent<Termina::Transform>().GetPosition();
					if (otherPos.z > farthestZ)
						farthestZ = otherPos.z;
				}
			}

			// Move the segment behind the farthest one
			glm::vec3 newPos = segmentPos;
			newPos.z = farthestZ + 31; // Assuming each segment is ~20 units long
			segment->GetComponent<Termina::Transform>().SetPosition(newPos);

			TN_INFO("SideScrolling: Recycled segment '%s' to Z: %.2f", segment->GetName().c_str(), newPos.z);
		}
	}
}