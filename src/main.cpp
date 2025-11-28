
#include "Mesh.hpp"
#include "objloader.hpp"
#include "shader.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// Estrutura que guarda o estado dos inputs
struct InputState {
  bool rotating = false;
  double lastX = 0, lastY = 0;
  float yaw = 0.0f, pitch = 0.0f;
  glm::vec3 trans = glm::vec3(0.0f);
  float lightRotationSpeed = 1.0f;
  bool lightPaused = false;
  float lightAngle = 0.0f;
  bool wireframe = false;

  // Estado para os botões (toggle)
  bool spacePressed = false;
  bool plusPressed = false;
  bool minusPressed = false;
  bool rPressed = false;
  bool fPressed = false;
};

// Processa os inputs do utilizador (teclado e rato)
void processInput(GLFWwindow *window, InputState &input) {
  // Sair com ESC
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Rotação com o Rato
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (!input.rotating) {
      input.lastX = x;
      input.lastY = y;
      input.rotating = true;
    }
    double dx = x - input.lastX;
    double dy = y - input.lastY;
    input.yaw += float(dx) * 0.005f;
    input.pitch += float(dy) * 0.005f;
    input.lastX = x;
    input.lastY = y;
  } else {
    input.rotating = false;
  }

  // Movimento da câmera
  float step = 0.05f;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    input.trans.y += step;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    input.trans.y -= step;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    input.trans.x -= step;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    input.trans.x += step;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
    input.trans.z += step;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
    input.trans.z -= step;

  // Pausar Luz (Espaço)
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    if (!input.spacePressed) {
      input.lightPaused = !input.lightPaused;
      std::printf("Rotação da luz: %s\n",
                  input.lightPaused ? "PARADA" : "A RODAR");
      input.spacePressed = true;
    }
  } else {
    input.spacePressed = false;
  }

  // Aumentar Velocidade da Luz (+)
  bool plus = (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
               glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS);
  if (plus) {
    if (!input.plusPressed) {
      input.lightRotationSpeed += 0.5f;
      std::printf("Velocidade de rotação da luz: %.1f\n",
                  input.lightRotationSpeed);
      input.plusPressed = true;
    }
  } else {
    input.plusPressed = false;
  }

  // Diminuir Velocidade da Luz (-)
  bool minus = (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS);
  if (minus) {
    if (!input.minusPressed) {
      input.lightRotationSpeed =
          std::max(0.0f, input.lightRotationSpeed - 0.5f);
      std::printf("Velocidade de rotação da luz: %.1f\n",
                  input.lightRotationSpeed);
      input.minusPressed = true;
    }
  } else {
    input.minusPressed = false;
  }

  // Alternar Wireframe (F)
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

  // Reiniciar Estado (R)
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    if (!input.rPressed) {
      input.lightRotationSpeed = 1.0f;
      input.trans = glm::vec3(0.0f);
      input.yaw = 0.0f;
      input.pitch = 0.0f;
      input.lightPaused = false;
      input.lightAngle = 0.0f;
      input.wireframe = false;
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      std::printf("Estado reiniciado: Posição, Rotação, Luz, Wireframe\n");
      input.rPressed = true;
    }
  } else {
    input.rPressed = false;
  }
}

// inicializa a janela
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

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::fprintf(stderr, "glewInit falhou\n");
    return nullptr;
  }
  glGetError();

  glfwSetFramebufferSizeCallback(
      win, [](GLFWwindow *, int w, int h) { glViewport(0, 0, w, h); });

  return win;
}

