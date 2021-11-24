set_allowedarchs("x64")
set_languages("c99", "c++20")
add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    add_defines("DEBUG")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize(fastest)
end

set_optimize("fastest")
packages = {"glfw", "glm", "spdlog", "yaml-cpp", "fmt", "tinyobjloader", "tinygltf", "entt", "stb"}

add_requires(packages)

target("Vendor")
    set_kind("static")
    add_files("Vendor/*/*.cpp")
    add_headerfiles("Vendor/*/*.h")
    add_packages(packages)
target_end()

target("Engine")
    set_kind("static")
    add_files("Engine/*.cpp","Engine/*/*.cpp","Engine/*/*/*.cpp")
    add_headerfiles("Engine/*.h", "Engine/*/*.h","Engine/*/*/*.h")
    add_packages(packages)
    add_deps("Vendor")
target_end()

target("EngineEditor")
    set_kind("binary")
    add_files("EngineEditor/*.cpp", "EngineEditor/*/*.cpp")
    add_headerfiles("EngineEditor/*.h", "EngineEditor/*/*.h")
    set_optimize("fastest")
    add_deps("Engine")
    add_deps("Vendor")
    add_packages(packages)
target_end()
