-- xmake.lua

target("Volk")
    set_kind("static")
    add_files("*.c")
    add_headerfiles("*.h")
    add_deps("Vulkan")
