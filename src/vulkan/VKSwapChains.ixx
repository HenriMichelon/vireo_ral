/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "vireo/backend/vulkan/Libraries.h"
export module vireo.vulkan.swapchains;

import vireo;
import vireo.vulkan.devices;

export namespace vireo {

    class VKSwapChain : public SwapChain {
    public:
        VKSwapChain(
            const std::shared_ptr<const VKDevice>& device,
            VkQueue presentQueue,
            void* windowHandle,
            ImageFormat format,
            PresentMode vSyncMode,
            uint32_t framesInFlight);

        ~VKSwapChain() override;

        auto getSwapChain() const { return swapChain; }

        auto getCurrentImage() const { return swapChainImages[imageIndex[currentFrameIndex]]; }

        auto getCurrentImageView() const { return swapChainImageViews[imageIndex[currentFrameIndex]]; }

        void nextFrameIndex() override;

        bool acquire(const std::shared_ptr<Fence>& fence) override;

        void present() override;

        void recreate() override;

        const auto& getCurrentImageAvailableSemaphoreInfo() const { return imageAvailableSemaphoreInfo[currentFrameIndex]; }

        const auto& getCurrentRenderFinishedSemaphoreInfo() const { return renderFinishedSemaphoreInfo[currentFrameIndex]; }

        void waitIdle() const override { vkDeviceWaitIdle(device->getDevice()); }

    private:
        static constexpr VkPresentModeKHR vkPresentModes[] {
            VK_PRESENT_MODE_IMMEDIATE_KHR,
            VK_PRESENT_MODE_FIFO_KHR
        };
        // For Device::querySwapChainSupport()
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR   capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR>   presentModes;
        };

        const std::shared_ptr<const VKDevice> device;
        VkSurfaceKHR            surface;
        VkSwapchainKHR          swapChain{VK_NULL_HANDLE};
        std::vector<VkImage>         swapChainImages;
        VkFormat                swapChainImageFormat;
        VkExtent2D              swapChainExtent;
        std::vector<VkImageView>     swapChainImageViews;
        VkQueue                 presentQueue;
        std::vector<uint32_t>        imageIndex;
        std::vector<VkSemaphore>     imageAvailableSemaphore;
        std::vector<VkSemaphore>     renderFinishedSemaphore;
        std::vector<VkSemaphoreSubmitInfo> imageAvailableSemaphoreInfo;
        std::vector<VkSemaphoreSubmitInfo> renderFinishedSemaphoreInfo;

#ifdef _WIN32
        HWND hWnd;
#endif

        void create();

        void cleanup() const;

        // Get the swap chain capabilities
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);

        // Get the swap chain format
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;

        // Get the swap chain present mode
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const;

        // Get the swap chain images sizes
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    };

}