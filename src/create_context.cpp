#include <cmath>
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "stb/stb_image.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "learnopengl/shader_m.h"
#include "terrapainter/util.h"
#include "terrapainter/math.h"
#include "learnopengl/camera.h"
#include "learnopengl/mesh.h"
// #include "learnopengl/texture.h"

#include <iostream>

using namespace std;

float get_dpi_scale()
{
  const int PRIMARY_DISPLAY_INDEX = 0;
#if defined(_WIN32)
  const float DEFAULT_DDPI = 72.0f;
#else
  const float DEFAULT_DDPI = 96.0f;
#endif

  float ddpi;
  if (!SDL_GetDisplayDPI(PRIMARY_DISPLAY_INDEX, nullptr, nullptr, &ddpi))
  {
    return ddpi / DEFAULT_DDPI;
  }
  else
  {
    return 1.0f;
  }
}

void genTexture(unsigned int &texture, string file_name)
{
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
  // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
  string path = "cs4621/images/" + file_name;
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
}

// ------------------- CAMERA CODE from LearnOpenGL (START) -----------------
// camera
Camera camera(glm::vec3(0.0f, 400.0f, 0.0f));

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
// ------------------- (END) -----------------

int main(int argc, char *argv[])
{

  // Initialize SDL
  SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "1");
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
  {
    error("Failed to initialize SDL: %s\n", SDL_GetError());
  }

  // Set version
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // Create window
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_Window *window = SDL_CreateWindow("Terrapainter", 100, 100, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

  // Create OpenGL context
  SDL_GLContext context = SDL_GL_CreateContext(window);

  SDL_GL_MakeCurrent(window, context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Set up IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init("#version 330");

  ImGuiStyle &style = ImGui::GetStyle();
  ImGuiIO &io = ImGui::GetIO();

  float dpi_scale = get_dpi_scale();
  // io.Fonts->AddFontFromFileTTF("../cs4621/extern/source-code-pro/SourceCodePro-Regular.ttf", floor(12 * dpi_scale));

  style.ScaleAllSizes(1.0f / dpi_scale);

  // Grab mouse
  SDL_SetRelativeMouseMode(SDL_TRUE);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
  {
    error("Failed to initialize GLAD");
  }

  // Set global opengl state
  glEnable(GL_DEPTH_TEST);

  // Set viewport
  glViewport(0, 0, 800, 600);

  Shader shader("cs4621/shaders/heightmap.vs", "cs4621/shaders/heightmap.fs");

  stbi_set_flip_vertically_on_load(true);
  int width, height, nChannels;
  unsigned char *data = stbi_load("cs4621/images/iceland_heightmap.png",
                                  &width, &height, &nChannels,
                                  0);
  if (data)
  {
    cout << "Loaded heightmap of size " << height << " x " << width << endl;
  }
  else
  {
    cout << "Failed to load texture" << endl;
  }

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  std::vector<Vertex> vertices;
  float yScale = 96.0f / 256.0f, yShift = 16.0f;
  int rez = 1;
  unsigned bytePerPixel = nChannels;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      unsigned char *pixelOffset = data + (j + width * i) * bytePerPixel;
      unsigned char y = pixelOffset[0];

      vertices.push_back(
          Vertex{
              .Position = vec3(-height / 2.0f + height * i / (float)height, (int)y * yScale - yShift, -width / 2.0f + width * j / (float)width)});
    }
  }
  std::cout << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
  stbi_image_free(data);

  // ------------------ Normal (start)-------------------------

  // facedata[i] is the vertex index for face i // 3
  std::vector<unsigned int> facedata;
  // loading each face in
  for (int i = 0; i < height - 1; i++)
  {
    for (int j = 0; j < width - 1; j++)
    {
      facedata.push_back(i * width + j);
      facedata.push_back(i * width + j + 1);
      facedata.push_back((i + 1) * width + j);
      facedata.push_back(i * width + j + 1);
      facedata.push_back((i + 1) * width + j + 1);
      facedata.push_back((i + 1) * width + j);
    }
  }

  // normal[i] is the vec3 normal of vertices[i]
  std::vector<vec3> normaldata;
  for (int i = 0; i < vertices.size(); i++)
  {
    normaldata.push_back(vec3(0));
  }

  for (int i = 0; i < facedata.size(); i += 3)
  {
    vec3 v1 = vertices.at(facedata.at(i)).Position;
    vec3 v2 = vertices.at(facedata.at(i + 1)).Position;
    vec3 v3 = vertices.at(facedata.at(i + 2)).Position;

    vec3 side1 = v2 - v1;
    vec3 side2 = v3 - v1;
    vec3 normal = cross(side1, side2);

    normaldata[facedata.at(i)] += normal;
    normaldata[facedata.at(i + 1)] += normal;
    normaldata[facedata.at(i + 2)] += normal;
  }

  for (int i = 0; i < normaldata.size(); i += 1)
  {
    normaldata[i] = normaldata[i].normalize();
    vertices[i].Normal = normaldata[i];
  }

  std::vector<vec3> n100(normaldata.begin() + 400000, normaldata.begin() + 400020);
  for (vec3 v : n100)
  {
    std::cout << v << std::endl;
  }

  std::vector<unsigned> indices;
  for (unsigned i = 0; i < height - 1; i += rez)
  {
    for (unsigned j = 0; j < width; j += rez)
    {
      for (unsigned k = 0; k < 2; k++)
      {
        indices.push_back(j + width * (i + k * rez));
      }
    }
  }
  // ------------------- Normal(End) -----------------

  std::cout << "Loaded " << indices.size() << " indices" << std::endl;

  vector<Texture> textures;
  Mesh *map = new Mesh(vertices, indices, textures);

  const int numStrips = (height - 1) / rez;
  const int numTrisPerStrip = (width / rez) * 2 - 2;
  std::cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << std::endl;
  std::cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << std::endl;

  // -----------------------Testing Texture -----------------------------------
  // unsigned int texture1;
  // glGenTextures(1, &texture1);
  // glBindTexture(GL_TEXTURE_2D, texture1);
  // // set the texture wrapping parameters
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // // set texture filtering parameters
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // // load image, create texture and generate mipmaps
  // int width_img, height_img, nrChannels_img;
  // stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
  // // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
  // string path = "cs4621/images/grass.png";
  // unsigned char *image = stbi_load(path.c_str(), &width_img, &height_img, &nrChannels_img, 0);
  // if (image)
  // {
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_img, height_img, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  //   glGenerateMipmap(GL_TEXTURE_2D);
  // }
  // else
  // {
  //   std::cout << "Failed to load texture" << std::endl;
  // }
  // stbi_image_free(image);

  // shader.use();
  // std::cout << glGetError() << std::endl;
  // shader.setInt("texture_diff1", 0);
  // std::cout << glGetError() << std::endl;

  unsigned int texture1;
  genTexture(texture1, "grass.png");
  shader.use();
  std::cout << glGetError() << std::endl;
  shader.setInt("texture_diff1", 0);

  // --------------------------------End Testing --------------------------------

  bool running = true;
  bool show_demo_window = true;

  // Run the event loop
  SDL_Event windowEvent;
  while (running)
  {
    // update time
    float currentFrame = static_cast<float>(SDL_GetTicks64()) * 0.001f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (currentKeyStates[SDL_SCANCODE_UP])
    {
      camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (currentKeyStates[SDL_SCANCODE_DOWN])
    {
      camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (currentKeyStates[SDL_SCANCODE_LEFT])
    {
      camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (currentKeyStates[SDL_SCANCODE_RIGHT])
    {
      camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    // rendering commands
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    shader.use();

    // Camera Projection
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)800 / (float)600, 0.1f, 100000.0f);
    shader.setMat4("projection", projection);

    // Camera View
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("view", view);

    // World Transformation
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    shader.setVec3("LightDir", glm::vec3(-2.0f, -5.0, 0.0f));
    shader.setVec3("viewPos", camera.Position);

    // render container
    map->DrawStrips(numTrisPerStrip, numStrips);
    // map->Draw(shader);

    // handle events
    while (SDL_PollEvent(&windowEvent))
    {
      ImGui_ImplSDL2_ProcessEvent(&windowEvent);
      // This makes dragging windows feel snappy
      io.MouseDrawCursor = ImGui::IsMouseDragging(0);

      if (windowEvent.type == SDL_QUIT)
      {
        running = false;
      }
      // This ridiculous if statement is from https://github.com/ocornut/imgui/blob/master/examples/example_sdl_opengl3/main.cpp
      else if (
          windowEvent.type == SDL_WINDOWEVENT &&
          windowEvent.window.event == SDL_WINDOWEVENT_CLOSE &&
          windowEvent.window.windowID == SDL_GetWindowID(window))
      {
        running = false;
      }
      else if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
      {
        glViewport(0, 0, windowEvent.window.data1, windowEvent.window.data2);
      }
      else if (windowEvent.type == SDL_KEYDOWN)
      {
        if (windowEvent.key.keysym.sym == SDLK_ESCAPE)
        {
          running = false;
        }
      }
      else if (windowEvent.type == SDL_MOUSEMOTION)
      {
        camera.ProcessMouseMovement(windowEvent.motion.xrel, -windowEvent.motion.yrel);
      }
      else if (windowEvent.type == SDL_MOUSEWHEEL)
      {
        camera.ProcessMouseScroll(static_cast<float>(windowEvent.wheel.y));
      }
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (show_demo_window)
    {
      ImGui::ShowDemoWindow(&show_demo_window);
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // swap buffers
    SDL_GL_SwapWindow(window);
  }

  // Shutdown IMGUI
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  // Shutdown SDL
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
