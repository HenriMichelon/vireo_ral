module;
#include "Libraries.h"
import std;

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

    class CommandList {
    public:
        virtual void begin() = 0;
        virtual void end() = 0;

        virtual ~CommandList() = default;
    };

    class CommandAllocator {
    public:
        enum Type {
            GRAPHIC,
            TRANSFER,
            COMPUTE,
        };

        virtual std::shared_ptr<CommandList> createCommandList() const  = 0;

        virtual ~CommandAllocator() = default;
    };

    class Device {
    public:
        virtual void waitIdle() {}

        virtual std::shared_ptr<CommandAllocator> createCommandAllocator(CommandAllocator::Type type) const = 0;

        virtual ~Device() = default;
    };

    class SubmitQueue {
    public:
        virtual void submit(const FrameData& frameData, std::vector<std::shared_ptr<CommandList>> commandLists) = 0;

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

        auto& getExtent() const { return extent; }

        auto getCurrentFrameIndex() const { return currentFrameIndex; }

        virtual void nextSwapChain() = 0;

        virtual void acquire(FrameData& frameData) {}

        virtual void begin(FrameData& frameData, std::shared_ptr<CommandList>& commandList) {}

        virtual void end(FrameData& frameData, std::shared_ptr<CommandList>& commandList) {}

        virtual void present(FrameData& frameData) = 0;

    protected:
        Extent      extent{};
        uint32_t    currentFrameIndex{0};
    };

    class RenderingBackEnd
    {
    public:
        virtual ~RenderingBackEnd() = default;

        virtual std::shared_ptr<FrameData> createFrameData(uint32_t frameIndex) = 0;

        virtual void destroyFrameData(std::shared_ptr<FrameData> frameData) = 0;

        auto& getInstance() const { return instance; }

        auto& getPhysicalDevice() const { return physicalDevice; }

        auto& getDevice() const { return device; }

        auto& getGraphicCommandQueue() const { return graphicCommandQueue; }

        auto& getPresentCommandQueue() const { return presentCommandQueue; }

        auto& getTransferCommandQueue() const { return transferCommandQueue; }

        auto& getSwapChain() const { return swapChain; }

    protected:
        std::shared_ptr<Instance>        instance;
        std::shared_ptr<PhysicalDevice>  physicalDevice;
        std::shared_ptr<Device>          device;
        std::shared_ptr<SubmitQueue>     graphicCommandQueue;
        std::shared_ptr<SubmitQueue>     presentCommandQueue;
        std::shared_ptr<SubmitQueue>     transferCommandQueue;
        std::shared_ptr<SwapChain>       swapChain;

    };
}