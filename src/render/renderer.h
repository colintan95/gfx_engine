#ifndef RENDER_RENDERER_H_
#define RENDER_RENDERER_H_

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "gal/gal_buffer.h"
#include "gal/gal_platform.h"
#include "gal/gal_command_buffer.h"
// #include "render/mesh.h"
#include "window/window.h"

namespace resource {
class ResourceSystem;
class ResourceManagerGAL;
} // namespace

namespace render {

class Renderer {
public:
  class InitException : public std::exception {
  public:
    const char* what() const final {
      return "Failed to initialize Renderer.";
    }
  };

public:
  Renderer(window::Window* window, resource::ResourceSystem* resource_system);
  ~Renderer();

  void Tick();

  // std::optional<MeshId> CreateMesh(const std::string& file_path);

private:  
  window::Window* window_ = nullptr;
  resource::ResourceSystem* resource_system_ = nullptr;

  gal::GALPipeline pipeline_;
  gal::GALBuffer vert_buffer_;
  gal::GALCommandBuffer command_buffer_;

  // std::unique_ptr<resource::ResourceManagerGAL> resource_manager_;
  std::unique_ptr<gal::GALPlatform> gal_platform_;
  // gal::GALCommandBuffer command_buffer_;

  // std::unordered_map<MeshId, Mesh> meshes_;
};

} // namespace

#endif // RENDER_RENDERER_H_