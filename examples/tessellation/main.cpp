#include "mai_renderer.h"

int main() {
  MAI::MAIRendererInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Asobi",
  };

  MAI::MAIRenderer *renderer = new MAI::MAIRenderer(info);

  renderer->run([&](uint32_t width, uint32_t height, float aspectRatio,
                    float deltaSeconds) {});
  delete renderer;
}
