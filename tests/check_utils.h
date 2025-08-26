#include <swiftnav/common.h>

#include <chrono>
#include <random>

class Random final {
 public:
  Random() : engine_{}, dist_{0, std::numeric_limits<size_t>::max()} {}

  double frand(double fmin, double fmax) {
    size_t v = dist_(engine_);
    double d = static_cast<double>(v) /
               static_cast<double>(std::numeric_limits<size_t>::max());
    return fmin + d * (fmax - fmin);
  }

  template <size_t N>
  void frand(double fmin, double fmax, double (&arr)[N]) {
    for (auto &i : arr) {
      i = frand(fmin, fmax);
    }
  }

  uint32_t sizerand(uint32_t sizemax) {
    double f = frand(0.0, 1.0);
    return static_cast<u32>(ceil(f * sizemax));
  }

 private:
  std::random_device engine_;
  std::uniform_int_distribution<size_t> dist_;
};
