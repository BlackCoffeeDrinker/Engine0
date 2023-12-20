#include <catch2/catch.hpp>

#include <Engine.hpp>

namespace platform {
std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &) {
  return nullptr;
}
}// namespace platform

namespace {
double ColorCompare_REF(int r1, int g1, int b1, int r2, int g2, int b2) {
  double luma1 = (r1 * 299 + g1 * 587 + b1 * 114) / (255.0 * 1000);
  double luma2 = (r2 * 299 + g2 * 587 + b2 * 114) / (255.0 * 1000);
  double lumadiff = luma1 - luma2;
  double diffR = (r1 - r2) / 255.0, diffG = (g1 - g2) / 255.0, diffB = (b1 - b2) / 255.0;
  return (diffR * diffR * 0.299 + diffG * diffG * 0.587 + diffB * diffB * 0.114) * 0.75
         + lumadiff * lumadiff;
}
}// namespace
