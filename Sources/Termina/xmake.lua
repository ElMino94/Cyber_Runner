-- xmake.lua

target("Termina")
    set_kind("static")
    set_group("Termina")

    add_files("*.cpp")
    add_headerfiles("*.hpp")
    add_includedirs(".")
    add_deps(
        "CGLTF",
        "GLFW",
        "GLM",
        "Im3D",
        "ImGui",
        "Jolt",
        "JSON",
        "MikkTSpace",
        "MiniAudio",
        "stb",
        "VMA",
        "Volk",
        "Vulkan"
    )
