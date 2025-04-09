/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "backend/vulkan/Tools.h"

export module vireo.backend.vulkan;

import vireo.backend;
import vireo.backend.buffer;
import vireo.backend.framedata;
import vireo.backend.descriptors;
import vireo.backend.device;
import vireo.backend.vulkan.buffer;
import vireo.backend.vulkan.device;

export namespace vireo::backend {

    class VKSubmitQueue : public SubmitQueue {
    public:
        VKSubmitQueue(CommandList::Type type, const VKDevice& device);

        auto getCommandQueue() const { return commandQueue; }

        void submit(const FrameData& frameData, std::vector<std::shared_ptr<CommandList>> commandLists) override;

        void submit(std::vector<std::shared_ptr<CommandList>> commandLists) override;

        void waitIdle() override;

    private:
        VkQueue commandQueue;
    };

    class VKCommandAllocator : public CommandAllocator {
    public:
        VKCommandAllocator(CommandList::Type type, const VKDevice& device);
        ~VKCommandAllocator() override;

        std::shared_ptr<CommandList> createCommandList(Pipeline& pipeline) const override;

        std::shared_ptr<CommandList> createCommandList() const override;

    private:
        const VKDevice& device;
        VkCommandPool   commandPool;
    };

    class VKCommandList : public CommandList {
    public:
        VKCommandList(const VKDevice& device, VkCommandPool commandPool);

        ~VKCommandList() override;

        void reset() override;

        void begin(Pipeline& pipeline) override;

        void begin() override;

        void end() override;

        void cleanup() override;

        void bindVertexBuffer(Buffer& buffer) override;

        void drawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount = 1) override;

        void upload(Buffer& destination, const void* source) override;

        auto getCommandBuffer() const { return commandBuffer; }

        void pipelineBarrier(
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            const std::vector<VkImageMemoryBarrier>& barriers) const;

        static VkImageMemoryBarrier imageMemoryBarrier(
           VkImage image,
           VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
           VkImageLayout oldLayout, VkImageLayout newLayout,
           uint32_t baseMipLevel = 0,
           uint32_t levelCount = VK_REMAINING_MIP_LEVELS
       );

