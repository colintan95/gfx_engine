#ifndef RESOURCE_RESOURCE_MANAGER_GAL_H_
#define RESOURCE_RESOURCE_MANAGER_GAL_H_

#include <memory>
#include <utility>
#include <vector>
#include "resource/resource_gal.h"
#include "resource/resource_manager_base.h"

namespace gal {
class GALPlatform;
} // namespace

namespace resource {

class ResourceManagerGAL : public ResourceManagerBase {
public:
  ResourceManagerGAL(gal::GALPlatform* gal_platform);
  ~ResourceManagerGAL();

  template<typename... Args>
  HandleGALBuffer CreateBuffer(Args... args) {
    std::unique_ptr<ResourceGAL<gal::GALBuffer>> resource = 
        std::make_unique<ResourceGAL<gal::GALBuffer>>();

    gal::GALBuffer buffer;
    if (!buffer.Create(gal_platform_, args...)) {
      return HandleGAL<gal::GALBuffer>();
    }
    resource->resource_ = std::move(buffer);

    HandleGAL<gal::GALBuffer> handle = CreateHandle<HandleGAL<gal::GALBuffer>>(resource.get());
    buffer_resources_.push_back(std::move(resource));

    return std::move(handle);
  }

  template<typename... Args>
  HandleGALTexture CreateTexture(Args... args) {
    std::unique_ptr<ResourceGAL<gal::GALTexture>> resource = 
        std::make_unique<ResourceGAL<gal::GALTexture>>();

    gal::GALTexture texture;
    if (!texture.Create(gal_platform_, args...)) {
      return HandleGAL<gal::GALTexture>();
    }
    resource->resource_ = std::move(texture);

    HandleGAL<gal::GALTexture> handle = CreateHandle<HandleGAL<gal::GALTexture>>(resource.get());
     texture_resources_.push_back(std::move(resource));

    return std::move(handle);
  }

private:
  gal::GALPlatform* gal_platform_;

  std::vector<std::unique_ptr<ResourceGAL<gal::GALBuffer>>> buffer_resources_;
  std::vector<std::unique_ptr<ResourceGAL<gal::GALTexture>>> texture_resources_;
};

} // namespace

#endif // RESOURCE_RESOURCE_MANAGER_GAL_H_