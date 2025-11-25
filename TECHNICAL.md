# Documentação Técnica - TP2 Rendering Engine

Este documento explica em detalhe a implementação do código e como funciona internamente o motor de renderização 3D.

---

## 🏗️ Arquitetura Geral

### Fluxo do Programa

```
1. Inicialização GLFW/GLEW
2. Carregamento do modelo .obj
3. Criação de buffers (VAO/VBO)
4. Compilação de shaders
5. Configuração de uniforms
6. Loop de renderização
7. Terminação
```

---

## 📐 1. Carregamento e Normalização do Modelo

### Carregamento OBJ (linhas 51-59)

```cpp
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;
bool res = loadOBJ("deer.obj", vertices, uvs, normals);
```

A função `loadOBJ()` (implementada em `objloader.cpp`) lê o ficheiro `.obj` e extrai:
- **Vértices**: Posições 3D de cada ponto do modelo
- **UV coordinates**: Coordenadas de textura (não utilizadas neste projeto)
- **Normais**: Vetores perpendiculares às superfícies (essenciais para iluminação)

### Buffer Interleaved (linhas 62-70)

```cpp
std::vector<float> interleaved;
for (size_t i = 0; i < vertices.size(); ++i) {
    const glm::vec3 &p = vertices[i];
    glm::vec3 n = normals[i];
    // pos.x, pos.y, pos.z, normal.x, normal.y, normal.z
    interleaved.push_back(p.x); ... interleaved.push_back(n.z);
}
```

**Porquê interleaved?**
- Melhor performance de cache: dados relacionados ficam adjacentes na memória
- Formato: `[x, y, z, nx, ny, nz, x, y, z, nx, ny, nz, ...]`
- Cada vértice ocupa 6 floats (3 posição + 3 normal)

### Normalização do Modelo (linhas 73-82)

```cpp
glm::vec3 minb(FLT_MAX), maxb(-FLT_MAX);
for (auto &v : vertices) {
    minb = glm::min(minb, v);
    maxb = glm::max(maxb, v);
}
glm::vec3 center = (minb + maxb) * 0.5f;
float extent = std::max(diag.x, std::max(diag.y, diag.z));
float baseScale = 1.0f / extent;
```

**Objetivo**: Garantir que qualquer modelo carregado fica visível e centrado

1. Calcula **bounding box**: caixa mínima que contém todo o modelo
2. Calcula **centro**: ponto médio da bounding box
3. Calcula **extent**: maior dimensão do modelo
4. Calcula **escala**: fator para normalizar o modelo para tamanho ~1 unidade

Aplicação posterior no loop (linhas 198-199):
```cpp
model = glm::scale(model, glm::vec3(baseScale));
model = glm::translate(model, -center);
```

---

## 🎨 2. Configuração OpenGL

### Vertex Array Object (VAO) e Buffers (linhas 84-95)

```cpp
GLuint vao, vbo;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), 
             interleaved.data(), GL_STATIC_DRAW);
```

**VAO (Vertex Array Object)**:
- Armazena a configuração de como os dados dos vértices são organizados
- Permite trocar rapidamente entre diferentes configurações de vértices

**VBO (Vertex Buffer Object)**:
- Buffer que armazena os dados dos vértices na GPU
- `GL_STATIC_DRAW`: primitiva que indica que os dados não mudam (otimização)

### Atributos dos Vértices (linhas 91-95)

```cpp
// Posição (location = 0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// Normal (location = 1)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 
                      (void*)(3*sizeof(float)));
glEnableVertexAttribArray(1);
```

**Parâmetros de `glVertexAttribPointer`**:
- `0` ou `1`: índice do atributo (corresponde ao `location` no shader)
- `3`: número de componentes (x, y, z)
- `GL_FLOAT`: tipo de dados
- `GL_FALSE`: não normalizar
- `6*sizeof(float)`: stride (distância entre vértices consecutivos)
- `(void*)0` ou `(void*)(3*sizeof(float))`: offset do primeiro componente

---

## 🎭 3. Sistema de Shaders

### Shader Phong (Modelo Principal)

#### Vertex Shader (`phong.vert`)

**Estruturas de dados**:
```glsl
struct LightInfo {
    vec4 Position;  // Posição em eye coords
    vec3 La, Ld, Ls; // Ambiente, Difusa, Especular
};

struct MaterialInfo {
    vec3 Ka, Kd, Ks; // Reflectividade
    float Shininess; // Brilho especular
};
```

**Cálculo de Iluminação Phong** (linhas 31-43):

