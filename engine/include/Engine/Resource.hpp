#pragma once

namespace e00 {

struct Resource {
  Resource() = default;
  Resource(const Resource &other) = default;


  virtual ~Resource() = default;

private:
};
}// namespace e00
