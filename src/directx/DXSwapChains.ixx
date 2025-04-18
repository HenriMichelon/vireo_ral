/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "vireo/backend/directx/Libraries.h"
export module vireo.directx.swapchains;

import vireo;
import vireo.directx.devices;

export namespace vireo {

    class DXSwapChain : public SwapChain {
    public:
        DXSwapChain(
            const ComPtr<IDXGIFactory4>& factory,
            const shared_ptr<DXDevice>& device,
            const ComPtr<ID3D12CommandQueue>& commandQueue,
            ImageFormat format,
            HWND hWnd, PresentMode vSyncMode, uint32_t framesInFlight);

        ~DXSwapChain() override;

        auto getSwapChain() { return swapChain; }

        auto getRenderTargets() const { return renderTargets; }

        auto getDescriptorSize() const {  return rtvDescriptorSize; }

        auto getHeap() const { return rtvHeap; }

        void nextSwapChain() override;

        bool acquire(const shared_ptr<Fence>& fence) override;

        void present() override;

        void recreate() override;

        void waitForLastPresentedFrame() const;

    private:
        const shared_ptr<DXDevice>     device;
        const ComPtr<IDXGIFactory4>    factory;
        ComPtr<IDXGISwapChain3>        swapChain;
        ComPtr<ID3D12CommandQueue>     presentCommandQueue;
        vector<ComPtr<ID3D12Resource>> renderTargets;
        ComPtr<ID3D12DescriptorHeap>   rtvHeap;
        UINT                           rtvDescriptorSize{0};
        HWND                           hWnd;
        const UINT                     syncInterval;
        const UINT                     presentFlags;
        HANDLE                         fenceEvent;
        shared_ptr<DXFence>            lastFence;

        void create();
    };

}