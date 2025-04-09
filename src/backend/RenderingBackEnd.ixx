module;
#include "Tools.h"

export module dxvk.backend;

import dxvk.backend.framedata;

export namespace dxvk::backend {

    class Instance {
    public:
        virtual ~Instance() = default;
    };

    class PhysicalDevice {
    public:
        virtual ~PhysicalDevice() = default;
    };

    class Buffer {
    public:
        enum Type {
            VERTEX,
            INDEX,
            UNIFORM
        };

        static constexpr size_t WHOLE_SIZE = ~0ULL;

        Buffer(const Type type): type{type} {}

        virtual ~Buffer() = default;

        auto getSize() const { return bufferSize; }

        auto getType() const { return type; }

        virtual void map() = 0;

        virtual void unmap() = 0;

        virtual void write(const void* data, size_t size = WHOLE_SIZE, size_t offset = 0) = 0;

    protected:
        Type    type;
        size_t  bufferSize{0};
        size_t  alignmentSize{0};
        void*   mappedAddress{nullptr};
    };

    class VertexInputLayout {
    public:
        enum AttributeFormat {
            R32G32_FLOAT,
            R32G32B32_FLOAT,
            R32G32B32A32_FLOAT,
        };
        struct AttributeDescription {
            std::string     binding;
            AttributeFormat format;
            uint32_t        offset;
        };
        virtual ~VertexInputLayout() = default;
    };

    class ShaderModule {
    public:
        virtual ~ShaderModule() = default;
    };

    class PipelineResources {
    public:
        virtual ~PipelineResources() = default;
    };

    class Pipeline {
    public:
        virtual ~Pipeline() = default;
    };

    class CommandList {
    public:
        enum Type {
            GRAPHIC,
            TRANSFER,
            COMPUTE,
        };

        virtual void begin(Pipeline& pipeline) = 0;

        virtual void reset() = 0;

        virtual void begin() = 0;

        virtual void end() = 0;

        virtual void bindVertexBuffer(Buffer& buffer) = 0;

        virtual void drawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount = 1) = 0;

        virtual void upload(Buffer& destination, const void* source) = 0;

        virtual void cleanup() = 0;

        virtual ~CommandList() = default;
    };

    class CommandAllocator {
    public:
        CommandAllocator(const CommandList::Type type) : commandListType{type} {}

        virtual std::shared_ptr<CommandList> createCommandList(Pipeline& pipeline) const  = 0;

        virtual std::shared_ptr<CommandList> createCommandList() const  = 0;

        virtual ~CommandAllocator() = default;

        auto getCommandListType() const { return commandListType; }

    private:
        CommandList::Type commandListType;
    };

    class Device {
    public:
        virtual ~Device() = default;
    };

    class SubmitQueue {
    public:
        virtual void submit(const FrameData& frameData, std::vector<std::shared_ptr<CommandList>> commandLists) = 0;

        virtual void submit(std::vector<std::shared_ptr<CommandList>> commandLists) = 0;

        virtual void waitIdle() = 0;

        virtual ~SubmitQueue() = default;
    };

    class SwapChain {
    public:
        static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

        struct Extent {
            uint32_t width;
            uint32_t height;
        };

        virtual ~SwapChain() = default;

        const auto& getExtent() const { return extent; }

        auto getCurrentFrameIndex() const { return currentFrameIndex; }

        virtual void nextSwapChain() = 0;

        virtual void acquire(FrameData& frameData) = 0;

        virtual void begin(FrameData& frameData, CommandList& commandList) {}

        virtual void end(FrameData& frameData, CommandList& commandList) {}

        virtual void present(FrameData& frameData) = 0;

    protected:
        Extent      extent{};
        uint32_t    currentFrameIndex{0};
    };

    class RenderingBackEnd {
    public:
        virtual ~RenderingBackEnd() = default;

        virtual std::shared_ptr<FrameData> createFrameData(uint32_t frameIndex) = 0;

        virtual void destroyFrameData(FrameData& frameData) {}

        virtual void waitIdle() = 0;

        virtual std::shared_ptr<CommandAllocator> createCommandAllocator(CommandList::Type type) const = 0;

        virtual std::shared_ptr<VertexInputLayout> createVertexLayout(
            size_t size,
            const std::vector<VertexInputLayout::AttributeDescription>& attributesDescriptions) const = 0;

        virtual std::shared_ptr<ShaderModule> createShaderModule(
            const std::string& fileName) const = 0;

        virtual std::shared_ptr<PipelineResources> createPipelineResources() const = 0;

        virtual std::shared_ptr<Pipeline> createPipeline(
            PipelineResources& pipelineResources,
            VertexInputLayout& vertexInputLayout,
            ShaderModule& vertexShader,
            ShaderModule& fragmentShader) const = 0;

        virtual std::shared_ptr<Buffer> createBuffer(Buffer::Type type, size_t size, size_t count = 1, size_t alignment = 1, const std::wstring& name = L"Buffer") const = 0;

        virtual void beginRendering(FrameData& frameData, PipelineResources& pipelineResources, Pipeline& pipeline, CommandList& commandList) = 0;

        virtual void endRendering(CommandList& commandList) = 0;

        void setClearColor(const glm::vec4& color) { clearColor = color; }

        auto& getInstance() const { return instance; }

        auto& getPhysicalDevice() const { return physicalDevice; }

        auto& getDevice() const { return device; }

        auto& getGraphicCommandQueue() const { return graphicCommandQueue; }

        auto& getTransferCommandQueue() const { return transferCommandQueue; }

        auto& getSwapChain() const { return swapChain; }

    protected:
        glm::vec4                        clearColor{0.0f};
        std::shared_ptr<Instance>        instance;
        std::shared_ptr<PhysicalDevice>  physicalDevice;
        std::shared_ptr<Device>          device;
        std::shared_ptr<SubmitQueue>     graphicCommandQueue;
        std::shared_ptr<SubmitQueue>     transferCommandQueue;
        std::shared_ptr<SwapChain>       swapChain;

    };
}