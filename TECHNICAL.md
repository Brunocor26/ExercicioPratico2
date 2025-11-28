# Documentação Técnica - TP2 Rendering Engine

Este documento explica em detalhe a implementação do código e como funciona internamente o motor de renderização 3D.

---

## 🏗️ Arquitetura Geral

### Fluxo do Programa

```
1. Inicialização GLFW/GLEW
2. Carregamento do modelo .obj (apenas geometria e normais)
3. Criação de buffers (VAO/VBO)
4. Compilação de shaders (Phong Per-Pixel)
5. Configuração de uniforms
6. Loop de renderização
7. Terminação
```

---

## 📐 1. Carregamento e Normalização do Modelo

### Carregamento OBJ

```cpp
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
bool res = loadOBJ("deer.obj", vertices, normals);
```

A função `loadOBJ()` (implementada em `objloader.cpp`) lê o ficheiro `.obj` e extrai:
- **Vértices**: Posições 3D de cada ponto do modelo
- **Normais**: Vetores perpendiculares às superfícies (essenciais para iluminação)
- *Nota: Coordenadas de textura (UVs) são ignoradas nesta versão.*

### Buffer Interleaved

```cpp
std::vector<float> data;
for (size_t i = 0; i < vertices.size(); ++i) {
    // pos.x, pos.y, pos.z, normal.x, normal.y, normal.z
    data.insert(data.end(), {v[i].x, v[i].y, v[i].z, n[i].x, n[i].y, n[i].z});
}
```

**Porquê interleaved?**
- Melhor performance de cache: dados relacionados ficam adjacentes na memória
- Formato: `[x, y, z, nx, ny, nz, ...]`
- Cada vértice ocupa 6 floats (3 posição + 3 normal)

### Normalização do Modelo

O código calcula a bounding box do modelo para determinar o centro e a escala ideal, garantindo que o modelo fica visível e centrado na cena, independentemente das suas dimensões originais.

---

## 🎨 2. Configuração OpenGL

### Vertex Array Object (VAO) e Buffers

**VAO (Vertex Array Object)**:
- Armazena a configuração de como os dados dos vértices são organizados.

**VBO (Vertex Buffer Object)**:
- Buffer que armazena os dados dos vértices na GPU.

### Atributos dos Vértices

```cpp
// Posição (location = 0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// Normal (location = 1)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
glEnableVertexAttribArray(1);
```

---

## 🎭 3. Sistema de Shaders

### Shader Phong (Modelo Principal)

A iluminação é calculada **por pixel** (Phong Shading) para maior qualidade visual, em oposição ao Gouraud Shading (por vértice).

#### Vertex Shader (`phong.vert`)

Responsável por transformar os vértices e preparar os dados para o fragment shader.

- **Entradas**: Posição e Normal do vértice.
- **Saídas**:
    - `Normal`: Normal transformada para Eye Space.
    - `EyeCoords`: Posição do vértice em Eye Space.
    - `gl_Position`: Posição projetada no ecrã (MVP * Vertex).

#### Fragment Shader (`phong.frag`)

Realiza o cálculo de iluminação Phong para cada pixel.

**Cálculo de Iluminação**:
1. **Ambiente**: `Light.La * Material.Ka`
2. **Difusa**: `Light.Ld * Material.Kd * max(dot(s, N), 0)`
3. **Especular**: `Light.Ls * Material.Ks * pow(max(dot(r,v), 0), Shininess)`

Onde:
- `N`: Normal interpolada e normalizada.
- `s`: Vetor direção da luz.
- `v`: Vetor direção da câmara (viewer).
- `r`: Vetor de reflexão.

### Shader Simples (Esfera da Luz)

Utilizado para desenhar a representação visual da fonte de luz.
- **Vertex Shader**: Apenas aplica a matriz MVP.
- **Fragment Shader**: Retorna uma cor sólida uniforme.

---

## 💡 4. Sistema de Iluminação Dinâmica

A luz orbita em torno do modelo numa trajetória circular.

**Cálculo da posição**:
```cpp
glm::vec4 lPos = view * glm::vec4(in.lightRadius * cos(in.lightAngle), 1, in.lightRadius * sin(in.lightAngle), 1);
```
- A posição é calculada em coordenadas do mundo e transformada para Eye Space pela matriz View antes de ser enviada para o shader.

---

## 🎮 5. Sistema de Input

O sistema de input utiliza **polling** direto do estado do GLFW na função `processInput()`.

### Controlos

- **Rato (Botão Esquerdo + Arrastar)**: Rotação do modelo (Yaw/Pitch).
- **WASDQE**: Translação do modelo nos eixos X, Y e Z.
- **Espaço**: Pausar/Retomar a rotação automática da luz.
- **+/-**: Aumentar/Diminuir velocidade da luz.
- **K/L**: Diminuir/Aumentar raio da órbita da luz.
- **C**: Alternar cor da luz.
- **R**: Reset da câmara e luz.

---

## 🔄 6. Loop de Renderização

1. **Processar Input**: Verificar teclado e rato.
2. **Limpar Buffers**: Color e Depth buffers.
3. **Atualizar Luz**: Calcular nova posição angular se não estiver em pausa.
4. **Configurar Matrizes**:
   - `View`: LookAt (câmara fixa).
   - `Projection`: Perspective.
   - `Model`: Transformações acumuladas (translação, rotação, escala).
5. **Renderizar Modelo**:
   - Ativar shader Phong.
   - Enviar uniforms (luz, material, matrizes).
   - Desenhar VAO do modelo.
6. **Renderizar Luz**:
   - Ativar shader Simples.
   - Calcular MVP para a esfera de luz.
   - Desenhar VAO da luz.
7. **Swap Buffers & Poll Events**.

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
   Passa Normal/EyeCoords              MVP transform
        ↓                                     ↓
   Fragment Shader                      Fragment Shader
   (phong.frag)                        (simple.frag)
        ↓                                     ↓
   Phong Lighting (Per-Pixel)           Solid color
        ↓                                     ↓
        └──────────────────┬──────────────────┘
                           ↓
                      Framebuffer
                           ↓
                        Screen
```

---

## 🔧 Otimizações Implementadas

1. **Buffer Interleaved**: Melhor cache locality.
2. **GL_STATIC_DRAW**: Indica que buffers de geometria não mudam frequentemente.
3. **Normalização prévia**: Bounding box calculada uma vez no carregamento.
4. **Per-Pixel Lighting**: Iluminação mais suave e realista que Gouraud shading.

---

## 📈 Possíveis Melhorias Futuras

1. **Texturas**: Implementar suporte para texturas difusas e normal maps.
2. **Múltiplas luzes**: Suporte para array de luzes no shader.
3. **Materiais diferentes**: Carregar propriedades de material do ficheiro .mtl.
4. **Shadow mapping**: Sombras dinâmicas.

---

*Esta documentação cobre a implementação atual do TP2, focada em iluminação Phong pura sem texturas.*
