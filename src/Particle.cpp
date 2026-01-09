#include "Particle.h"
#include <cassert>
#include <cmath>

void Particle::integrate(float duration) {
  assert(duration > 0.0f);
  position += velocity * duration;
  glm::vec3 resultingAcc = acceleration;
  resultingAcc += forceAcc * inverseMasses;

  velocity += resultingAcc * duration;
  velocity *= powf(damping, duration);
}
