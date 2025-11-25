
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "objloader.hpp"
#include <cfloat>

//estrutura que representa o estado de input
struct InputState
{
  bool rotating = false;
  double lastX = 0, lastY = 0;
  float yaw = 0.0f, pitch = 0.0f;
    glm::vec3 trans = glm::vec3 (0.0f);
  float lightRotationSpeed = 1.0f;
  bool lightPaused = false;
  float lightAngle = 0.0f;

  // State for toggles
  bool spacePressed = false;
  bool plusPressed = false;
  bool minusPressed = false;
  bool rPressed = false;
};

//processa os inputs do utilizador
void
processInput (GLFWwindow *window, InputState & input)
{
  // ESC
  if (glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose (window, true);

  // Mouse Rotation
  if (glfwGetMouseButton (window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
      double x, y;
      glfwGetCursorPos (window, &x, &y);
      if (!input.rotating)
	{
	  input.lastX = x;
	  input.lastY = y;
	  input.rotating = true;
	}
      double dx = x - input.lastX;
      double dy = y - input.lastY;
      input.yaw += float (dx) * 0.005f;
      input.pitch += float (dy) * 0.005f;
      input.lastX = x;
      input.lastY = y;
    }
  else
    {
      input.rotating = false;
    }

  // Movement
  float step = 0.05f;
  if (glfwGetKey (window, GLFW_KEY_S) == GLFW_PRESS
      || glfwGetKey (window, GLFW_KEY_DOWN) == GLFW_PRESS)
    input.trans.y += step;
  if (glfwGetKey (window, GLFW_KEY_W) == GLFW_PRESS
      || glfwGetKey (window, GLFW_KEY_UP) == GLFW_PRESS)
    input.trans.y -= step;
  if (glfwGetKey (window, GLFW_KEY_D) == GLFW_PRESS
      || glfwGetKey (window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    input.trans.x -= step;
  if (glfwGetKey (window, GLFW_KEY_A) == GLFW_PRESS
      || glfwGetKey (window, GLFW_KEY_LEFT) == GLFW_PRESS)
    input.trans.x += step;
  if (glfwGetKey (window, GLFW_KEY_Q) == GLFW_PRESS
      || glfwGetKey (window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
    input.trans.z += step;
  if (glfwGetKey (window, GLFW_KEY_E) == GLFW_PRESS
      || glfwGetKey (window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
    input.trans.z -= step;

  // Light Pause (Space)
  if (glfwGetKey (window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
      if (!input.spacePressed)
	{
	  input.lightPaused = !input.lightPaused;
	  std::printf ("Rotação da luz: %s\n",
		       input.lightPaused ? "PARADA" : "A RODAR");
	  input.spacePressed = true;
	}
    }
  else
    {
      input.spacePressed = false;
    }

  // Light Speed +
  bool plus = (glfwGetKey (window, GLFW_KEY_EQUAL) == GLFW_PRESS
	       || glfwGetKey (window, GLFW_KEY_KP_ADD) == GLFW_PRESS);
  if (plus)
    {
      if (!input.plusPressed)
	{
	  input.lightRotationSpeed += 0.5f;
	  std::printf ("Velocidade de rotação da luz: %.1f\n",
		       input.lightRotationSpeed);
	  input.plusPressed = true;
	}
    }
  else
    {
      input.plusPressed = false;
    }

  // Light Speed -
  bool minus = (glfwGetKey (window, GLFW_KEY_MINUS) == GLFW_PRESS
		|| glfwGetKey (window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS);
  if (minus)
    {
      if (!input.minusPressed)
	{
	  input.lightRotationSpeed =
	    std::max (0.0f, input.lightRotationSpeed - 0.5f);
	  std::printf ("Velocidade de rotação da luz: %.1f\n",
		       input.lightRotationSpeed);
	  input.minusPressed = true;
	}
    }
  else
    {
      input.minusPressed = false;
    }

  // Reset R
  if (glfwGetKey (window, GLFW_KEY_R) == GLFW_PRESS)
    {
      if (!input.rPressed)
	{
	  input.lightRotationSpeed = 1.0f;
          input.trans = glm::vec3(0.0f);
          input.yaw = 0.0f;
          input.pitch = 0.0f;
          input.lightPaused = false;
          input.lightAngle = 0.0f;
	  std::printf ("Estado reiniciado: Posição, Rotação, Luz\n");
	  input.rPressed = true;
	}
    }
  else
    {
      input.rPressed = false;
    }
}


//inicializa a janela
GLFWwindow *
initWindow (int width, int height, const char *title)
{
  if (!glfwInit ())
    {
      std::fprintf (stderr, "GLFW init falhou\n");
      return nullptr;
    }

  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow (width, height, title, nullptr, nullptr);
  if (!win)
    {
      std::fprintf (stderr, "Falha a criar janela\n");
      glfwTerminate ();
      return nullptr;
    }
  glfwMakeContextCurrent (win);

  glewExperimental = GL_TRUE;
  if (glewInit () != GLEW_OK)
    {
      std::fprintf (stderr, "glewInit falhou\n");
      return nullptr;
    }
  glGetError ();

  glfwSetFramebufferSizeCallback (win,[](GLFWwindow *, int w, int h)
				  { glViewport (0, 0, w, h);
				  });

  return win;
}

//carrega o modelo 3D do veado
GLuint
setupDeerMesh (const char *filename, size_t &vertexCount, float &baseScale,
	       glm::vec3 & center)
{
  std::vector < glm::vec3 > vertices;
  std::vector < glm::vec2 > uvs;
  std::vector < glm::vec3 > normals;
  bool res = loadOBJ (filename, vertices, uvs, normals);
  if (!res)
    {
      std::fprintf (stderr, "Impossible to open %s or parse it\n", filename);
      return 0;
    }

  //interleaves vertices and normals
  std::vector < float >interleaved;
  interleaved.reserve (vertices.size () * 6);
  for (size_t i = 0; i < vertices.size (); ++i)
    {
      const glm::vec3 & p = vertices[i];
      glm::vec3 n (0.0f, 0.0f, 1.0f);
      if (i < normals.size ())
	n = normals[i];
      interleaved.push_back (p.x);
      interleaved.push_back (p.y);
      interleaved.push_back (p.z);
      interleaved.push_back (n.x);
      interleaved.push_back (n.y);
      interleaved.push_back (n.z);
    }

  glm::vec3 minb (FLT_MAX), maxb (-FLT_MAX);
for (auto & v:vertices)
    {
      minb = glm::min (minb, v);
      maxb = glm::max (maxb, v);
    }
  center = (minb + maxb) * 0.5f;
  glm::vec3 diag = maxb - minb;
  float extent = std::max (diag.x, std::max (diag.y, diag.z));
  if (extent <= 0.0f)
    extent = 1.0f;
  baseScale = 1.0f / extent;

  GLuint vao = 0, vbo = 0;
  glGenVertexArrays (1, &vao);
  glBindVertexArray (vao);
  glGenBuffers (1, &vbo);
  glBindBuffer (GL_ARRAY_BUFFER, vbo);
  glBufferData (GL_ARRAY_BUFFER, interleaved.size () * sizeof (float),
		interleaved.data (), GL_STATIC_DRAW);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof (float),
			 (void *) 0);
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof (float),
			 (void *) (3 * sizeof (float)));
  glEnableVertexAttribArray (1);

  vertexCount = vertices.size ();
  return vao;
}

//cria uma esfera para representar a luz
GLuint
setupLightSphere (size_t &indexCount)
{
  std::vector < float >lightSphereVerts;
  const int slices = 10, stacks = 10;
  const float lightSphereRadius = 0.05f;
  for (int i = 0; i <= stacks; ++i)
    {
      float phi = M_PI * float (i) / float (stacks);
      for (int j = 0; j <= slices; ++j)
	{
	  float theta = 2.0f * M_PI * float (j) / float (slices);
	  float x = lightSphereRadius * std::sin (phi) * std::cos (theta);
	  float y = lightSphereRadius * std::cos (phi);
	  float z = lightSphereRadius * std::sin (phi) * std::sin (theta);
	  lightSphereVerts.push_back (x);
	  lightSphereVerts.push_back (y);
	  lightSphereVerts.push_back (z);
	}
    }

  std::vector < unsigned int >lightSphereIndices;
  for (int i = 0; i < stacks; ++i)
    {
      for (int j = 0; j < slices; ++j)
	{
	  int first = i * (slices + 1) + j;
	  int second = first + slices + 1;
	  lightSphereIndices.push_back (first);
	  lightSphereIndices.push_back (second);
	  lightSphereIndices.push_back (first + 1);
	  lightSphereIndices.push_back (second);
	  lightSphereIndices.push_back (second + 1);
	  lightSphereIndices.push_back (first + 1);
	}
    }

  GLuint lightVAO = 0, lightVBO = 0, lightEBO = 0;
  glGenVertexArrays (1, &lightVAO);
  glBindVertexArray (lightVAO);
  glGenBuffers (1, &lightVBO);
  glBindBuffer (GL_ARRAY_BUFFER, lightVBO);
  glBufferData (GL_ARRAY_BUFFER, lightSphereVerts.size () * sizeof (float),
		lightSphereVerts.data (), GL_STATIC_DRAW);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (float),
			 (void *) 0);
  glEnableVertexAttribArray (0);
  glGenBuffers (1, &lightEBO);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, lightEBO);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER,
		lightSphereIndices.size () * sizeof (unsigned int),
		lightSphereIndices.data (), GL_STATIC_DRAW);

  indexCount = lightSphereIndices.size ();
  return lightVAO;
}

int
main ()
{
  GLFWwindow *win = initWindow (900, 600, "TP2 - Rendering .obj file");
  if (!win)
    return -1;

  size_t vertexCount = 0;
  float baseScale = 1.0f;
  glm::vec3 center (0.0f);
  GLuint vao = setupDeerMesh ("deer.obj", vertexCount, baseScale, center);
  if (vao == 0)
    {
      glfwTerminate ();
      return -1;
    }

  // shaders (use the Phong shaders)
  GLuint prog =
    linkProgramFromFiles ("shaders/phong.vert", "shaders/phong.frag");
  if (!prog)
    {
      std::fprintf (stderr, "Shaders não compilados\n");
      return -1;
    }
  glUseProgram (prog);

  // Uniform locations used by the phong shader
  GLint uModelView = getUniform (prog, "ModelViewMatrix");
  GLint uNormalMat = getUniform (prog, "NormalMatrix");
  // Note: 'ProjectionMatrix' is declared in the shader but not used; we skip fetching it to avoid driver optimization warnings.
  GLint uMVP = getUniform (prog, "MVP");

  // Light struct members
  GLint uLight_Position = getUniform (prog, "Light.Position");
  GLint uLight_La = getUniform (prog, "Light.La");
  GLint uLight_Ld = getUniform (prog, "Light.Ld");
  GLint uLight_Ls = getUniform (prog, "Light.Ls");

  // Material struct members
  GLint uMat_Ka = getUniform (prog, "Material.Ka");
  GLint uMat_Kd = getUniform (prog, "Material.Kd");
  GLint uMat_Ks = getUniform (prog, "Material.Ks");
  GLint uMat_Shininess = getUniform (prog, "Material.Shininess");

  // set material (static)
  glUniform3f (uMat_Ka, 0.2f, 0.2f, 0.2f);
  glUniform3f (uMat_Kd, 0.6f, 0.6f, 0.6f);
  glUniform3f (uMat_Ks, 0.9f, 0.9f, 0.9f);
  glUniform1f (uMat_Shininess, 32.0f);

  // set light ambient/diffuse/specular (static intensities)
  glUniform3f (uLight_La, 0.1f, 0.1f, 0.1f);
  glUniform3f (uLight_Ld, 0.8f, 0.8f, 0.8f);
  glUniform3f (uLight_Ls, 1.0f, 1.0f, 1.0f);

  // input state for interaction
  InputState input;

  // Time for light rotation


  // Create a simple shader for rendering the light source
  GLuint lightProg =
    linkProgramFromFiles ("shaders/simple.vert", "shaders/simple.frag");
  if (!lightProg)
    {
      std::fprintf (stderr, "Shaders não compilados\n");
      return -1;
    }
  GLint uLightMVP = getUniform (lightProg, "MVP");
  GLint uLightColor = getUniform (lightProg, "LightColor");

  size_t lightIndexCount = 0;
  GLuint lightVAO = setupLightSphere (lightIndexCount);



  glClearColor (0.1f, 0.1f, 0.1f, 1.0f);	//cor de fundo

  glEnable (GL_DEPTH_TEST);	//necessário

  while (!glfwWindowShouldClose (win))
    {
      processInput (win, input);

      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Update light rotation angle based on time and speed (only if not paused)
      if (!input.lightPaused)
	{
	  input.lightAngle += input.lightRotationSpeed * 0.01f;	// Adjust rotation speed
	}

      // update camera/projection each frame (handles resize)
      int fbw, fbh;
      glfwGetFramebufferSize (win, &fbw, &fbh);
      float aspect = (fbh == 0) ? 1.0f : (float) fbw / (float) fbh;
      glm::vec3 camPos (0.0f, 0.0f, 2.5f);
      glm::mat4 view =
	glm::lookAt (camPos, glm::vec3 (0.0f), glm::vec3 (0.0f, 1.0f, 0.0f));
      glm::mat4 proj =
	glm::perspective (glm::radians (45.0f), aspect, 0.01f, 100.0f);
      // compute matrices required by the phong shader (Projection matrix declared in shader is unused)

      // build model transform from input (translate, rotate, scale and center)
      glm::mat4 model (1.0f);
      // translate user-controlled
      model = glm::translate (model, input.trans);
      // rotate (note: rotate expects radians)
      model = glm::rotate (model, input.yaw, glm::vec3 (0, 1, 0));
      model = glm::rotate (model, input.pitch, glm::vec3 (1, 0, 0));
      // scale to normalized size and center
      model = glm::scale (model, glm::vec3 (baseScale));
      model = glm::translate (model, -center);
      // build ModelView, MVP and NormalMatrix and send to shader
      glm::mat4 modelView = view * model;
      glm::mat4 MVP = proj * modelView;
      glm::mat3 normalMatrix =
	glm::mat3 (glm::transpose (glm::inverse (modelView)));
      glUniformMatrix4fv (uModelView, 1, GL_FALSE,
			  glm::value_ptr (modelView));
      glUniformMatrix4fv (uMVP, 1, GL_FALSE, glm::value_ptr (MVP));
      glUniformMatrix3fv (uNormalMat, 1, GL_FALSE,
			  glm::value_ptr (normalMatrix));

      // Rotating point light in a circle around the deer (world coords)
      float lightRadius = 2.0f;	// Distance from origin
      glm::vec3 lightPos (lightRadius * std::cos (input.lightAngle), 1.0f,	// Keep light at fixed height
			  lightRadius * std::sin (input.lightAngle));
      glm::vec4 lightPosEye = view * glm::vec4 (lightPos, 1.0f);	// convert to eye coords
      glUniform4f (uLight_Position, lightPosEye.x, lightPosEye.y,
		   lightPosEye.z, lightPosEye.w);

      glBindVertexArray (vao);
      glDrawArrays (GL_TRIANGLES, 0, (GLsizei) vertexCount);	// draw triangles

      // Render the light source as a small sphere
      glUseProgram (lightProg);
      glm::mat4 lightModel = glm::translate (glm::mat4 (1.0f), lightPos);
      glm::mat4 lightMVP = proj * view * lightModel;
      glUniformMatrix4fv (uLightMVP, 1, GL_FALSE, glm::value_ptr (lightMVP));
      glUniform3f (uLightColor, 1.0f, 1.0f, 0.0f);	// Yellow/bright color
      glBindVertexArray (lightVAO);
      glDrawElements (GL_TRIANGLES, (GLsizei) lightIndexCount,
		      GL_UNSIGNED_INT, 0);

      // Switch back to the phong shader for next frame
      glUseProgram (prog);


      glfwSwapBuffers (win);
      glfwPollEvents ();
    }

  glfwTerminate ();
}
