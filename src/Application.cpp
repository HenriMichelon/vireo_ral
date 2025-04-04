module;
#include "Libraries.h"
#include "Tools.h"

module dxvk.app;

import dxvk.app.win32;
import dxvk.backend.directx;
import dxvk.backend.vulkan;

namespace dxvk {

    Application::Application(UINT width, UINT height, std::wstring name) :
        m_width(width),
        m_height(height),
        m_title(name) {
        m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        // renderingBackEnd = std::make_shared<backend::VKRenderingBackEnd>(width, height);
        renderingBackEnd = std::make_shared<backend::DXRenderingBackEnd>(width, height);
    }

    void Application::OnInit() {
        for (uint32_t i = 0; i < backend::SwapChain::FRAMES_IN_FLIGHT; i++) {
            framesData[i] = renderingBackEnd->createFrameData(i);
            graphicCommandAllocator[i] = renderingBackEnd->getDevice()->createCommandAllocator(backend::CommandAllocator::GRAPHIC);
            graphicCommandList[i] = graphicCommandAllocator[i]->createCommandList();
        }
        pipelineResources["default"] = renderingBackEnd->createPipelineResources();
        auto attributes = std::vector{
            backend::VertexInputLayout::AttributeDescription{"POSITION", backend::VertexInputLayout::R32G32B32_FLOAT, 0},
            backend::VertexInputLayout::AttributeDescription{"COLOR",    backend::VertexInputLayout::R32G32B32_FLOAT, 12}
        };
        auto defaultVertexInputLayout = renderingBackEnd->createVertexLayout(sizeof(Vertex), attributes);
        // dxc -T "fs_5_0" -E "PSMain" -Fo shaders1_frag.cso .\shaders1.hlsl
        auto vertexShader = renderingBackEnd->createShaderModule("shaders/shaders1_vert.cso", "VSMain");
        // dxc -T "ps_5_0" -E "PSMain" -Fo shaders1_frag.cso .\shaders1.hlsl
        auto fragmentShader = renderingBackEnd->createShaderModule("shaders/shaders1_frag.cso", "PSMain");
        pipelines["default"] = renderingBackEnd->createPipeline(
            *pipelineResources["default"],
            *defaultVertexInputLayout,
            *vertexShader,
            *fragmentShader);
    }

    void Application::OnUpdate() {
    }

    void Application::OnRender() {
        auto& swapChain = renderingBackEnd->getSwapChain();
        auto& frameData = *framesData[swapChain->getCurrentFrameIndex()];
        auto& commandList = graphicCommandList[swapChain->getCurrentFrameIndex()];

        swapChain->acquire(frameData);

        commandList->begin();
        swapChain->begin(frameData, commandList);
        //draw
        swapChain->end(frameData, commandList);
        commandList->end();
        renderingBackEnd->getGraphicCommandQueue()->submit(frameData, {commandList});

        swapChain->present(frameData);
        swapChain->nextSwapChain();
    }

    void Application::OnDestroy() {
        renderingBackEnd->getDevice()->waitIdle();
        for (uint32_t i = 0; i < backend::SwapChain::FRAMES_IN_FLIGHT; i++) {
            renderingBackEnd->destroyFrameData(framesData[i]);
        }
    }

    // Helper function for setting the window's title text.
    void Application::SetCustomWindowText(LPCWSTR text) {
        std::wstring windowText = m_title + L": " + text;
        SetWindowText(Win32Application::getHwnd(), windowText.c_str());
    }

    // Generate a simple black and white checkerboard texture.
    std::vector<UINT8> Application::GenerateTextureData() {
        const UINT rowPitch = TextureWidth * TexturePixelSize;
        const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
        const UINT cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
        const UINT textureSize = rowPitch * TextureHeight;

        std::vector<UINT8> data(textureSize);
        UINT8* pData = &data[0];

        for (UINT n = 0; n < textureSize; n += TexturePixelSize)
        {
            UINT x = n % rowPitch;
            UINT y = n / rowPitch;
            UINT i = x / cellPitch;
            UINT j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pData[n] = 0x00;        // R
                pData[n + 1] = 0x00;    // G
                pData[n + 2] = 0x00;    // B
                pData[n + 3] = 0xff;    // A
            }
            else
            {
                pData[n] = 0xff;        // R
                pData[n + 1] = 0xff;    // G
                pData[n + 2] = 0xff;    // B
                pData[n + 3] = 0xff;    // A
            }
        }

        return data;
    }

}