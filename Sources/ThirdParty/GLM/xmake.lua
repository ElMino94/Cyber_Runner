-- xmake.lua

target("GLM")
    set_kind("headeronly")
    add_headerfiles("**.hpp", "**.inl")
