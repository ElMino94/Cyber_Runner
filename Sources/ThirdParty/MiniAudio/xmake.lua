-- xmake.lua

target("MiniAudio")
    set_kind("static")
    add_files("*.c")
    add_headerfiles("*.h")
