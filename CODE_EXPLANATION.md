# Code Explanation: 3D Deer Renderer with Phong and Blinn-Phong Lighting

## Project Overview

This project renders a 3D deer model with realistic lighting using OpenGL. The application supports both **Phong** and **Blinn-Phong** lighting models, allowing real-time comparison between the two techniques.

**Key Technologies:**
- OpenGL 3.3 (via GLAD)
- GLFW (window management)
- GLM (mathematics)
- LearnOpenGL libraries (Camera, Shader, Transform, FileSystem)
- Custom OBJ/MTL file loaders

---

## Architecture

### Main Components

#### 1. **Input Management** (`InputState` struct)
Tracks all user interactions:
- **Light controls**: Speed, pause/resume, orbit angle
- **Display modes**: Wireframe toggle, Blinn-Phong toggle
- **Model controls**: Rotation via arrow keys
- **Camera controls**: WASD movement + mouse look

```cpp
struct InputState {
  float lightRotationSpeed = 1.0f;  // Light orbit speed
  bool lightPaused = false;         // Pause light rotation
  float lightAngle = 0.0f;          // Current light position
  bool wireframe = false;           // Wireframe mode toggle
  bool blinn = false;               // Blinn-Phong mode toggle
};
```

#### 2. **Model Loading** (`setupDeerMesh()`)
Loads the 3D model with materials:
1. Reads OBJ file vertices and normals
2. Attempts to load MTL (Material Template Library) file
3. Calculates bounding box to center and scale the model
4. Creates VAO/VBO/EBO buffers for GPU rendering

#### 3. **Lighting** (`setupLightBox()`)
Creates a small cube to represent the light source:
- Simple 8-vertex cube (0.1 × 0.1 × 0.1 units)
- 36 indices for 6 faces (2 triangles each)
- Rendered in yellow for visibility

#### 4. **Transform System** (`Transform` class)
Manages model-space transformations:
- Local position, rotation, scale
- Computes model matrix (TRS: Translation × Rotation × Scale)
- Dirty flag optimization (only recomputes when changed)

#### 5. **Camera System** (`Camera` class)
Implements FPS-style camera:
- WASD movement (forward, backward, left, right)
- Mouse look (pitch and yaw)
- Zoom control (scroll wheel)
- Automatic view matrix calculation

---

## Rendering Pipeline

### Per-Frame Steps

1. **Input Processing**
   - Read keyboard and mouse input
   - Update camera position/rotation
   - Update light angle (if not paused)

2. **Matrix Calculations**
   ```cpp
   glm::mat4 view = camera.GetViewMatrix();
   glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), aspect, 0.01f, 100.0f);
   glm::mat4 model = modelTransform.getModelMatrix();
   glm::mat4 modelView = view * model;
   glm::mat4 MVP = proj * modelView;
   ```

3. **Light Position Calculation**
   - Light orbits around the model at radius 2.0
   - Orbit controlled by `lightAngle` (updated by `lightRotationSpeed`)
   - Converted to camera space for shader use

4. **Rendering**
   - Render deer with Phong/Blinn-Phong shader
   - Render light source (yellow cube) with simple shader

---

## Lighting Models: Phong vs Blinn-Phong

### Phong Lighting

**Formula:**
$$L = K_a L_a + K_d L_d (\hat{n} \cdot \hat{l}) + K_s L_s (\hat{v} \cdot \hat{r})^{Ns}$$

Where:
- $K_a, K_d, K_s$ = Ambient, diffuse, specular material coefficients
- $L_a, L_d, L_s$ = Ambient, diffuse, specular light intensities
- $\hat{n}$ = Surface normal
- $\hat{l}$ = Light direction
- $\hat{v}$ = View direction
- $\hat{r}$ = Reflected light direction (calculated as $\hat{r} = 2(\hat{n} \cdot \hat{l})\hat{n} - \hat{l}$)
- $Ns$ = Shininess exponent

**How it works:**
1. **Ambient**: Always present, no direction dependency
2. **Diffuse**: Based on angle between normal and light
3. **Specular**: Shiny highlights based on reflection and view angle

**Strengths:**
- Fast to compute (fewer operations)
- Intuitive understanding
- Works well for most surfaces

**Weaknesses:**
- Specular highlight can look unrealistic
- Highlight disappears abruptly when view angle exceeds light reflection angle
- Produces a narrow, sharp specular region

### Blinn-Phong Lighting

**Formula:**
$$L = K_a L_a + K_d L_d (\hat{n} \cdot \hat{l}) + K_s L_s (\hat{n} \cdot \hat{h})^{Ns}$$

Where:
- $\hat{h} = \frac{\hat{l} + \hat{v}}{|\hat{l} + \hat{v}|}$ (halfway vector)
- All other terms same as Phong

**How it works:**
- Uses the **halfway vector** ($\hat{h}$) instead of reflection vector
- Halfway vector is the normalized sum of light and view directions
- Specular highlight based on angle between normal and halfway vector

