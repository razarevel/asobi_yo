#include "Bitmaps.h"
#include "UtilsCubemap.h"
#include "mai_renderer.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "Camera.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using glm::vec2;
using glm::vec3;

struct MouseState {
  glm::vec2 pos = glm::vec2(0.0f);
  bool pressedLeft = false;
} mouseState;

const vec3 kInitialCameraPos = vec3(0.0f, 1.0f, -1.5f);
const vec3 kInitialCameraTarget = vec3(0.0f, 0.5f, 0.0f);

CameraPositioner_FirstPerson positioner(kInitialCameraPos, kInitialCameraTarget,
                                        vec3(0.0f, 1.0f, 0.0f));
Camera camera(positioner);

int main() {
  MAI::MAIRendererInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Asobi yo",
  };
  MAI::MAIRenderer *renderer = new MAI::MAIRenderer(info);

  const aiScene *scene = aiImportFile(RESOURCES_PATH "rubber_duck/scene.gltf",
                                      aiProcess_Triangulate);
  assert(scene);
  const aiMesh *mesh = scene->mMeshes[0];

  struct Vertex {
    glm::vec3 pos;
    glm::vec3 norms;
    glm::vec2 texCoords;
  };

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vertices.reserve(mesh->mNumVertices);
  indices.reserve(3 * mesh->mNumFaces);
  for (size_t i = 0; i < mesh->mNumVertices; i++) {
    const aiVector3D p = mesh->mVertices[i];
    const aiVector3D n = mesh->mNormals[i];
    const aiVector3D t =
        mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f);
    vertices.push_back({
        .pos = glm::vec3(p.x, p.y, p.z),
        .norms = glm::vec3(n.x, n.y, n.y),
        .texCoords = glm::vec2(t.x, t.y),
    });
  }

  for (size_t i = 0; i < mesh->mNumFaces; i++)
    for (size_t j = 0; j != 3; j++)
      indices.emplace_back(mesh->mFaces[i].mIndices[j]);

  aiReleaseImport(scene);

  MAI::VKbuffer *vertBuffer = renderer->createBuffer({
      .size = sizeof(Vertex) * vertices.size(),
      .data = vertices.data(),
  });
  MAI::VKbuffer *indexBuffer = renderer->createBuffer({
      .size = sizeof(uint32_t) * indices.size(),
      .data = indices.data(),
  });

  MAI::VKShader *vert =
      renderer->createShader(EXAMPLES_DIR "cubemap/spvs/main.vspv");
  MAI::VKShader *frag =
      renderer->createShader(EXAMPLES_DIR "cubemap/spvs/main.fspv");

  struct PushConstant {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
    uint32_t texId;
    uint32_t texCube;
    uint64_t vertices;
    uint64_t indices;
  } pc = {};

  MAI::VKPipeline *pipeline = renderer->createPipeline({
      .vert = vert,
      .frag = frag,
      .pushConstants =
          {
              .stageFlags =
                  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
              .offset = 0,
              .size = sizeof(pc),
          },
  });
  delete vert;
  delete frag;

  int w, h, comp;
  const uint8_t *img =
      stbi_load(RESOURCES_PATH "rubber_duck/textures/Duck_baseColor.png", &w,
                &h, &comp, 4);
  assert(img);

  MAI::VKTexture *texture = renderer->createTexture({
      .width = (uint32_t)w,
      .height = (uint32_t)h,
      .data = img,
  });

  stbi_image_free((void *)img);

  MAI::VKTexture *cubemapTex = nullptr;
  {
    int w, h;
    const float *img =
        stbi_loadf(RESOURCES_PATH "piazza_bologni_1k.hdr", &w, &h, nullptr, 4);
    Bitmap in(w, h, 4, eBitmapFormat_Float, img);
    Bitmap out = convertEquirectangularMapToVerticalCross(in);
    stbi_image_free((void *)img);

    Bitmap cubemap = convertVerticalCrossToCubeMapFaces(out);

    cubemapTex = renderer->createTexture({
        .width = (uint32_t)cubemap.w_,
        .height = (uint32_t)cubemap.h_,
        .data = cubemap.data_.data(),
        .format = MAI::MAI_TEXTURE_CUBE,
    });
  }

  struct PushConstantCubemap {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
    uint32_t texId;
    uint32_t texCube;
  } pcCubemap = {};

  MAI::VKShader *vertSky =
      renderer->createShader(EXAMPLES_DIR "cubemap/spvs/cubemap.vspv");
  MAI::VKShader *fragSky =
      renderer->createShader(EXAMPLES_DIR "cubemap/spvs/cubemap.fspv");
  MAI::VKPipeline *pipelineSkybox = renderer->createPipeline({
      .vert = vertSky,
      .frag = fragSky,
      .cullMode = VK_CULL_MODE_NONE,
      .pushConstants =
          {
              .stageFlags =
                  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
              .offset = 0,
              .size = sizeof(pcCubemap),
          },
  });
  delete vertSky;
  delete fragSky;

  glfwSetCursorPosCallback(
      renderer->getWindow(), [](auto *window, double x, double y) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        mouseState.pos.x = static_cast<float>(x / width);
        mouseState.pos.y = 1.0f - static_cast<float>(y / height);
      });

  glfwSetMouseButtonCallback(renderer->getWindow(), [](auto *window, int button,
                                                       int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      mouseState.pressedLeft = action == GLFW_PRESS;
    }
  });

  glfwSetKeyCallback(
      renderer->getWindow(),
      [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        const bool pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_ESCAPE && pressed)
          glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (key == GLFW_KEY_W)
          positioner.movement_.forward_ = pressed;
        if (key == GLFW_KEY_S)
          positioner.movement_.backward_ = pressed;
        if (key == GLFW_KEY_A)
          positioner.movement_.left_ = pressed;
        if (key == GLFW_KEY_D)
          positioner.movement_.right_ = pressed;
        if (key == GLFW_KEY_1)
          positioner.movement_.up_ = pressed;
        if (key == GLFW_KEY_2)
          positioner.movement_.down_ = pressed;
        if (mods & GLFW_MOD_SHIFT)
          positioner.movement_.fastSpeed_ = pressed;
        if (key == GLFW_KEY_SPACE) {
          positioner.lookAt(kInitialCameraPos, kInitialCameraTarget,
                            vec3(0.0f, 1.0f, 0.0f));
          positioner.setSpeed(vec3(0));
        }
      });

  renderer->run([&](uint32_t width, uint32_t height, float aspectRatio,
                    float deltaSeconds) {
    positioner.update(deltaSeconds, mouseState.pos, mouseState.pressedLeft);
    const glm::vec4 cameraPos = glm::vec4(camera.getPosition(), 1.0f);

    glm::mat4 p =
        glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 1000.0f);

    glm::mat4 m1 =
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
    const glm::mat4 m2 = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(),
                                     glm::vec3(0.0f, 1.0f, 0.0f));

    p[1][1] *= -1;

    pcCubemap.model = m2 * m1;
    pcCubemap.view = camera.getViewMatrix();
    pcCubemap.cameraPos = cameraPos;
    pcCubemap.proj = p;
    pcCubemap.texId = texture->getTextureIndex();
    pcCubemap.texCube = cubemapTex->getTextureIndex();

    renderer->bindRenderPipeline(pipelineSkybox);
    renderer->updatePushConstant(sizeof(pcCubemap), &pcCubemap);
    renderer->cmdDraw(36);

    pc.model = m2 * m1;
    pc.view = camera.getViewMatrix();
    pc.proj = p;
    pc.cameraPos = cameraPos;
    pc.texId = texture->getTextureIndex();
    pc.texCube = cubemapTex->getTextureIndex();
    pc.vertices = vertBuffer->gpuAddress();
    pc.indices = indexBuffer->gpuAddress();

    renderer->bindRenderPipeline(pipeline);
    renderer->updatePushConstant(sizeof(pc), &pc);
    renderer->BindDepthState(
        {.compareOp = VK_COMPARE_OP_LESS, .depthWriteEnable = true});
    renderer->cmdDraw(indices.size());
  });

  delete texture;
  delete cubemapTex;
  delete vertBuffer;
  delete indexBuffer;
  delete pipeline;
  delete pipelineSkybox;
  delete renderer;
}
