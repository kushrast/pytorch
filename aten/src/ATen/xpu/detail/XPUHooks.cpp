#include <ATen/xpu/PinnedMemoryAllocator.h>
#include <ATen/xpu/XPUContext.h>
#include <ATen/xpu/XPUDevice.h>
#include <ATen/xpu/XPUGeneratorImpl.h>
#include <ATen/xpu/detail/XPUHooks.h>
#include <c10/util/CallOnce.h>
#include <c10/util/Logging.h>
#include <c10/xpu/XPUCachingAllocator.h>

namespace at::xpu::detail {

void XPUHooks::initXPU() const {
  C10_LOG_API_USAGE_ONCE("aten.init.xpu");
  const auto device_count = c10::xpu::device_count_ensure_non_zero();
  c10::xpu::XPUCachingAllocator::init(device_count);
}

bool XPUHooks::hasXPU() const {
  return true;
}

std::string XPUHooks::showConfig() const {
  return "XPU backend";
}

int32_t XPUHooks::getGlobalIdxFromDevice(const at::Device& device) const {
  TORCH_CHECK(device.is_xpu(), "Only the XPU device type is expected.");
  return at::xpu::getGlobalIdxFromDevice(device.index());
}

Generator XPUHooks::getXPUGenerator(DeviceIndex device_index) const {
  return make_generator<at::XPUGeneratorImpl>(device_index);
}

const Generator& XPUHooks::getDefaultXPUGenerator(
    DeviceIndex device_index) const {
  return at::xpu::detail::getDefaultXPUGenerator(device_index);
}

Device XPUHooks::getDeviceFromPtr(void* data) const {
  return at::xpu::getDeviceFromPtr(data);
}

/**
 * DEPRECATED: use deviceCount() instead
 */
c10::DeviceIndex XPUHooks::getNumGPUs() const {
  return at::xpu::device_count();
}

/**
 * DEPRECATED: use getCurrentDevice() instead
 */
DeviceIndex XPUHooks::current_device() const {
  return c10::xpu::current_device();
}

void XPUHooks::deviceSynchronize(DeviceIndex device_index) const {
  // Only the SYCL queues we have reserved will be synchronized, see Note
  // [Synchronize Streams on Device].
  c10::xpu::syncStreamsOnDevice(device_index);
}

Allocator* XPUHooks::getPinnedMemoryAllocator() const {
  return at::xpu::getPinnedMemoryAllocator();
}

bool XPUHooks::isPinnedPtr(const void* data) const {
  if (!at::xpu::is_available()) {
    return false;
  }

  return sycl::usm::alloc::host ==
      sycl::get_pointer_type(data, c10::xpu::get_device_context());
}

bool XPUHooks::hasPrimaryContext(DeviceIndex device_index) const {
  // The default context is utilized for each device. So it always returns true.
  return true;
}

DeviceIndex XPUHooks::deviceCount() const {
  return at::xpu::device_count();
}

DeviceIndex XPUHooks::getCurrentDevice() const {
  return at::xpu::current_device();
}

REGISTER_XPU_HOOKS(XPUHooks);

} // namespace at::xpu::detail
