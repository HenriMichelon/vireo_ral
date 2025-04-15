/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "vireo/backend/vulkan/Tools.h"
export module vireo.vulkan.commands;

import vireo;

import vireo.vulkan.devices;
import vireo.vulkan.descriptors;
import vireo.vulkan.resources;

export namespace vireo {

    class VKSubmitQueue : public SubmitQueue {
    public:
        VKSubmitQueue(const shared_ptr<const VKDevice>& device, CommandType type, const string& name);

        auto getCommandQueue() const { return commandQueue; }

        void submit(const shared_ptr<const FrameData>& frameData, const vector<shared_ptr<const CommandList>>& commandLists) const override;

        void submit(const vector<shared_ptr<const CommandList>>& commandLists) const override;

        void waitIdle() const override;

    private:
        VkQueue commandQueue;
    };

    class VKCommandAllocator : public CommandAllocator {
    public:
        VKCommandAllocator(const shared_ptr<const VKDevice>& device, CommandType type);

        ~VKCommandAllocator() override;

        void reset() const override;

        shared_ptr<CommandList> createCommandList(const shared_ptr<const Pipeline>& pipeline) const override;

        shared_ptr<CommandList> createCommandList() const override;

    private:
        const shared_ptr<const VKDevice> device;
        VkCommandPool                    commandPool;
    };

    class VKCommandList : public CommandList {
    public:
        static constexpr VkPrimitiveTopology vkPrimitives[]{
            VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
        };

        VKCommandList(const shared_ptr<const VKDevice>& device, VkCommandPool commandPool);

        void begin() const override;

        void end() const override;

        void cleanup() override;

        void upload(const shared_ptr<const Buffer>& destination, const void* source) override;

        void upload(const shared_ptr<const Image>& destination, const void* source) override;

        void bindVertexBuffer(const shared_ptr<const Buffer>& buffer) const override;

        void bindPipeline(const shared_ptr<const Pipeline>& pipeline) override;

        void bindDescriptors(const vector<shared_ptr<const DescriptorSet>>& descriptors) const override;

        void drawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount = 1) const override;

        void setViewports(uint32_t count, const vector<Extent>& extent) const override;

        void setScissors(uint32_t count, const vector<Extent>& extent) const override;

        void setPrimitiveTopology(PrimitiveTopology primitiveTopology) const override;

        auto getCommandBuffer() const { return commandBuffer; }

        void pipelineBarrier(
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            const vector<VkImageMemoryBarrier>& barriers) const;

        static VkImageMemoryBarrier imageMemoryBarrier(
           VkImage image,
           VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
           VkImageLayout oldLayout, VkImageLayout newLayout,
           uint32_t baseMipLevel = 0,
           uint32_t levelCount = VK_REMAINING_MIP_LEVELS
       );

    private:
        const shared_ptr<const VKDevice> device;
        VkCommandBuffer                  commandBuffer;
        VkPipelineLayout                 lastBoundLayout{VK_NULL_HANDLE};
        vector<VkBuffer>                 stagingBuffers{};
        vector<VkDeviceMemory>           stagingBuffersMemory{};
    };

}