```glsl
vec3 tnorm = normalize(NormalMatrix * VertexNormal);
vec4 eyeCoords = ModelViewMatrix * vec4(VertexPosition, 1.0);
vec3 s = normalize(vec3(Light.Position - eyeCoords)); // vetor para a luz
vec3 v = normalize(-eyeCoords.xyz);                    // vetor para a câmera
vec3 r = reflect(-s, tnorm);                          // reflexão
```

**Componentes de iluminação**:
1. **Ambiente**: `Light.La * Material.Ka` - iluminação base constante
2. **Difusa**: `Light.Ld * Material.Kd * max(dot(s, tnorm), 0)` - baseada no ângulo
3. **Especular**: `Light.Ls * Material.Ks * pow(max(dot(r,v), 0), Shininess)` - reflexos

#### Fragment Shader (`phong.frag`)

Extremamente simples - apenas passa a cor calculada no vertex shader:
```glsl
FragColor = vec4(LightIntensity, 1.0);
```

### Shader Simples (Esfera da Luz)

#### Vertex Shader (`simple.vert`)
```glsl
gl_Position = MVP * vec4(VertexPosition, 1.0);
```
Apenas transforma a posição - sem iluminação.

#### Fragment Shader (`simple.frag`)
```glsl
FragColor = vec4(LightColor, 1.0);
```
Cor sólida uniforme (amarelo brilhante).

---

## 💡 4. Sistema de Iluminação Dinâmica

### Geração da Esfera de Luz (linhas 153-179)

```cpp
const int slices = 10, stacks = 10;
const float lightSphereRadius = 0.05f;

for (int i = 0; i <= stacks; ++i) {
    float phi = M_PI * float(i) / float(stacks);      // ângulo vertical
    for (int j = 0; j <= slices; ++j) {
        float theta = 2.0f * M_PI * float(j) / float(slices); // ângulo horizontal
        float x = lightSphereRadius * std::sin(phi) * std::cos(theta);
        float y = lightSphereRadius * std::cos(phi);
        float z = lightSphereRadius * std::sin(phi) * std::sin(theta);
    }
}
```

**Coordenadas esféricas**:
- `phi` (φ): ângulo de 0 a π (polo norte a polo sul)
- `theta` (θ): ângulo de 0 a 2π (à volta do equador)
- Conversão para cartesianas: `(r*sin(φ)*cos(θ), r*cos(φ), r*sin(φ)*sin(θ))`

### Rotação da Luz (linhas 254-257, 284-290)

```cpp
// Atualização do ângulo
if (!input.lightPaused) {
    lightAngle += input.lightRotationSpeed * 0.01f;
}

// Cálculo da posição
float lightRadius = 2.0f;
glm::vec3 lightPos(
    lightRadius * std::cos(lightAngle),  // movimento em X
    1.0f,                                 // altura fixa
    lightRadius * std::sin(lightAngle)    // movimento em Z
);
```

**Movimento circular**:
- Raio constante de 2.0 unidades
- Altura Y fixa em 1.0
- X e Z variam ciclicamente usando cos/sin
- Resulta em órbita circular horizontal ao redor do veado

---

## 🎮 5. Sistema de Input

### Estado de Input (linha 136)

```cpp
struct InputState {
    bool rotating;           // mouse está a arrastar?
    double lastX, lastY;     // última posição do mouse
    float yaw, pitch;        // rotações acumuladas
    glm::vec3 trans;         // translação acumulada
    float lightRotationSpeed; // velocidade da luz
    bool lightPaused;        // luz em pausa?
} input;
```

### Callbacks GLFW

#### Mouse (linhas 199-213)

**Movimento do cursor**:
```cpp
glfwSetCursorPosCallback(win, [](GLFWwindow *w, double x, double y){
    if (s->rotating) {
        double dx = x - s->lastX;
        double dy = y - s->lastY;
        s->yaw += float(dx) * 0.005f;    // sensibilidade horizontal
        s->pitch += float(dy) * 0.005f;  // sensibilidade vertical
    }
});
```

**Botão do mouse**:
```cpp
glfwSetMouseButtonCallback(win, [](GLFWwindow *w, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        s->rotating = (action == GLFW_PRESS);
    }
});
```

#### Teclado (linhas 217-243)

**Pausa (SPACE)**:
```cpp
if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
    s->lightPaused = !s->lightPaused;  // toggle
}
```

**Velocidade da luz (+/-/R)**:
```cpp
if (key == GLFW_KEY_EQUAL) s->lightRotationSpeed += 0.5f;
if (key == GLFW_KEY_MINUS) s->lightRotationSpeed = max(0.0f, s->lightRotationSpeed - 0.5f);
if (key == GLFW_KEY_R) s->lightRotationSpeed = 1.0f;
```

---

## 🔄 6. Loop de Renderização

### Estrutura (linhas 248-311)

