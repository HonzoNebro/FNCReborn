// Run with any C++17 compiler; no SDK or game data required.
#include "../src/widescreen.h"

#include <cassert>
#include <cmath>
#include <limits>

int main() {
  using fnc_wide::Expand;
  fnc_wide::Projection out{};
  constexpr float inverse = 9.0f / 16.0f;
  constexpr double target = 3440.0 / 1440.0;
  assert(!Expand(0, inverse, 0.6f, out));
  assert(!Expand(16.0 / 9.0, inverse, 0.6f, out));
  assert(!Expand(4.0 / 3.0, inverse, 0.6f, out));
  assert(!Expand(5, inverse, 0.6f, out));
  assert(!Expand(target, 1.0f, 0.6f, out));  // Leave other camera formats alone.
  assert(!Expand(target, inverse, 0, out));
  assert(!Expand(target, inverse, 3.14f, out));
  assert(!Expand(std::numeric_limits<double>::infinity(), inverse, 0.6f, out));
  assert(!Expand(target, std::numeric_limits<float>::quiet_NaN(), 0.6f, out));
  assert(!Expand(target, inverse, std::numeric_limits<float>::quiet_NaN(), out));
  const float fovs[] = {0.38f, 0.6f, 1.3f, 2.5f};
  const double aspects[] = {target, 32.0 / 9.0, 4.0};
  for (double aspect : aspects) {
    for (float fov : fovs) {
      assert(Expand(aspect, inverse, fov, out));
      assert(std::fabs(out.inverse_aspect - 1.0 / aspect) < 1e-6);
      assert(out.horizontal_fov > fov);
      // A wider horizontal view must preserve the vertical projection scale.
      double original_vertical = std::tan(double(fov) / 2) * inverse;
      double new_vertical = std::tan(double(out.horizontal_fov) / 2) * out.inverse_aspect;
      assert(std::fabs(original_vertical - new_vertical) < 1e-5);
    }
  }
}
