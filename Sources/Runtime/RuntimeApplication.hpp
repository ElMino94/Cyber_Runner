#pragma once

#include <Termina/Core/Application.hpp>

class RuntimeApplication : public Termina::Application
{
public:
    RuntimeApplication();
    ~RuntimeApplication() = default;

    void OnPreUpdate(float dt) override;
};