```cpp
while (!glfwWindowShouldClose(win)) {
    1. Limpar framebuffer
    2. Atualizar ângulo da luz
    3. Calcular matrizes (view, projection)
    4. Construir matriz model
    5. Calcular posição da luz
    6. Renderizar veado (Phong)
    7. Renderizar esfera da luz (Simple)
    8. Swap buffers
    9. Poll events
}
```

### Transformações (Matrizes)

#### View Matrix (linha 262)
```cpp
glm::vec3 camPos(0.0f, 0.0f, 2.5f);
glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
```
- Câmera na posição (0, 0, 2.5)
- A olhar para a origem (0, 0, 0)
- "Up" vector: (0, 1, 0)

#### Projection Matrix (linha 263)
```cpp
glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 100.0f);
```
- FOV: 45 graus
- Aspect ratio dinâmico (adapta ao resize)
- Near plane: 0.01
- Far plane: 100.0

#### Model Matrix (linhas 267-274)
```cpp
glm::mat4 model(1.0f);
model = glm::translate(model, input.trans);      // 1. translação do user
model = glm::rotate(model, input.yaw, Y_AXIS);   // 2. rotação horizontal
model = glm::rotate(model, input.pitch, X_AXIS); // 3. rotação vertical
model = glm::scale(model, glm::vec3(baseScale)); // 4. normalização
model = glm::translate(model, -center);          // 5. centrar na origem
```

**Ordem importa!** Transformações são aplicadas de baixo para cima:
1. Centrar modelo na origem
2. Escalar para tamanho normalizado
3. Aplicar rotações do utilizador
4. Aplicar translação do utilizador

#### Matrizes Derivadas (linhas 275-280)
```cpp
glm::mat4 modelView = view * model;
glm::mat4 MVP = proj * modelView;
glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelView)));
```

**Normal Matrix**:
- Transformação correta para normais (não é simplesmente modelView!)
- Usa inversa transposta para manter perpendiculares corretos
- Apenas 3x3 (normais são vetores de direção, não pontos)

### Conversão de Coordenadas da Luz (linha 289)

```cpp
glm::vec4 lightPosEye = view * glm::vec4(lightPos, 1.0f);
```

**World coords → Eye coords**:
- Luz calculada em coordenadas do mundo
- Shader espera posição em coordenadas da câmera (eye space)
- Multiplicação pela view matrix faz a conversão

---

## 📊 Fluxo de Dados Completo

```
deer.obj → loadOBJ() → vectors (vertices, normals)
                           ↓
                    Interleaved buffer
                           ↓
                      VBO (GPU memory)
                           ↓
             VAO (attribute configuration)
                           ↓
        ┌──────────────────┴──────────────────┐
        ↓                                     ↓
   Vertex Shader                        Vertex Shader
   (phong.vert)                        (simple.vert)
        ↓                                     ↓
   Phong lighting                        MVP transform
        ↓                                     ↓
   Fragment Shader                      Fragment Shader
   (phong.frag)                        (simple.frag)
        ↓                                     ↓
   LightIntensity color                  Solid color
        ↓                                     ↓
        └──────────────────┬──────────────────┘
                           ↓
                      Framebuffer
                           ↓
                        Screen
```

---

## 🔧 Otimizações Implementadas

1. **Buffer Interleaved**: Melhor cache locality
2. **GL_STATIC_DRAW**: Indica que buffers não mudam
3. **Normalização prévia**: Bounding box calculada uma vez
4. **Iluminação no vertex shader**: Mais rápido que per-pixel
5. **VAO switching**: Trocar rapidamente entre deer e light sphere

---

## 📈 Possíveis Melhorias Futuras

1. **Per-pixel lighting**: Mover cálculos Phong para fragment shader
2. **Múltiplas luzes**: Array de luzes
3. **Texturas**: Usar UV coordinates
4. **Materiais diferentes**: Per-object materials
5. **Shadow mapping**: Sombras dinâmicas
6. **Normal mapping**: Detalhes de superfície
7. **Framerate independent animation**: Usar deltaTime real

---

## 🎓 Conceitos-Chave

- **Pipeline gráfico**: Vertex → Geometry → Fragment → Screen
- **Coordenadas**: Object → World → Eye → Clip → NDC → Screen
- **Iluminação Phong**: Ambiente + Difusa + Especular
- **Transformações**: Translate, Rotate, Scale (ordem importa!)
- **Buffers**: VAO/VBO/EBO para dados de geometria
- **Shaders**: Programas que correm na GPU

---

*Esta documentação cobre a implementação completa do TP2. Para mais detalhes sobre uma secção específica, consulta os comentários no código ou as linhas referenciadas.*
