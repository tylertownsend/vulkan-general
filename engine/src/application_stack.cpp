#include "engine/private/application_stack.h"

namespace engine {

void ApplicationStack::Push(std::unique_ptr<Layer> layer) {
  stack_.emplace_back(std::move(layer));
}
}