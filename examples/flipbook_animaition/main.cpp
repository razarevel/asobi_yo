#include "mai_renderer.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstdint>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using glm::mat4;

const double kAnimationFPS = 50.0f;
const uint32_t kNumFlipBooks = 3;
const uint32_t kNumFlipbookFrames = 100;

struct AnimateState {
  glm::vec2 position = glm::vec2(0.0f);
  double startTime = 0;
  float time = 0;
  uint32_t textureIndex = 0;
  uint32_t firstFrame = 0;
};

std::vector<AnimateState> g_Animations;
std::vector<AnimateState> g_AnimationKeyFrames;
float timelineOffset = 0.0f;
float showTimeline = false;

void updateAnimation(float time);
void setAnimationOffset(float offset);

int main() {
  MAI::MAIRendererInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Asobi",
  };

  MAI::MAIRenderer *renderer = new MAI::MAIRenderer(info);

  std::vector<MAI::VKTexture *> textures;
  textures.reserve(kNumFlipBooks * kNumFlipbookFrames);

  for (uint32_t book = 0; book != kNumFlipBooks; book++)
    for (uint32_t frame = 0; frame != kNumFlipbookFrames; frame++) {
      char fname[1024];
      snprintf(fname, sizeof(fname),
               RESOURCES_PATH "explosion0%01u/explosion%02u-frame%03u.tga",
               book, book, frame + 1);

      int w, h, comp;
      const stbi_uc *pixels = stbi_load(fname, &w, &h, &comp, 4);
      assert(pixels);
      textures.push_back(renderer->createTexture({
          .width = (uint32_t)w,
          .height = (uint32_t)h,
          .data = pixels,
      }));

      stbi_image_free((void *)pixels);
    }

  MAI::VKShader *vert =
      renderer->createShader(EXAMPLES_DIR "flipbook_animaition/spvs/main.vspv");
  MAI::VKShader *frag =
      renderer->createShader(EXAMPLES_DIR "flipbook_animaition/spvs/main.fspv");

  struct PushConstat {
    glm::mat4 proj;
    uint32_t textureId;
    glm::vec2 pos;
    glm::vec2 size;
    float alphaScale;
  } pc;

  MAI::VKPipeline *pipeline = renderer->createPipeline({
      .vert = vert,
      .frag = frag,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
      .cullMode = VK_CULL_MODE_NONE,
      .color =
          {
              .blendEnable = true,
              .srcColorBlend = VK_BLEND_FACTOR_SRC_ALPHA,
              .dstColorBlend = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
          },
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

  glfwSetMouseButtonCallback(renderer->getWindow(), [](GLFWwindow *window,
                                                       int button, int action,
                                                       int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      double xpos, ypos;
      int w, h;
      glfwGetWindowSize(window, &w, &h);
      glfwGetCursorPos(window, &xpos, &ypos);
      float x = static_cast<float>(xpos / w);
      float y = static_cast<float>(ypos / h);
      g_Animations.push_back(AnimateState{
          .position = glm::vec2(x, y),
          .startTime = glfwGetTime(),
          .textureIndex = 0,
          .firstFrame = kNumFlipbookFrames * (uint32_t)(rand() % kNumFlipBooks),
      });
    }
  });

  g_Animations.push_back(AnimateState{
      .position = glm::vec2(0.5f, 0.5f),
      .startTime = glfwGetTime(),
      .textureIndex = 0,
      .firstFrame = kNumFlipbookFrames * (uint32_t)(rand() & kNumFlipBooks),
  });

  renderer->run([&](uint32_t width, uint32_t height, float aspectRatio,
                    float deltaSeconds) {
    updateAnimation(deltaSeconds);

    auto easing = [](float t) -> float {
      const float p1 = 0.1f;
      const float p2 = 0.8f;
      if (t <= p1)
        return glm::smoothstep(0.0f, 1.0f, t / p1);
      if (t >= p2)
        return glm::smoothstep(1.0f, 0.0f, (t - p2) / (1.0f - p2));

      return 1.0f;
    };

    renderer->bindRenderPipeline(pipeline);
    for (const AnimateState &s : g_Animations) {
      const float t = s.time / (kNumFlipbookFrames / kAnimationFPS);
      pc = {
          .proj = glm::ortho(0.0f, float(width), 0.0f, float(height)),
          .textureId = textures[s.textureIndex]->getTextureIndex(),
          .pos = s.position * glm::vec2(width, height),
          .size = glm::vec2(height * 0.5f),
          .alphaScale = easing(t),
      };

      renderer->updatePushConstant(sizeof(pc), &pc);
      renderer->cmdDraw(4);
    }
  });

  for (auto &it : textures)
    delete it;

  delete pipeline;
  delete renderer;
}

void updateAnimation(float deltaSecond) {
  for (size_t i = 0; i < g_Animations.size();) {
    g_Animations[i].time += deltaSecond;
    g_Animations[i].textureIndex =
        g_Animations[i].firstFrame +
        (uint32_t)(kAnimationFPS * g_Animations[i].time);

    uint32_t frame = (uint32_t)(kAnimationFPS * g_Animations[i].time);
    if (frame >= kNumFlipbookFrames) {
      g_Animations.erase(g_Animations.begin() + i);
      continue;
    }
    i++;
  }
}

void setAnimationOffset(float offset) {
  for (size_t i = 0; i < g_Animations.size(); i++) {
    g_Animations[i].time =
        std::max(g_AnimationKeyFrames[i].time + offset, 0.0f);
    g_Animations[i].textureIndex =
        g_Animations[i].firstFrame +
        (uint32_t)(kAnimationFPS * g_Animations[i].time);
    g_Animations[i].textureIndex =
        std::min(g_Animations[i].textureIndex,
                 kNumFlipbookFrames + g_Animations[i].firstFrame - 1);
  }
}
