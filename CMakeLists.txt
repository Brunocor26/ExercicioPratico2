cmake_minimum_required(VERSION 3.16)
project(tp2_circle_param C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajudar CMake a encontrar libs via Homebrew (Apple Silicon)
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew /opt/homebrew/lib/cmake)

# Dependências
find_package(glfw3 REQUIRED)   # alvo: glfw
find_package(GLEW  REQUIRED)   # GLEW_INCLUDE_DIRS / GLEW_LIBRARIES

# Executável
add_executable(tp2
  src/main.cpp
  src/shader.cpp
  src/objloader.cpp
)

# Includes do projeto (para "common/shader.hpp") e objloader.hpp
target_include_directories(tp2 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/common
)

# GLEW
if (TARGET GLEW::GLEW)
  target_link_libraries(tp2 PRIVATE GLEW::GLEW)
else()
  target_include_directories(tp2 PRIVATE ${GLEW_INCLUDE_DIRS})
  target_link_libraries(tp2 PRIVATE ${GLEW_LIBRARIES})
endif()

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
set(SHADERS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADERS_DST_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_command(TARGET tp2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${SHADERS_SRC_DIR} ${SHADERS_DST_DIR}
  COMMENT "A copiar pasta 'shaders/' para a build..."
)

set_target_properties(tp2 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)