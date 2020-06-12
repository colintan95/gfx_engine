#include "platform/glfw/input_source_glfw.h"

#include <memory>

namespace input {
namespace internal {

std::unique_ptr<InputSource> CreateInputSource() {
  return std::make_unique<InputSourceGLFW>();
}

} // namespace
} // namespace