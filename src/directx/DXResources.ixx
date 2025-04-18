/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "vireo/backend/directx/Libraries.h"
export module vireo.directx.resources;

import vireo;

export namespace vireo {

    class DXBuffer : public Buffer {
    public:
        static constexpr D3D12_RESOURCE_STATES ResourceStates[] {
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
            D3D12_RESOURCE_STATE_INDEX_BUFFER,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        };

        DXBuffer(
            const ComPtr<ID3D12Device>& device,
            BufferType type,
            size_t size,
            size_t count,
            size_t minOffsetAlignment,
            const wstring& name);

        void map() override;

        void unmap() override;

        void write(const void* data, size_t size = WHOLE_SIZE, size_t offset = 0) override;

        const auto& getBufferView() const { return bufferView; }

        auto& getBuffer() const { return buffer; }

    private:
        ComPtr<ID3D12Device>            device;
        ComPtr<ID3D12Resource>          buffer;
        D3D12_VERTEX_BUFFER_VIEW        bufferView;
        D3D12_CONSTANT_BUFFER_VIEW_DESC bufferViewDesc;
    };

    class DXSampler : public Sampler {
    public:
        static constexpr D3D12_TEXTURE_ADDRESS_MODE addressModes[] {
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,
        };

        DXSampler(
            Filter minFilter,
            Filter magFilter,
            AddressMode addressModeU,
            AddressMode addressModeV,
            AddressMode addressModeW,
            float minLod = 0.0f,
            float maxLod = 1.0f,
            bool anisotropyEnable = true,
            MipMapMode mipMapMode = MipMapMode::LINEAR);

        const auto& getSamplerDesc() const { return samplerDesc; }

    private:
        D3D12_SAMPLER_DESC samplerDesc;
    };

    class DXImage : public Image {
    public:
        static constexpr DXGI_FORMAT dxFormats[] = {
            DXGI_FORMAT_R8_UNORM,
            DXGI_FORMAT_R8_SNORM,
            DXGI_FORMAT_R8_UINT,
            DXGI_FORMAT_R8_SINT,

            DXGI_FORMAT_R8G8_UNORM,
            DXGI_FORMAT_R8G8_SNORM,
            DXGI_FORMAT_R8G8_UINT,
            DXGI_FORMAT_R8G8_SINT,

            DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_FORMAT_R8G8B8A8_SNORM,
            DXGI_FORMAT_R8G8B8A8_UINT,
            DXGI_FORMAT_R8G8B8A8_SINT,
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,

            DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            DXGI_FORMAT_B8G8R8X8_UNORM,
            DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,

            DXGI_FORMAT_R10G10B10A2_UNORM,
            DXGI_FORMAT_R10G10B10A2_UINT,

            DXGI_FORMAT_R16_UNORM,
            DXGI_FORMAT_R16_SNORM,
            DXGI_FORMAT_R16_UINT,
            DXGI_FORMAT_R16_SINT,
            DXGI_FORMAT_R16_FLOAT,

            DXGI_FORMAT_R16G16_UNORM,
            DXGI_FORMAT_R16G16_SNORM,
            DXGI_FORMAT_R16G16_UINT,
            DXGI_FORMAT_R16G16_SINT,
            DXGI_FORMAT_R16G16_FLOAT,

            DXGI_FORMAT_R16G16B16A16_UNORM,
            DXGI_FORMAT_R16G16B16A16_SNORM,
            DXGI_FORMAT_R16G16B16A16_UINT,
            DXGI_FORMAT_R16G16B16A16_SINT,
            DXGI_FORMAT_R16G16B16A16_FLOAT,

            DXGI_FORMAT_R32_UINT,
            DXGI_FORMAT_R32_SINT,
            DXGI_FORMAT_R32_FLOAT,

            DXGI_FORMAT_R32G32_UINT,
            DXGI_FORMAT_R32G32_SINT,
            DXGI_FORMAT_R32G32_FLOAT,

            DXGI_FORMAT_R32G32B32_UINT,
            DXGI_FORMAT_R32G32B32_SINT,
            DXGI_FORMAT_R32G32B32_FLOAT,

            DXGI_FORMAT_R32G32B32A32_UINT,
            DXGI_FORMAT_R32G32B32A32_SINT,
            DXGI_FORMAT_R32G32B32A32_FLOAT,

            DXGI_FORMAT_D16_UNORM,
            DXGI_FORMAT_D24_UNORM_S8_UINT,
            DXGI_FORMAT_D32_FLOAT,
            DXGI_FORMAT_D32_FLOAT_S8X24_UINT,

            DXGI_FORMAT_BC1_UNORM,
            DXGI_FORMAT_BC1_UNORM_SRGB,
            DXGI_FORMAT_BC2_UNORM,
            DXGI_FORMAT_BC2_UNORM_SRGB,
            DXGI_FORMAT_BC3_UNORM,
            DXGI_FORMAT_BC3_UNORM_SRGB,
            DXGI_FORMAT_BC4_UNORM,
            DXGI_FORMAT_BC4_SNORM,
            DXGI_FORMAT_BC5_UNORM,
            DXGI_FORMAT_BC5_SNORM,
            DXGI_FORMAT_BC6H_UF16,
            DXGI_FORMAT_BC6H_SF16,
            DXGI_FORMAT_BC7_UNORM,
            DXGI_FORMAT_BC7_UNORM_SRGB,
        };

        DXImage(
            const ComPtr<ID3D12Device> &device,
            ImageFormat format,
            uint32_t    width,
            uint32_t    height,
            const wstring& name,
            bool        useByComputeShader,
            bool        allowRenderTarget,
            MSAA        msaa);

        auto getImage() const { return image; }

    private:
        ComPtr<ID3D12Device>            device;
        ComPtr<ID3D12Resource>          image;
    };

    class DXRenderTarget : public RenderTarget {
    public:
        DXRenderTarget(const ComPtr<ID3D12Device> &device, const shared_ptr<DXImage>& image);

        auto& getHandle() const { return handle; }

    private:
        ComPtr<ID3D12DescriptorHeap> heap;
        D3D12_CPU_DESCRIPTOR_HANDLE  handle;
    };

}