    private:
        const VKDevice& device;
        VkCommandBuffer commandBuffer;
        std::vector<VkBuffer> stagingBuffers{};
        std::vector<VkDeviceMemory> stagingBuffersMemory{};
    };

    class VKSwapChain : public SwapChain {
    public:
        VKSwapChain(const VKPhysicalDevice& physicalDevice, const VKDevice& device);

        ~VKSwapChain() override;

        auto getSwapChain() const { return swapChain; }

        auto getFormat() const { return swapChainImageFormat; }

        const auto& getImageViews() const { return swapChainImageViews; }

        const auto& getImages() const { return swapChainImages; }

        void nextSwapChain() override;

        bool acquire(FrameData& frameData) override;

        void begin(FrameData& frameData, CommandList& commandList) override;

        void end(FrameData& frameData, CommandList& commandList) override;

        void present(FrameData& framesData) override;

    private:
        // For Device::querySwapChainSupport()
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR        capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR>   presentModes;
        };

        const VKDevice&             device;
        const VKPhysicalDevice&     physicalDevice;
        VkSurfaceKHR                surface;
        VkSwapchainKHR              swapChain;
        std::vector<VkImage>        swapChainImages;
        VkFormat                    swapChainImageFormat;
        VkExtent2D                  swapChainExtent;
        std::vector<VkImageView>    swapChainImageViews;
        VkQueue                     presentQueue;

        void recreate();

        void create();

        void cleanup() const;

        // Get the swap chain capabilities
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface) const;

        // Get the swap chain format, default for sRGB/NON-LINEAR
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
                const std::vector<VkSurfaceFormatKHR> &availableFormats);

        // Get the swap chain present mode, default to MAILBOX, if not available FIFO (V-SYNC)
        static VkPresentModeKHR chooseSwapPresentMode(
                const std::vector<VkPresentModeKHR> &availablePresentModes);

        // Get the swap chain images sizes
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    };

    class VKVertexInputLayout : public VertexInputLayout {
    public:
        static constexpr VkFormat VKFormat[] {
            VK_FORMAT_R32G32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32A32_SFLOAT
        };

        VKVertexInputLayout(size_t size, const std::vector<AttributeDescription>& attributesDescriptions);

        const auto& getVertexBindingDescription() const { return vertexBindingDescription; }

        const auto& getVertexAttributeDescription() const { return vertexAttributeDescriptions; }

    private:
        VkVertexInputBindingDescription                vertexBindingDescription;
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
    };

    class VKShaderModule: public ShaderModule {
    public:
        VKShaderModule(VkDevice device, const std::string& fileName);

        ~VKShaderModule() override;

        auto getShaderModule() const { return shaderModule; }

    private:
        VkDevice       device;
        VkShaderModule shaderModule;
    };

    class VKPipelineResources : public PipelineResources {
    public:
        VKPipelineResources(VkDevice device);

        ~VKPipelineResources() override;

        auto getPipelineLayout() const { return pipelineLayout; }

    private:
        VkDevice         device;
        VkPipelineLayout pipelineLayout;
    };

    class VKPipeline : public Pipeline {
    public:
        VKPipeline(
           VkDevice device,
           VKSwapChain& swapChain,
           PipelineResources& pipelineResources,
           VertexInputLayout& vertexInputLayout,
           ShaderModule& vertexShader,
           ShaderModule& fragmentShader,
           const std::wstring& name);

        auto getPipeline() const { return pipeline; }

        ~VKPipeline() override;

    private:
        VkDevice     device;
        VkPipeline   pipeline;
    };

    class VKRenderingBackEnd : public RenderingBackEnd {
    public:
        VKRenderingBackEnd();

        void waitIdle() override;

        std::shared_ptr<CommandAllocator> createCommandAllocator(CommandList::Type type) const override;

        std::shared_ptr<FrameData> createFrameData(uint32_t frameIndex) override;

        void destroyFrameData(FrameData& frameData) override;

        std::shared_ptr<VertexInputLayout> createVertexLayout(
            size_t size,
            const std::vector<VertexInputLayout::AttributeDescription>& attributesDescriptions) const override;

        std::shared_ptr<ShaderModule> createShaderModule(const std::string& fileName) const override;

        std::shared_ptr<PipelineResources> createPipelineResources() const override;

        std::shared_ptr<Pipeline> createPipeline(
            PipelineResources& pipelineResources,
            VertexInputLayout& vertexInputLayout,
            ShaderModule& vertexShader,
            ShaderModule& fragmentShader,
            const std::wstring& name = L"Pipeline") const override;

        std::shared_ptr<Buffer> createBuffer(Buffer::Type type, size_t size, size_t count = 1, size_t alignment = 1, const std::wstring& name = L"Buffer") const override;

        std::shared_ptr<DescriptorAllocator> createDescriptorAllocator(DescriptorType type, uint32_t capacity) override;

        void beginRendering(FrameData& frameData, PipelineResources& pipelineResources, Pipeline& pipeline, CommandList& commandList) override;

        void endRendering(CommandList& commandList) override;

        auto getVKInstance() const { return std::reinterpret_pointer_cast<VKInstance>(instance); }

        auto getVKPhysicalDevice() const { return std::reinterpret_pointer_cast<VKPhysicalDevice>(physicalDevice); }

        auto getVKDevice() const { return std::reinterpret_pointer_cast<VKDevice>(device); }

        auto getVKSwapChain() const { return std::reinterpret_pointer_cast<VKSwapChain>(swapChain); }

        auto getVKGraphicCommandQueue() const { return std::reinterpret_pointer_cast<VKSubmitQueue>(graphicCommandQueue); }

    private:
        const VkClearValue depthClearValue{.depthStencil = {1.0f, 0}};
    };

}