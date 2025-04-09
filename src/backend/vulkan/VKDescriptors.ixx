/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "backend/vulkan/Libraries.h"
export module vireo.backend.vulkan.descriptors;

import vireo.backend.buffer;
import vireo.backend.descriptors;

export namespace vireo::backend {

    class VKDescriptorAllocator : public DescriptorAllocator {
    public:
        VKDescriptorAllocator(DescriptorType type, VkDevice device, uint32_t capacity);

        ~VKDescriptorAllocator() override;

        void update(DescriptorHandle handle, Buffer& buffer) override;

        uint64_t getGPUHandle(DescriptorHandle handle) const override;

    private:
        VkDevice              device;
        VkDescriptorPool      pool;
        VkDescriptorSetLayout setLayout;
        VkDescriptorSet       set;
    };

}