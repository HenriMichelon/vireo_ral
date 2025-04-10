/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "backend/vulkan/Tools.h"

export module vireo.backend.vulkan.buffer;

import vireo.backend.buffer;
import vireo.backend.vulkan.device;

export namespace vireo::backend {

    class VKBuffer : public Buffer {
    public:
        VKBuffer(
            const VKDevice& device,
            Type type,
            size_t size,
            size_t count,
            size_t minOffsetAlignment,
            const std::wstring& name);

        ~VKBuffer() override;

        void map() override;

        void unmap() override;

        void write(const void* data, size_t size = WHOLE_SIZE, size_t offset = 0) override;

        inline auto getBuffer() const { return buffer; }

        static void createBuffer(
            const VKDevice& device,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            uint32_t memoryTypeIndex,
            VkBuffer& buffer,
            VkDeviceMemory& memory);

    private:
        const VKDevice& device;
        VkBuffer        buffer{VK_NULL_HANDLE};
        VkDeviceMemory  bufferMemory{VK_NULL_HANDLE};
    };

}