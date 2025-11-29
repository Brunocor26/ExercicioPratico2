#include "Mesh.hpp"
#include "objloader.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/camera.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/transform.h>
#include <vector>

// Camera setup
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 900.0f / 2.0;
float lastY = 600.0f / 2.0;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Store all user input states (mouse, keyboard)
struct InputState {
  float lightRotationSpeed = 1.0f; // Speed of light orbiting
  bool lightPaused = false;        // Is light rotation paused?
  float lightAngle = 0.0f;         // Current light position angle
  bool wireframe = false;          // Show wireframe mode?
  bool blinn = false;              // Blinn-Phong lighting?

  // Store if key was pressed (prevents repeated triggers)
  bool spacePressed = false;
  bool plusPressed = false;
  bool minusPressed = false;
  bool rPressed = false;
  bool fPressed = false;
  bool bPressed = false;
};

// Transform for the model
Transform modelTransform;

// Read keyboard and mouse input and update the InputState
void processInput(GLFWwindow *window, InputState &input) {
  // Close window if ESC is pressed
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Camera movement
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);

  // Model rotation with arrow keys (using Transform)
  glm::vec3 currentRot = modelTransform.getLocalRotation();
  float rotSpeed = 50.0f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    currentRot.x -= rotSpeed;
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    currentRot.x += rotSpeed;
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    currentRot.y -= rotSpeed;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    currentRot.y += rotSpeed;
  modelTransform.setLocalRotation(currentRot);

  // Pause/unpause light rotation with SPACE
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    if (!input.spacePressed) {
      input.lightPaused = !input.lightPaused;
      std::printf("Light rotation: %s\n",
                  input.lightPaused ? "PAUSED" : "RUNNING");
      input.spacePressed = true;
    }
  } else {
    input.spacePressed = false;
  }

  // Increase light rotation speed with + key
  bool plus = (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
               glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS);
  if (plus) {
    if (!input.plusPressed) {
      input.lightRotationSpeed += 0.5f;
      std::printf("Light rotation speed: %.1f\n", input.lightRotationSpeed);
      input.plusPressed = true;
    }
  } else {
    input.plusPressed = false;
  }

  // Decrease light rotation speed with - key
  bool minus = (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS);
  if (minus) {
    if (!input.minusPressed) {
      input.lightRotationSpeed =
          std::max(0.0f, input.lightRotationSpeed - 0.5f);
      std::printf("Light rotation speed: %.1f\n", input.lightRotationSpeed);
      input.minusPressed = true;
    }
  } else {
    input.minusPressed = false;
  }

  // Toggle wireframe mode with F key
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    if (!input.fPressed) {
      input.wireframe = !input.wireframe;
      if (input.wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      std::printf("Wireframe: %s\n", input.wireframe ? "ON" : "OFF");
      input.fPressed = true;
    }
  } else {
    input.fPressed = false;
  }

  // Toggle Blinn-Phong with B key
  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
    if (!input.bPressed) {
      input.blinn = !input.blinn;
      std::printf("Blinn-Phong: %s\n", input.blinn ? "ON" : "OFF");
      input.bPressed = true;
    }
  } else {
    input.bPressed = false;
  }

  // Reset everything to initial state with R key
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    if (!input.rPressed) {
      input.lightRotationSpeed = 1.0f;
      input.lightPaused = false;
      input.lightAngle = 0.0f;
      input.wireframe = false;
      input.blinn = false;
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));     // Reset camera
      modelTransform.setLocalRotation(glm::vec3(0.0f)); // Reset rotation
      std::printf("State reset: Position, Rotation, Light, Wireframe, Blinn\n");
      input.rPressed = true;
    }
  } else {
    input.rPressed = false;
  }
}

// Mouse callback
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset =
      lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// Scroll callback
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(yoffset);
}

// Create and setup OpenGL window with GLFW and GLAD
GLFWwindow *initWindow(int width, int height, const char *title) {
  if (!glfwInit()) {
    std::fprintf(stderr, "GLFW init falhou\n");
    return nullptr;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!win) {
    std::fprintf(stderr, "Falha a criar janela\n");
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(win);
  glfwSetCursorPosCallback(win, mouse_callback);
  glfwSetScrollCallback(win, scroll_callback);

  // Tell GLFW to capture our mouse
  glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::fprintf(stderr, "Failed to initialize GLAD\n");
    return nullptr;
  }

  glfwSetFramebufferSizeCallback(
      win, [](GLFWwindow *, int w, int h) { glViewport(0, 0, w, h); });

  return win;
}

