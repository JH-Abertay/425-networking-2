cmake_minimum_required(VERSION 3.28)
project(SFML_ECS_Networking LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.0
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL SYSTEM)
FetchContent_MakeAvailable(SFML)

# cmake -S . -B build -G Ninja
add_executable(${PROJECT_NAME}_server SFML-ECS-Networking/server.cpp)
add_executable(${PROJECT_NAME}_client SFML-ECS-Networking/client.cpp)

foreach(TARGET ${PROJECT_NAME}_server ${PROJECT_NAME}_client)
  target_compile_features(${TARGET} PRIVATE cxx_std_17)
  target_link_libraries(${TARGET} PRIVATE SFML::Graphics SFML::Network)
  target_include_directories(${TARGET} PRIVATE entt)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT
                                                              ${TARGET})
endforeach()
