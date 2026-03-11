-- xmake.lua

target("ImGui")
    set_kind("static")
    add_files("*.cpp")
    add_headerfiles("*.h")
    add_deps("GLFW")

    add_defines("IMGUI_IMPL_GLFW_DISABLE_X11")
