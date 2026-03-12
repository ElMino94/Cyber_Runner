#pragma once

#include <vector>
#include <string>

namespace Termina {
    class LaunchProcess
    {
    public:
        static int Launch(const std::string& executable, const std::vector<std::string>& arguments);
    };
}