**Strengths:**
- More realistic specular highlights
- Smoother, broader highlights
- Better matches real-world observations
- More computationally efficient (no reflection calculation needed)
- Highlight remains visible at steeper angles

**Weaknesses:**
- Less intuitive to understand initially
- Requires slightly different parameter tuning

### Visual Comparison

| Aspect | Phong | Blinn-Phong |
|--------|-------|-------------|
| Reflection calculation | Explicit ($\hat{r}$ vector) | Implicit (halfway vector) |
| Highlight appearance | Sharp, narrow | Soft, broad |
| Realism | Good | Better |
| Performance | Slightly faster | Slightly slower |
| Parameter matching | Difficult | Easier |

**Quick visual test:** Toggle Blinn-Phong with **B key**:
- Notice the specular highlight becomes smoother and more natural
- Highlight remains visible at steeper viewing angles
- Overall appearance is more like real materials

---

## User Controls

### Camera Controls
- **W/A/S/D**: Move camera forward/left/backward/right
- **Mouse**: Look around (hold left mouse button)
- **Scroll**: Zoom in/out

### Model Controls
- **Arrow Keys (↑↓←→)**: Rotate model

### Light Controls
- **SPACE**: Pause/resume light rotation
- **+/-**: Increase/decrease light orbit speed

### Display Controls
- **F**: Toggle wireframe mode
- **B**: Toggle Blinn-Phong lighting
- **R**: Reset all (position, rotation, light, wireframe, Blinn-Phong)
- **ESC**: Exit program

---

## Shader System

### Phong Shader (`phong.vert` + `phong.frag`)

**Vertex Shader:**
- Transforms vertex positions to eye space
- Calculates normal in eye space (using normal matrix)
- Passes to fragment shader

**Fragment Shader:**
- Implements Phong or Blinn-Phong (controlled by uniform `blinn`)
- Calculates reflection/halfway vector
- Computes final color with all lighting components

### Simple Shader (`simple.vert` + `simple.frag`)

Used for rendering the light source:
- Transforms vertices with MVP matrix
- Outputs simple yellow color

---

## File Structure

```
ExercicioPrtico2/
├── src/
│   ├── main.cpp              # Main application code
│   ├── objloader.cpp         # OBJ/MTL file loading
│   └── shader.cpp            # (if exists)
├── common/
│   ├── Mesh.hpp              # Custom mesh class
│   ├── objloader.hpp         # OBJ loader header
│   └── learnopengl/          # LearnOpenGL utilities
│       ├── camera.h          # FPS camera implementation
│       ├── shader.h          # Shader compilation/management
│       ├── transform.h       # Transform matrix calculations
│       └── filesystem.h      # Asset path management
├── shaders/
│   ├── phong.vert            # Phong vertex shader
│   ├── phong.frag            # Phong fragment shader
│   ├── simple.vert           # Light source vertex shader
│   └── simple.frag           # Light source fragment shader
├── deer.obj                  # 3D model file
└── deer.mtl                  # Material definitions
```

---

## Key Implementation Details

### Material Loading
Materials are loaded from MTL files containing:
- **Ka** (Ambient color): How material appears under ambient light
- **Kd** (Diffuse color): Main color of the material
- **Ks** (Specular color): Color of highlights
- **Ns** (Shininess): Controls highlight size (higher = sharper)

### Bounding Box Calculation
Used to automatically scale and center the model:
```cpp
glm::vec3 minb = min point of all vertices;
glm::vec3 maxb = max point of all vertices;
center = (minb + maxb) * 0.5f;  // Center point
extent = max dimension of bounding box;
baseScale = 1.0f / extent;      // Normalize size
```

### Light Orbit
Light position calculated as:
```cpp
glm::vec3 lightPos(
    lightRadius * cos(lightAngle),  // x: circle orbit
    1.0f,                            // y: fixed height
    lightRadius * sin(lightAngle)    // z: circle orbit
);
```

---

## Performance Considerations

- **Dirty flag optimization**: Transform only recomputes when values change
- **Vertex array objects (VAOs)**: GPU-side caching of vertex data
- **Single light source**: Simplified for performance
- **No texture mapping**: Reduces memory and computation

---

## Extending the Project

To add more features:

1. **Multiple lights**: Loop through light array, accumulate contributions
2. **Textures**: Extend Vertex struct with TexCoords, update shaders
3. **Normal mapping**: Use normal maps for surface detail
4. **Shadows**: Implement shadow mapping with depth texture
5. **Post-processing**: Add bloom, tone mapping, etc.

---

## References

- LearnOpenGL: https://learnopengl.com/
- Phong Lighting: https://en.wikipedia.org/wiki/Phong_reflection_model
- Blinn-Phong: https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model
- OpenGL Documentation: https://www.khronos.org/opengl/wiki/

