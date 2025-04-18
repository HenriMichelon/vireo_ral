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
import vireo.config;
import vireo.vulkan.devices;

export namespace vireo {

    class VKSwapChain : public SwapChain {
    public:
        static constexpr auto RENDER_FORMAT{ImageFormat::R8G8B8A8_SRGB};

        VKSwapChain(
            const shared_ptr<const VKDevice>& device,
            void* windowHandle,
            PresentMode vSyncMode);

        ~VKSwapChain() override;

        auto getSwapChain() const { return swapChain; }

        auto getFormat() const { return swapChainImageFormat; }

        // const auto& getImageViews() const { return swapChainImageViews; }

        // const auto& getImages() const { return swapChainImages; }

        auto getCurrentImage() const { return swapChainImages[imageIndex[currentFrameIndex]]; }

        auto getCurrentImageView() const { return swapChainImageViews[imageIndex[currentFrameIndex]]; }

        void nextSwapChain() override;

        bool acquire(const shared_ptr<FrameData>& frameData) override;

        void present(const shared_ptr<FrameData>& framesData) override;

        void recreate() override;

    private:
        static constexpr VkPresentModeKHR vkPresentModes[] {
            VK_PRESENT_MODE_IMMEDIATE_KHR,
            VK_PRESENT_MODE_FIFO_KHR
        };
        // For Device::querySwapChainSupport()
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR        capabilities;
            vector<VkSurfaceFormatKHR> formats;
            vector<VkPresentModeKHR>   presentModes;
        };

        const shared_ptr<const VKDevice>         device;
        const VKPhysicalDevice& physicalDevice;
        const PresentMode         vSyncMode;
        VkSwapchainKHR          swapChain;
        vector<VkImage>         swapChainImages;
        VkFormat                swapChainImageFormat;
        VkExtent2D              swapChainExtent;
        vector<VkImageView>     swapChainImageViews;
        VkQueue                 presentQueue;
        vector<uint32_t>        imageIndex;


#ifdef _WIN32
        HWND hWnd;
#endif

        void create();

        void cleanup() const;

        // Get the swap chain capabilities
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);

        // Get the swap chain format, default for sRGB/NON-LINEAR
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
                const vector<VkSurfaceFormatKHR> &availableFormats);

        // Get the swap chain present mode
        static VkPresentModeKHR chooseSwapPresentMode(
                PresentMode vSyncMode,
                const vector<VkPresentModeKHR> &availablePresentModes);

        // Get the swap chain images sizes
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    };

}