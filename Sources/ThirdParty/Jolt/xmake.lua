-- xmake.lua

target("Jolt")
    set_kind("static")
    add_files("**.cpp")
    add_headerfiles("**.hpp")