// Load deer 3D model: read OBJ file, load material from MTL file,
// calculate size and position, return Mesh object
Mesh *setupDeerMesh(const char *filename, float &baseScale, glm::vec3 &center,
                    Material &outMaterial) {
  std::string fullPath = FileSystem::getPath(filename);
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  bool res = loadOBJ(fullPath.c_str(), positions, normals);
  if (!res) {
    std::fprintf(stderr, "Impossível abrir %s ou processá-lo\n",
                 fullPath.c_str());
    return nullptr;
  }

  // Try to load material file (.mtl) that matches the .obj file
  std::string mtlPath = fullPath;
  size_t dotPos = mtlPath.find_last_of('.');
  if (dotPos != std::string::npos) {
    mtlPath = mtlPath.substr(0, dotPos) + ".mtl";
  }
  std::map<std::string, Material> materials;
  if (loadMTL(mtlPath.c_str(), materials)) {
    // Usar o primeiro material encontrado
    if (!materials.empty()) {
      outMaterial = materials.begin()->second;
      std::printf("Material carregado de %s\n", mtlPath.c_str());
    }
  } else {
    std::printf("Ficheiro .mtl não encontrado em %s, usando material padrão\n",
                mtlPath.c_str());
    outMaterial = {glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.6f, 0.6f, 0.6f),
                   glm::vec3(0.9f, 0.9f, 0.9f), 32.0f, 1.0f};
  }

  std::vector<Vertex> vertices;
  for (size_t i = 0; i < positions.size(); ++i) {
    Vertex v;
    v.Position = positions[i];
    if (i < normals.size())
      v.Normal = normals[i];
    else
      v.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
    vertices.push_back(v);
  }

  glm::vec3 minb(FLT_MAX), maxb(-FLT_MAX);
  for (auto &v : positions) {
    minb = glm::min(minb, v);
    maxb = glm::max(maxb, v);
  }
  center = (minb + maxb) * 0.5f;
  glm::vec3 diag = maxb - minb;
  float extent = std::max(diag.x, std::max(diag.y, diag.z));
  if (extent <= 0.0f)
    extent = 1.0f;
  baseScale = 1.0f / extent;

  // Empty indices because loadOBJ returns individual triangles, not indexed
  // geometry
  std::vector<unsigned int> indices;
  return new Mesh(vertices, indices);
}

// Create a small box to show the light source position
GLuint setupLightBox(size_t &indexCount) {
  // 8 vertices of a cube (size 0.1 x 0.1 x 0.1)
  std::vector<float> boxVerts = {
      // Front face
      -0.05f,
      -0.05f,
      0.05f,
      0.05f,
      -0.05f,
      0.05f,
      0.05f,
      0.05f,
      0.05f,
      -0.05f,
      0.05f,
      0.05f,
      // Back face
      -0.05f,
      -0.05f,
      -0.05f,
      0.05f,
      -0.05f,
      -0.05f,
      0.05f,
      0.05f,
      -0.05f,
      -0.05f,
      0.05f,
      -0.05f,
  };

  // 36 indices (6 faces × 6 indices per face = 2 triangles per face)
  std::vector<unsigned int> boxIndices = {
      // Front face
      0,
      1,
      2,
      2,
      3,
      0,
      // Back face
      5,
      4,
      7,
      7,
      6,
      5,
      // Top face
      3,
      2,
      6,
      6,
      7,
      3,
      // Bottom face
      4,
      5,
      1,
      1,
      0,
      4,
      // Right face
      1,
      5,
      6,
      6,
      2,
      1,
      // Left face
      4,
      0,
      3,
      3,
      7,
      4,
  };

  GLuint lightVAO = 0, lightVBO = 0, lightEBO = 0;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);
  glGenBuffers(1, &lightVBO);
  glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
  glBufferData(GL_ARRAY_BUFFER, boxVerts.size() * sizeof(float),
               boxVerts.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glGenBuffers(1, &lightEBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               boxIndices.size() * sizeof(unsigned int), boxIndices.data(),
               GL_STATIC_DRAW);

  indexCount = boxIndices.size();
  return lightVAO;
}

