/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "vireo/backend/directx/Tools.h"
module vireo.directx.pipelines;

import vireo.directx.descriptors;
import vireo.directx.swapchains;

namespace vireo {

    DXVertexInputLayout::DXVertexInputLayout(const vector<AttributeDescription>& attributesDescriptions) {
        for (const auto& attributesDescription : attributesDescriptions) {
            inputElementDescs.push_back({
                .SemanticName = attributesDescription.binding.c_str(),
                .SemanticIndex = 0,
                .Format = DXFormat[static_cast<int>(attributesDescription.format)],
                .InputSlot = 0,
                .AlignedByteOffset = attributesDescription.offset,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0
            });
        }
    }

    DXShaderModule::DXShaderModule(const string& fileName) {
        ifstream shaderFile(fileName + ".dxil", ios::binary | ios::ate);
        if (!shaderFile) {
            die("Error loading shader " + fileName);
        }
        streamsize size = shaderFile.tellg();
        shaderFile.seekg(0, ios::beg);
        if (FAILED(D3DCreateBlob(size, &shader))) {
            die("Error creating blob for  shader " + fileName);
        }
        shaderFile.read(static_cast<char*>(shader->GetBufferPointer()), size);
    }

    DXPipelineResources::DXPipelineResources(
        const ComPtr<ID3D12Device>& device,
        const vector<shared_ptr<DescriptorLayout>>& descriptorLayouts,
        const PushConstantsDesc& pushConstants,
        const wstring& name) {

        constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
               D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
               D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
               D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
               D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        vector<CD3DX12_ROOT_PARAMETER1> rootParameters(descriptorLayouts.size());
        for (int i = 0; i < descriptorLayouts.size(); i++) {
            const auto layout = static_pointer_cast<DXDescriptorLayout>(descriptorLayouts[i]);
            for (auto& range : layout->getRanges()) {
                range.RegisterSpace = i;
            }
            rootParameters[i].InitAsDescriptorTable(
                layout->getRanges().size(),
                layout->getRanges().data(),
                D3D12_SHADER_VISIBILITY_ALL);
        }

        if (pushConstants.size > 0) {
            auto pushConstantRootParams = CD3DX12_ROOT_PARAMETER1 {};
            pushConstantRootParams.InitAsConstants(
                pushConstants.size / sizeof(uint32_t),
                0,
                rootParameters.size(),
                D3D12_SHADER_VISIBILITY_ALL
            );
            if (pushConstants.stage == ShaderStage::VERTEX) {
                pushConstantRootParams.ShaderVisibility  = D3D12_SHADER_VISIBILITY_VERTEX;
            } else if (pushConstants.stage == ShaderStage::FRAGMENT) {
                pushConstantRootParams.ShaderVisibility  = D3D12_SHADER_VISIBILITY_PIXEL;
            }
            pushConstantsRootParameterIndex = rootParameters.size();
            rootParameters.push_back(pushConstantRootParams);
        }

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(
            rootParameters.size(),
            rootParameters.empty() ? nullptr : rootParameters.data(),
            0,
            nullptr,
            rootSignatureFlags);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        auto hr = D3DX12SerializeVersionedRootSignature(
            &rootSignatureDesc,
            featureData.HighestVersion,
            &signature,
            &error);
        if (FAILED(hr)){
            die(static_cast<char*>(error->GetBufferPointer()));
        }

        DieIfFailed(device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)));
#ifdef _DEBUG
        rootSignature->SetName((L"DXPipelineResources : " + name).c_str());
#endif
    }

    DXPipeline::DXPipeline(
        const ComPtr<ID3D12Device>& device,
        const shared_ptr<PipelineResources>& pipelineResources,
        const shared_ptr<const VertexInputLayout>& vertexInputLayout,
        const shared_ptr<const ShaderModule>& vertexShader,
        const shared_ptr<const ShaderModule>& fragmentShader,
        const Configuration& configuration,
        const wstring& name):
        Pipeline{pipelineResources} {
        auto dxVertexInputLayout = static_pointer_cast<const DXVertexInputLayout>(vertexInputLayout);
        auto dxPipelineResources = static_pointer_cast<const DXPipelineResources>(pipelineResources);
        auto dxVertexShader = static_pointer_cast<const DXShaderModule>(vertexShader);
        auto dxPixelShader = static_pointer_cast<const DXShaderModule>(fragmentShader);

        auto rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        rasterizerState.FillMode = configuration.polygonMode == PolygonMode::FILL ? D3D12_FILL_MODE_SOLID  : D3D12_FILL_MODE_WIREFRAME;
        rasterizerState.CullMode = dxCullMode[static_cast<int>(configuration.cullMode)];
        rasterizerState.FrontCounterClockwise = configuration.frontFaceCounterClockwise;
        rasterizerState.DepthBias = static_cast<INT>(configuration.depthBiasEnable ? configuration.depthBiasConstantFactor : 0.0f);
        rasterizerState.DepthBiasClamp = configuration.depthBiasEnable ? configuration.depthBiasClamp : 0.0f;
        rasterizerState.SlopeScaledDepthBias = configuration.depthBiasEnable ? configuration.depthBiasSlopeFactor : 0.0f;

        auto depthStencil = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        depthStencil.DepthEnable = configuration.depthTestEnable;
        depthStencil.DepthWriteMask = configuration.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL  : D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencil.DepthFunc = dxCompareOp[static_cast<int>(configuration.depthCompareOp)];

        auto psoDesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{
            .pRootSignature = dxPipelineResources->getRootSignature().Get(),
            .VS = CD3DX12_SHADER_BYTECODE(dxVertexShader->getShader().Get()),
            .PS = CD3DX12_SHADER_BYTECODE(dxPixelShader->getShader().Get()),
            .BlendState = configuration.colorBlendEnable ? blendStateEnable : blendStateDisable,
            .SampleMask = UINT_MAX,
            .RasterizerState = rasterizerState,
            .DepthStencilState = depthStencil,
            .InputLayout = {
                dxVertexInputLayout->getInputElementDescs().data(),
                static_cast<UINT>(dxVertexInputLayout->getInputElementDescs().size())
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = { DXSwapChain::RENDER_FORMAT },
            .SampleDesc = {
                .Count = 1,
            }
        };
        psoDesc.BlendState.AlphaToCoverageEnable = configuration.alphaToCoverageEnable;
        DieIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
#ifdef _DEBUG
        pipelineState->SetName((L"DXPipeline : " + name).c_str());
#endif
    }


}
