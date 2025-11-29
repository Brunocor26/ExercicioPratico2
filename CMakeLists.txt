cmake_minimum_required(VERSION 3.16)
project(tp2_circle_param C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajudar CMake a encontrar libs via Homebrew (Apple Silicon)
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew /opt/homebrew/lib/cmake)

# Dependências
find_package(glfw3 REQUIRED)   # alvo: glfw
# find_package(GLEW  REQUIRED)   # GLEW removido em favor do GLAD

# Executável
add_executable(tp2
  src/main.cpp
  src/objloader.cpp
  src/glad.c
)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)

# Includes do projeto (para "common/shader.hpp") e objloader.hpp
target_include_directories(tp2 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${CMAKE_CURRENT_BINARY_DIR}/configuration
)

# GLAD (via source file included above)
target_link_libraries(tp2 PRIVATE dl) # dl is needed for glad on Linux

# GLFW
target_link_libraries(tp2 PRIVATE glfw)

# OpenGL por SO
if (APPLE)
  target_compile_definitions(tp2 PRIVATE GL_SILENCE_DEPRECATION)
  target_link_libraries(tp2 PRIVATE "-framework OpenGL")
elseif (WIN32)
  target_link_libraries(tp2 PRIVATE opengl32)
else()
  find_package(OpenGL REQUIRED)
  target_link_libraries(tp2 PRIVATE OpenGL::GL)
endif()

# Copiar shaders/ para a build
add_custom_command(TARGET tp2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_BINARY_DIR}/shaders
    COMMENT "A copiar pasta 'shaders/' para a build..."
)

# Copiar deer.obj para a diretoria de build
add_custom_command(TARGET tp2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/deer.obj
    ${CMAKE_BINARY_DIR}/deer.obj
    COMMENT "A copiar 'deer.obj' para a build..."
)

# Copiar deer.mtl para a diretoria de build
add_custom_command(TARGET tp2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/deer.mtl
    ${CMAKE_BINARY_DIR}/deer.mtl
    COMMENT "A copiar 'deer.mtl' para a build..."
)

set_target_properties(tp2 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)