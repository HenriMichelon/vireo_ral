/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "vireo/backend/vulkan/Tools.h"
module vireo.vulkan.swapchains;

import vireo.vulkan.commands;
import vireo.vulkan.framedata;

namespace vireo {

    VKSwapChain::VKSwapChain(const VKPhysicalDevice& physicalDevice, const VKDevice& device, void* windowHandle):
        device{device},
        physicalDevice{physicalDevice},
        surface{physicalDevice.getSurface()},
#ifdef _WIN32
        hWnd{static_cast<HWND>(windowHandle)}
#endif
    {
        vkGetDeviceQueue(
            device.getDevice(),
            device.getPresentQueueFamilyIndex(),
            0,
            &presentQueue);
        create();
    }

    void VKSwapChain::create() {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice.getPhysicalDevice(), physicalDevice.getSurface());
        const VkSurfaceFormatKHR surfaceFormat= chooseSwapSurfaceFormat(swapChainSupport.formats);
        const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = FRAMES_IN_FLIGHT;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        {
            VkSwapchainCreateInfoKHR createInfo = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = surface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = swapChainExtent,
                .imageArrayLayers = 1,
                // VK_IMAGE_USAGE_TRANSFER_DST_BIT for Blit or Revolve (see presentToSwapChain())
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                .preTransform = swapChainSupport.capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = presentMode,
                .clipped = VK_TRUE
            };
            if (device.getPresentQueueFamilyIndex() != device.getGraphicsQueueFamilyIndex()) {
                const uint32_t queueFamilyIndices[] = {device.getPresentQueueFamilyIndex(), device.getGraphicsQueueFamilyIndex()};
                createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices   = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices   = nullptr;
            }
            // Need VK_KHR_SWAPCHAIN extension, or it will crash (no validation error)
            DieIfFailed(vkCreateSwapchainKHR(device.getDevice(), &createInfo, nullptr, &swapChain));
#ifdef _DEBUG
            vkSetObjectName(device.getDevice(), reinterpret_cast<uint64_t>(swapChain), VK_OBJECT_TYPE_SWAPCHAIN_KHR,
                "VKSwapChain");
#endif
        }
        vkGetSwapchainImagesKHR(device.getDevice(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        swapChainImageViews.resize(swapChainImages.size());
        swapChainImageFormat = surfaceFormat.format;

        vkGetSwapchainImagesKHR(device.getDevice(), swapChain, &imageCount, swapChainImages.data());
        extent      = Extent{ swapChainExtent.width, swapChainExtent.height };
        aspectRatio = static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = device.createImageView(swapChainImages[i],
                                                     swapChainImageFormat,
                                                     VK_IMAGE_ASPECT_COLOR_BIT,
                                                     1);
#ifdef _DEBUG
            vkSetObjectName(device.getDevice(), reinterpret_cast<uint64_t>(swapChainImageViews[i]), VK_OBJECT_TYPE_IMAGE_VIEW,
                "VKSwapChain Image View " + to_string(i));
            vkSetObjectName(device.getDevice(), reinterpret_cast<uint64_t>(swapChainImages[i]), VK_OBJECT_TYPE_IMAGE,
                "VKSwapChain Image " + to_string(i));
#endif
        }

        // For bliting image to swapchain
        // constexpr VkOffset3D vkOffset0{0, 0, 0};
        // const VkOffset3D     vkOffset1{
        //     static_cast<int32_t>(swapChainExtent.width),
        //     static_cast<int32_t>(swapChainExtent.height),
        //     1,
        // };
        // colorImageBlit.srcOffsets[0]                 = vkOffset0;
        // colorImageBlit.srcOffsets[1]                 = vkOffset1;
        // colorImageBlit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        // colorImageBlit.srcSubresource.mipLevel       = 0;
        // colorImageBlit.srcSubresource.baseArrayLayer = 0;
        // colorImageBlit.srcSubresource.layerCount     = 1;
        // colorImageBlit.dstOffsets[0]                 = vkOffset0;
        // colorImageBlit.dstOffsets[1]                 = vkOffset1;
        // colorImageBlit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        // colorImageBlit.dstSubresource.mipLevel       = 0;
        // colorImageBlit.dstSubresource.baseArrayLayer = 0;
        // colorImageBlit.dstSubresource.layerCount     = 1;
    }

    void VKSwapChain::recreate() {
        vkDeviceWaitIdle(device.getDevice());
        cleanup();
        create();
    }

    void VKSwapChain::cleanup() const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation#page_Recreating-the-swap-chain
        for (const auto &swapChainImageView : swapChainImageViews) {
            vkDestroyImageView(device.getDevice(), swapChainImageView, nullptr);
        }
        vkDestroySwapchainKHR(device.getDevice(), swapChain, nullptr);
    }

    VKSwapChain::SwapChainSupportDetails VKSwapChain::querySwapChainSupport(
        const VkPhysicalDevice vkPhysicalDevice,
        const VkSurfaceKHR surface) const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Querying-details-of-swap-chain-support
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice,
                                                      surface,
                                                      &presentModeCount,
                                                      details.presentModes.data());
        }
        return details;
    }

    VkSurfaceFormatKHR VKSwapChain::chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR> &availableFormats) {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Choosing-the-right-settings-for-the-swap-chain
        for (const auto &availableFormat : availableFormats) {
            // Using sRGB no-linear color space
            // https://learnopengl.com/Advanced-Lighting/Gamma-Correction
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { return availableFormat; }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR VKSwapChain::chooseSwapPresentMode(const vector<VkPresentModeKHR> &availablePresentModes) {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Presentation-mode
        constexpr  auto mode = VK_PRESENT_MODE_FIFO_KHR; //static_cast<VkPresentModeKHR>(app().getConfig().vSyncMode);
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == mode) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VKSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Swap-extent
#ifdef _WIN32
        RECT windowRect{};
        if (GetClientRect(hWnd, &windowRect) == 0) {
            die("Error getting window rect");
        }
        VkExtent2D actualExtent{
            .width = static_cast<uint32_t>(windowRect.right - windowRect.left),
            .height = static_cast<uint32_t>(windowRect.bottom - windowRect.top)
        };
#endif
        // actualExtent.width = glm::max(
        //         capabilities.minImageExtent.width,
        //         glm::min(capabilities.maxImageExtent.width, actualExtent.width));
        // actualExtent.height = glm::max(
        //         capabilities.minImageExtent.height,
        //         glm::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }

    void VKSwapChain::nextSwapChain() {
        currentFrameIndex = (currentFrameIndex + 1) % FRAMES_IN_FLIGHT;
    }

    void VKSwapChain::present(shared_ptr<FrameData>& frameData) {
        auto data = static_pointer_cast<VKFrameData>(frameData);

        {
            const VkSwapchainKHR   swapChains[] = { swapChain };
            const VkPresentInfoKHR presentInfo{
                .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores    = &data->renderFinishedSemaphore,
                .swapchainCount     = 1,
                .pSwapchains        = swapChains,
                .pImageIndices      = &data->imageIndex,
                .pResults           = nullptr // Optional
            };
            const auto result = vkQueuePresentKHR(presentQueue, &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                recreate();
            } else if (result != VK_SUCCESS) {
                die("failed to present swap chain image!");
            }
        }
    }

    void VKSwapChain::begin(shared_ptr<FrameData>&, shared_ptr<CommandList>&) {
    }

    void VKSwapChain::end(shared_ptr<FrameData>& frameData, shared_ptr<CommandList>& commandList) {
        const auto data = static_pointer_cast<VKFrameData>(frameData);
        static_pointer_cast<VKCommandList>(commandList)->pipelineBarrier(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            {
                VKCommandList::imageMemoryBarrier(swapChainImages[data->imageIndex],
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            });
    }

    bool VKSwapChain::acquire(shared_ptr<FrameData>& frameData) {
        auto data = static_pointer_cast<VKFrameData>(frameData);
        // wait until the GPU has finished rendering the frame.
        if (vkWaitForFences(device.getDevice(), 1, &data->inFlightFence, VK_TRUE, UINT64_MAX) == VK_TIMEOUT) {
            die("timeout waiting for inFlightFence");
            return false;
        }
        // get the next available swap chain image
        {
            const auto result = vkAcquireNextImageKHR(
                 device.getDevice(),
                 swapChain,
                 UINT64_MAX,
                 data->imageAvailableSemaphore,
                 VK_NULL_HANDLE,
                 &data->imageIndex);
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                recreate();
                // for (const auto &renderer : renderers) { renderer->recreateImagesResources(); }
                return false;
            }
            if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                die("failed to acquire swap chain image :", to_string(result));
            }
        }
        vkResetFences(device.getDevice(), 1, &data->inFlightFence);
        return true;
    }

    VKSwapChain::~VKSwapChain() {
        cleanup();
    }

}