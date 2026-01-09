#pragma once

#include <glm/glm.hpp>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 acceleration;

  glm::vec3 forceAcc;

  float damping;
  float inverseMasses;

  void integrate(float duration);
};
