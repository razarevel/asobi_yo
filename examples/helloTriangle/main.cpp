#include "mai_renderer.h"

int main() {
  MAI::MAIRendererInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Asobi yo",
  };

  MAI::MAIRenderer *renderer = new MAI::MAIRenderer(info);

  MAI::VKShader *vert =
      renderer->createShader(EXAMPLES_DIR "helloTriangle/spvs/main.vspv");
  MAI::VKShader *frag =
      renderer->createShader(EXAMPLES_DIR "helloTriangle/spvs/main.fspv");

  MAI::VKPipeline *pipeline = renderer->createPipeline({
      .vert = vert,
      .frag = frag,
      .cullMode = VK_CULL_MODE_NONE,
  });
  delete vert;
  delete frag;

  renderer->run([&](uint32_t width, uint32_t height, float aspectRatio,
                    float deltaSeconds) {
    renderer->bindRenderPipeline(pipeline);
    renderer->cmdDraw(3, 1, 0, 0);
  });

  delete pipeline;
  delete renderer;
}
