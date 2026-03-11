-- xmake.lua

target("Im3D")
    set_kind("static")
    add_files("**.cpp")
    add_headerfiles("**.hpp")
    add_deps("ImGui")
