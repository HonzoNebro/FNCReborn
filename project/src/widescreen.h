// Original projection math for the optional widescreen camera experiment.
#pragma once

#include <cmath>

namespace fnc_wide {
struct Projection {
  float inverse_aspect;
  float horizontal_fov;
};

// The supported game's camera stores H/W and horizontal FOV in radians.
// Preserve tan(vertical FOV / 2) while exposing more horizontal view.
inline bool Expand(double target_aspect, float inverse_aspect, float horizontal_fov,
                   Projection& out) {
  constexpr double pi = 3.14159265358979323846;
  constexpr double original_inverse = 9.0 / 16.0;
  if (!std::isfinite(target_aspect) || target_aspect <= 16.0 / 9.0 + 1e-5 ||
      target_aspect > 4.0 || !std::isfinite(inverse_aspect) ||
      std::fabs(inverse_aspect - original_inverse) > 0.001 ||
      !std::isfinite(horizontal_fov) || horizontal_fov <= 0.01 ||
      horizontal_fov >= pi - 0.01) {
    return false;
  }
  double target_inverse = 1.0 / target_aspect;
  double expanded = 2.0 * std::atan(std::tan(double(horizontal_fov) * 0.5) *
                                    double(inverse_aspect) / target_inverse);
  if (!std::isfinite(expanded) || expanded >= pi - 0.01) return false;
  out = {float(target_inverse), float(expanded)};
  return true;
}
}  // namespace fnc_wide
