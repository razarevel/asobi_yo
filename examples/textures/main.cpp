#include "mai_renderer.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using glm::mat4;
using glm::vec3;

int main() {
  MAI::MAIRendererInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Asobi yo",
  };
  MAI::MAIRenderer *renderer = new MAI::MAIRenderer(info);

  MAI::VKShader *vert =
      renderer->createShader(EXAMPLES_DIR "textures/spvs/main.vspv");
  MAI::VKShader *frag =
      renderer->createShader(EXAMPLES_DIR "textures/spvs/main.fspv");

  MAI::VKPipeline *pipeline = renderer->createPipeline({
      .vert = vert,
      .frag = frag,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
      .cullMode = VK_CULL_MODE_NONE,
      .pushConstants =
          {
              .stageFlags =
                  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
              .offset = 0,
              .size = sizeof(glm::mat4) + sizeof(uint32_t),
          },
  });
  delete vert;
  delete frag;

  int w, h, comp;
  const uint8_t *img = stbi_load(RESOURCES_PATH "wood.jpg", &w, &h, &comp, 4);
  assert(img);
  // create texture
  MAI::VKTexture *texture = renderer->createTexture({
      .width = (uint32_t)w,
      .height = (uint32_t)h,
      .data = img,
  });

  stbi_image_free((void *)img);

  renderer->run([&](uint32_t width, uint32_t height, float aspectRatio,
                    float deltaSeconds) {
    const mat4 m =
        glm::rotate(mat4(1.0f), (float)glfwGetTime(), vec3(0.0f, 0.0f, 1.0f));
    const mat4 p = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f, 1.f, -1.f);

    struct PushConstant {
      mat4 mvp;
      uint32_t texture;
    } pc = {
        .mvp = p * m,
        .texture = texture->getTextureIndex(),
    };

    renderer->bindRenderPipeline(pipeline);
    renderer->updatePushConstant(sizeof(pc), &pc);
    renderer->cmdDraw(6);
  });

  delete texture;
  delete pipeline;
  delete renderer;
}
