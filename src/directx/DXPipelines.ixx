/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "vireo/backend/directx/Libraries.h"
export module vireo.directx.pipelines;

import vireo;

export namespace vireo {

    class DXVertexInputLayout : public VertexInputLayout {
    public:
        static constexpr DXGI_FORMAT DXFormat[] {
            DXGI_FORMAT_R32G32_FLOAT,
            DXGI_FORMAT_R32G32B32_FLOAT,
            DXGI_FORMAT_R32G32B32A32_FLOAT
        };

        DXVertexInputLayout(const vector<VertexAttributeDesc>& attributesDescriptions);

        const auto& getInputElementDescs() const { return inputElementDescs; }

    private:
        vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
    };

    class DXShaderModule: public ShaderModule {
    public:
        DXShaderModule(const string& fileName);

        auto getShader() const { return shader; }

    private:
        ComPtr<ID3DBlob> shader;
    };

    class DXPipelineResources : public PipelineResources {
    public:
        // static constexpr auto PUSH_CONSTANTS_SHADER_REGISTER{20};
        DXPipelineResources(
            const ComPtr<ID3D12Device>& device,
            const vector<shared_ptr<DescriptorLayout>>& descriptorLayouts,
            const PushConstantsDesc& pushConstant,
            const wstring& name);

        auto getRootSignature() const { return rootSignature; }

        auto getPushConstantsRootParameterIndex() const { return pushConstantsRootParameterIndex; }

    private:
        ComPtr<ID3D12RootSignature> rootSignature;
        UINT pushConstantsRootParameterIndex{0};
    };

    class DXComputePipeline : public ComputePipeline {
    public:
        DXComputePipeline(
            const ComPtr<ID3D12Device>& device,
            const shared_ptr<PipelineResources>& pipelineResources,
            const shared_ptr<const ShaderModule>& shader,
            const wstring& name);

        auto getPipelineState() const { return pipelineState; }

    private:
        ComPtr<ID3D12PipelineState> pipelineState;
    };

    class DXGraphicPipeline : public GraphicPipeline {
    public:
        static constexpr D3D12_CULL_MODE dxCullMode[] {
            D3D12_CULL_MODE_NONE,
            D3D12_CULL_MODE_FRONT,
            D3D12_CULL_MODE_BACK
        };
        static constexpr D3D12_COMPARISON_FUNC dxCompareOp[] {
            D3D12_COMPARISON_FUNC_NEVER,
            D3D12_COMPARISON_FUNC_LESS,
            D3D12_COMPARISON_FUNC_EQUAL,
            D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_COMPARISON_FUNC_GREATER,
            D3D12_COMPARISON_FUNC_NOT_EQUAL,
            D3D12_COMPARISON_FUNC_GREATER_EQUAL,
            D3D12_COMPARISON_FUNC_ALWAYS,
        };
        static constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE  dxPrimitivesTypes[] {
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT ,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE ,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE ,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE ,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE ,
        };
        static constexpr D3D_PRIMITIVE_TOPOLOGY dxPrimitives[] {
            D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
            D3D_PRIMITIVE_TOPOLOGY_LINELIST,
            D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
        };

        DXGraphicPipeline(
            const ComPtr<ID3D12Device>& device,
            const shared_ptr<PipelineResources>& pipelineResources,
            const shared_ptr<const VertexInputLayout>& vertexInputLayout,
            const shared_ptr<const ShaderModule>& vertexShader,
            const shared_ptr<const ShaderModule>& fragmentShader,
            const Configuration& configuration,
            const wstring& name);

        auto getPipelineState() const { return pipelineState; }

        auto getPrimitiveTopology() const { return primitiveTopology; }

    private:
        ComPtr<ID3D12PipelineState> pipelineState;
        const D3D_PRIMITIVE_TOPOLOGY primitiveTopology;

        static constexpr auto blendStateEnable = D3D12_BLEND_DESC{
            .AlphaToCoverageEnable = FALSE,
            .IndependentBlendEnable = FALSE,
            .RenderTarget = {
                D3D12_RENDER_TARGET_BLEND_DESC {
                    .BlendEnable = TRUE,
                    .LogicOpEnable = FALSE,
                    .SrcBlend = D3D12_BLEND_SRC_ALPHA,
                    .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
                    .BlendOp = D3D12_BLEND_OP_ADD,
                    .SrcBlendAlpha = D3D12_BLEND_ONE,
                    .DestBlendAlpha = D3D12_BLEND_ZERO,
                    .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                    .LogicOp = D3D12_LOGIC_OP_NOOP,
                    .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
                }
            }
        };

        static constexpr auto blendStateDisable = D3D12_BLEND_DESC{
            .AlphaToCoverageEnable = FALSE,
            .IndependentBlendEnable = FALSE,
            .RenderTarget = {
                D3D12_RENDER_TARGET_BLEND_DESC {
                    .BlendEnable = FALSE,
                    .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
                }
            }
        };
    };

}