-- xmake.lua

target("stb")
    set_kind("static")
    add_files("*.c")
    add_headerfiles("*.h")
