#include <Termina/Scripting/API/ScriptingAPI.hpp>

using namespace TerminaScript;



class Procedural : public TerminaScript::ScriptableComponent
{
public:
    Procedural() = default;
    Procedural(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start() override;
    void Update(float dt) override;
};