int main() {
  // initialize window
  GLFWwindow *win = initWindow(900, 600, "TP2 - Rendering .obj file");
  if (!win) // treat if error creating window
    return -1;

  float baseScale = 1.0f; // How much to scale the model
  glm::vec3 center(0.0f); // Center point of the model

  // Object to store material properties (Ka, Kd, Ks, Ns)
  Material deerMaterial;

  // Load the deer 3D model from file
  Mesh *deerMesh = setupDeerMesh("deer.obj", baseScale, center, deerMaterial);
  if (!deerMesh) {
    glfwTerminate();
    return -1;
  }

  // Compile and link Phong shader program for realistic lighting using
  // LearnOpenGL Shader class
  Shader phongShader(FileSystem::getPath("shaders/phong.vert").c_str(),
                     FileSystem::getPath("shaders/phong.frag").c_str());
  Shader lightShader(FileSystem::getPath("shaders/simple.vert").c_str(),
                     FileSystem::getPath("shaders/simple.frag").c_str());

  // State for inputs
  InputState input;

  size_t lightIndexCount = 0;
  GLuint lightVAO = setupLightBox(lightIndexCount);

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray background

  glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D effect

  // Configure model transform
  modelTransform.setLocalScale(glm::vec3(baseScale));
  modelTransform.setLocalPosition(-center * baseScale); // Center the model
  modelTransform.computeModelMatrix();

  while (!glfwWindowShouldClose(win)) {
    // Per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Read user input
    processInput(win, input);

    // Clear screen for next frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update light position angle if not paused
    if (!input.lightPaused) {
      input.lightAngle += input.lightRotationSpeed * 0.01f;
    }

    // Setup camera and projection (handles window resize)
    int fbw, fbh;
    glfwGetFramebufferSize(win, &fbw, &fbh);
    float aspect = (fbh == 0) ? 1.0f : (float)fbw / (float)fbh;

    // Use Camera class for view matrix
    glm::mat4 view = camera.GetViewMatrix();
    // Create perspective projection (45 degree field of view)
    glm::mat4 proj =
        glm::perspective(glm::radians(camera.Zoom), aspect, 0.01f, 100.0f);

    // --- Draw Deer ---
    // Update model matrix from Transform class
    modelTransform.computeModelMatrix();
    glm::mat4 model = modelTransform.getModelMatrix();

    // Calculate transformation matrices for shader
    glm::mat4 modelView = view * model;
    // Model-View-Projection matrix
    glm::mat4 MVP = proj * modelView;
    // Normal matrix for lighting calculations
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelView)));

    // Activate shader
    phongShader.use();

    // Send matrices to shader
    phongShader.setMat4("ModelViewMatrix", modelView);
    phongShader.setMat4("MVP", MVP);
    phongShader.setMat3("NormalMatrix", normalMatrix);

    // Calculate light position orbiting around the model
    float lightRadius = 2.0f; // Distance from center
    glm::vec3 lightPos(lightRadius * std::cos(input.lightAngle),
                       2.0f, // Higher light for better floor shadow/lighting
                       lightRadius * std::sin(input.lightAngle));
    // Convert light position to camera space
    glm::vec4 lightPosEye = view * glm::vec4(lightPos, 1.0f);

    phongShader.setVec4("Light.Position", lightPosEye);
    phongShader.setVec3("Light.La", 0.1f, 0.1f, 0.1f);
    phongShader.setVec3("Light.Ld", 0.8f, 0.8f, 0.8f);
    phongShader.setVec3("Light.Ls", 1.0f, 1.0f, 1.0f);

    // Send material values to shader (loaded from .mtl or default)
    phongShader.setVec3("Material.Ka", deerMaterial.Ka);
    phongShader.setVec3("Material.Kd", deerMaterial.Kd);
    phongShader.setVec3("Material.Ks", deerMaterial.Ks);
    phongShader.setFloat("Material.Shininess", deerMaterial.Ns);

    // Blinn-Phong toggle
    phongShader.setBool("blinn", input.blinn);

    deerMesh->Draw(phongShader.ID);

    // Draw light source as small yellow sphere
    lightShader.use();
    glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);
    glm::mat4 lightMVP = proj * view * lightModel;
    lightShader.setMat4("MVP", lightMVP);
    lightShader.setVec3("LightColor", 1.0f, 1.0f, 0.0f); // Yellow color

    glBindVertexArray(lightVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)lightIndexCount, GL_UNSIGNED_INT, 0);

    // Display rendered image on screen
    glfwSwapBuffers(win);
    // Handle window events (close, resize, etc.)
    glfwPollEvents();
  }

  // Clean up memory
  delete deerMesh;
  // Close OpenGL window and cleanup
  glfwTerminate();
}