
target("Editor")
    set_kind("binary")
    set_group("Termina")

    add_files("*.cpp")
    add_headerfiles("*.hpp")
    add_includedirs(".")
    add_deps("Termina")