// Carrega o modelo 3D do veado e retorna um objeto Mesh
Mesh *setupDeerMesh(const char *filename, float &baseScale, glm::vec3 &center) {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  bool res = loadOBJ(filename, positions, uvs, normals);
  if (!res) {
    std::fprintf(stderr, "Impossível abrir %s ou processá-lo\n", filename);
    return nullptr;
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

  // Usamos índices vazios por enquanto, pois loadOBJ retorna triângulos
  // independentes
  std::vector<unsigned int> indices;
  return new Mesh(vertices, indices);
}

// Cria uma esfera para representar a luz
GLuint setupLightSphere(size_t &indexCount) {
  std::vector<float> lightSphereVerts;
  const int slices = 10, stacks = 10;
  const float lightSphereRadius = 0.05f;
  for (int i = 0; i <= stacks; ++i) {
    float phi = M_PI * float(i) / float(stacks);
    for (int j = 0; j <= slices; ++j) {
      float theta = 2.0f * M_PI * float(j) / float(slices);
      float x = lightSphereRadius * std::sin(phi) * std::cos(theta);
      float y = lightSphereRadius * std::cos(phi);
      float z = lightSphereRadius * std::sin(phi) * std::sin(theta);
      lightSphereVerts.push_back(x);
      lightSphereVerts.push_back(y);
      lightSphereVerts.push_back(z);
    }
  }

  std::vector<unsigned int> lightSphereIndices;
  for (int i = 0; i < stacks; ++i) {
    for (int j = 0; j < slices; ++j) {
      int first = i * (slices + 1) + j;
      int second = first + slices + 1;
      lightSphereIndices.push_back(first);
      lightSphereIndices.push_back(second);
      lightSphereIndices.push_back(first + 1);
      lightSphereIndices.push_back(second);
      lightSphereIndices.push_back(second + 1);
      lightSphereIndices.push_back(first + 1);
    }
  }

  GLuint lightVAO = 0, lightVBO = 0, lightEBO = 0;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);
  glGenBuffers(1, &lightVBO);
  glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
  glBufferData(GL_ARRAY_BUFFER, lightSphereVerts.size() * sizeof(float),
               lightSphereVerts.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glGenBuffers(1, &lightEBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               lightSphereIndices.size() * sizeof(unsigned int),
               lightSphereIndices.data(), GL_STATIC_DRAW);

  indexCount = lightSphereIndices.size();
  return lightVAO;
}

int main() {
  GLFWwindow *win = initWindow(900, 600, "TP2 - Rendering .obj file");
  if (!win)
    return -1;

  float baseScale = 1.0f;
  glm::vec3 center(0.0f);
  Mesh *deerMesh = setupDeerMesh("deer.obj", baseScale, center);
  if (!deerMesh) {
    glfwTerminate();
    return -1;
  }

  // Shaders (usar Phong)
  GLuint prog =
      linkProgramFromFiles("shaders/phong.vert", "shaders/phong.frag");
  if (!prog) {
    std::fprintf(stderr, "Shaders não compilados\n");
    return -1;
  }
  glUseProgram(prog);

  // Localização dos Uniforms do shader Phong
  GLint uModelView = getUniform(prog, "ModelViewMatrix");
  GLint uNormalMat = getUniform(prog, "NormalMatrix");
  // Nota: 'ProjectionMatrix' é declarado no shader mas não usado; ignoramos
  // para evitar avisos.
  GLint uMVP = getUniform(prog, "MVP");

  // Membros da struct Light
  GLint uLight_Position = getUniform(prog, "Light.Position");
  GLint uLight_La = getUniform(prog, "Light.La");
  GLint uLight_Ld = getUniform(prog, "Light.Ld");
  GLint uLight_Ls = getUniform(prog, "Light.Ls");

  // Membros da struct Material
  GLint uMat_Ka = getUniform(prog, "Material.Ka");
  GLint uMat_Kd = getUniform(prog, "Material.Kd");
  GLint uMat_Ks = getUniform(prog, "Material.Ks");
  GLint uMat_Shininess = getUniform(prog, "Material.Shininess");

  // Definir material (estático)
  glUniform3f(uMat_Ka, 0.2f, 0.2f, 0.2f);
  glUniform3f(uMat_Kd, 0.6f, 0.6f, 0.6f);
  glUniform3f(uMat_Ks, 0.9f, 0.9f, 0.9f);
  glUniform1f(uMat_Shininess, 32.0f);

  // Definir luz ambiente/difusa/especular (intensidades estáticas)
  glUniform3f(uLight_La, 0.1f, 0.1f, 0.1f);
  glUniform3f(uLight_Ld, 0.8f, 0.8f, 0.8f);
  glUniform3f(uLight_Ls, 1.0f, 1.0f, 1.0f);

  // Estado dos inputs para interação
  InputState input;

  // Tempo para rotação da luz

  // Criar um shader simples para desenhar a fonte de luz
  GLuint lightProg =
      linkProgramFromFiles("shaders/simple.vert", "shaders/simple.frag");
  if (!lightProg) {
    std::fprintf(stderr, "Shaders não compilados\n");
    return -1;
  }
  GLint uLightMVP = getUniform(lightProg, "MVP");
  GLint uLightColor = getUniform(lightProg, "LightColor");

  size_t lightIndexCount = 0;
  GLuint lightVAO = setupLightSphere(lightIndexCount);

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Cor de fundo

  glEnable(GL_DEPTH_TEST); // Necessário para profundidade

  while (!glfwWindowShouldClose(win)) {
    processInput(win, input);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Atualizar ângulo da luz se não estiver pausada
    if (!input.lightPaused) {
      input.lightAngle +=
          input.lightRotationSpeed * 0.01f; // Ajustar velocidade
    }

    // Atualizar câmera/projeção a cada frame (lida com redimensionamento)
    int fbw, fbh;
    glfwGetFramebufferSize(win, &fbw, &fbh);
    float aspect = (fbh == 0) ? 1.0f : (float)fbw / (float)fbh;
    glm::vec3 camPos(0.0f, 0.0f, 2.5f);
    glm::mat4 view =
        glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj =
        glm::perspective(glm::radians(45.0f), aspect, 0.01f, 100.0f);

    // Construir transformação do modelo com base nos inputs
    glm::mat4 model(1.0f);
    // Translação controlada pelo utilizador
    model = glm::translate(model, input.trans);
    // Rotação (espera radianos)
    model = glm::rotate(model, input.yaw, glm::vec3(0, 1, 0));
    model = glm::rotate(model, input.pitch, glm::vec3(1, 0, 0));
    // Escala para tamanho normalizado e centralizar
    model = glm::scale(model, glm::vec3(baseScale));
    model = glm::translate(model, -center);

    // Matrizes para o shader
    glm::mat4 modelView = view * model;
    glm::mat4 MVP = proj * modelView;
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelView)));
    glUniformMatrix4fv(uModelView, 1, GL_FALSE, glm::value_ptr(modelView));
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix3fv(uNormalMat, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // Luz pontual rodando em círculo à volta do veado
    float lightRadius = 2.0f; // Distância da origem
    glm::vec3 lightPos(lightRadius * std::cos(input.lightAngle),
                       1.0f, // Altura fixa
                       lightRadius * std::sin(input.lightAngle));
    glm::vec4 lightPosEye =
        view * glm::vec4(lightPos, 1.0f); // Converter para coordenadas de olho
    glUniform4f(uLight_Position, lightPosEye.x, lightPosEye.y, lightPosEye.z,
                lightPosEye.w);

    deerMesh->Draw(prog);

    // Desenhar a fonte de luz como uma pequena esfera
    glUseProgram(lightProg);
    glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);
    glm::mat4 lightMVP = proj * view * lightModel;
    glUniformMatrix4fv(uLightMVP, 1, GL_FALSE, glm::value_ptr(lightMVP));
    glUniform3f(uLightColor, 1.0f, 1.0f, 0.0f); // Cor amarela/brilhante
    glBindVertexArray(lightVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)lightIndexCount, GL_UNSIGNED_INT, 0);

    // Voltar ao shader Phong para o próximo frame
    glUseProgram(prog);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  delete deerMesh;
  glfwTerminate();
}
