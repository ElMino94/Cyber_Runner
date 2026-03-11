-- xmake.lua

target("VMA")
    set_kind("static")
    add_files("*.cpp")
    add_headerfiles("*.h")
    add_deps("Volk", "Vulkan")
