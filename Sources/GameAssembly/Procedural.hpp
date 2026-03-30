#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <vector>
using namespace TerminaScript;



class Procedural : public TerminaScript::ScriptableComponent
{
private:
	std::vector<Termina::Actor*> m_Objects;
	std::vector<Termina::Actor*> m_ObjectsToDestroy;
	int m_ObjectCount = 0;
	int m_MaxObjects = 5;
	int m_ObjectsSpeed = 1;

public:
    Procedural() = default;
    Procedural(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start() override;
    void Update(float dt) override;



	void ObjectsUpdate(float dt);
	void procéduralGeneration();
	void DestroyObjects();
	void DestroyObjectsUpdate();
	





	int GetObjectCount() const { return m_ObjectCount; }
	int GetMaxObjects() const { return m_MaxObjects; }
	int GetObjectsSpeed() const { return m_ObjectsSpeed; }

	void SetMaxObjects(int maxObjects) { m_MaxObjects = maxObjects; }
	void SetObjectsSpeed(int speed) { m_ObjectsSpeed = speed; }
	void UpdateObjectCount(int count) { m_ObjectCount = count; }
};
