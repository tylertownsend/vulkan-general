#pragma once
#include <memory>
#include <vector>

#include "engine/private/layer.h"

namespace engine {

class ApplicationStack {
 public:
  void Push(std::unique_ptr<Layer> layer);

 private:
  std::vector<std::unique_ptr<Layer>> stack_;
};
}