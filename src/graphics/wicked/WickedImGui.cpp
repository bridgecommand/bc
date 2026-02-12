/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     Dear ImGui integration with Wicked Engine.
     Adapted from WE Example_ImGui (MIT license).
     Original: Copyright (c) 2014-2023 Omar Cornut */

#ifdef WITH_WICKED_ENGINE

#include "WickedImGui.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <cstring>
#include <iostream>

using namespace wi::graphics;

namespace bc { namespace graphics { namespace wicked {

// Module-level state
static Shader imguiVS;
static Shader imguiPS;
static Texture fontTexture;
static Sampler imguiSampler;
static InputLayout imguiInputLayout;
static PipelineState imguiPSO;
static bool initialized = false;
static bool deviceObjectsCreated = false;

struct ImGui_Impl_Data {};

static ImGui_Impl_Data* ImGui_Impl_GetBackendData() {
    return ImGui::GetCurrentContext()
        ? (ImGui_Impl_Data*)ImGui::GetIO().BackendRendererUserData
        : nullptr;
}

static bool CreateDeviceObjects() {
    auto* bd = ImGui_Impl_GetBackendData();
    if (!bd) return false;

    ImGuiIO& io = ImGui::GetIO();

    // Build font texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    TextureDesc texDesc;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.mip_levels = 1;
    texDesc.array_size = 1;
    texDesc.format = Format::R8G8B8A8_UNORM;
    texDesc.bind_flags = BindFlag::SHADER_RESOURCE;

    SubresourceData texData;
    texData.data_ptr = pixels;
    texData.row_pitch = width * GetFormatStride(texDesc.format);
    texData.slice_pitch = texData.row_pitch * height;

    GetDevice()->CreateTexture(&texDesc, &texData, &fontTexture);

    SamplerDesc samplerDesc;
    samplerDesc.address_u = TextureAddressMode::WRAP;
    samplerDesc.address_v = TextureAddressMode::WRAP;
    samplerDesc.address_w = TextureAddressMode::WRAP;
    samplerDesc.filter = Filter::MIN_MAG_MIP_LINEAR;
    GetDevice()->CreateSampler(&samplerDesc, &imguiSampler);

    io.Fonts->SetTexID((ImTextureID)&fontTexture);

    // Input layout matching ImDrawVert
    imguiInputLayout.elements = {
        {"POSITION", 0, Format::R32G32_FLOAT, 0,
         (uint32_t)IM_OFFSETOF(ImDrawVert, pos), InputClassification::PER_VERTEX_DATA},
        {"TEXCOORD", 0, Format::R32G32_FLOAT, 0,
         (uint32_t)IM_OFFSETOF(ImDrawVert, uv), InputClassification::PER_VERTEX_DATA},
        {"COLOR", 0, Format::R8G8B8A8_UNORM, 0,
         (uint32_t)IM_OFFSETOF(ImDrawVert, col), InputClassification::PER_VERTEX_DATA},
    };

    // Create graphics pipeline
    PipelineStateDesc desc;
    desc.vs = &imguiVS;
    desc.ps = &imguiPS;
    desc.il = &imguiInputLayout;
    desc.dss = wi::renderer::GetDepthStencilState(wi::enums::DSSTYPE_DEPTHREAD);
    desc.rs = wi::renderer::GetRasterizerState(wi::enums::RSTYPE_DOUBLESIDED);
    desc.bs = wi::renderer::GetBlendState(wi::enums::BSTYPE_TRANSPARENT);
    desc.pt = PrimitiveTopology::TRIANGLELIST;
    GetDevice()->CreatePipelineState(&desc, &imguiPSO);

    deviceObjectsCreated = true;
    std::cout << "WickedImGui: Device objects created (" << width << "x" << height
              << " font atlas)" << std::endl;
    return true;
}

void ImGuiInit(void* platformWindow) {
    if (initialized) return;

    // Load shaders - they need to be compiled .cso files alongside the executable
    {
        auto shaderPath = wi::renderer::GetShaderSourcePath();
        wi::renderer::SetShaderSourcePath(wi::helper::GetCurrentPath() + "/");
        wi::renderer::LoadShader(ShaderStage::VS, imguiVS, "ImGuiVS.cso");
        wi::renderer::LoadShader(ShaderStage::PS, imguiPS, "ImGuiPS.cso");
        wi::renderer::SetShaderSourcePath(shaderPath);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Enable keyboard nav (useful for bridge simulator controls)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Dark style suits maritime bridge displays
    ImGui::StyleColorsDark();

    // Setup renderer backend
    ImGui_Impl_Data* bd = IM_NEW(ImGui_Impl_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "BridgeCommand_Wicked";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    initialized = true;
    std::cout << "WickedImGui: Initialized" << std::endl;
}

void ImGuiShutdown() {
    if (!initialized) return;

    auto* bd = ImGui_Impl_GetBackendData();
    if (bd) {
        ImGui::GetIO().BackendRendererUserData = nullptr;
        ImGui::GetIO().BackendRendererName = nullptr;
        IM_DELETE(bd);
    }

    ImGui::DestroyContext();
    initialized = false;
    deviceObjectsCreated = false;
    std::cout << "WickedImGui: Shutdown" << std::endl;
}

void ImGuiNewFrame() {
    if (!initialized) return;

    // Create device objects on first frame (deferred to ensure device is ready)
    if (!deviceObjectsCreated) {
        CreateDeviceObjects();
    }

    ImGui::NewFrame();
}

void ImGuiRender(CommandList cmd) {
    if (!initialized || !deviceObjectsCreated) return;

    ImGui::Render();
    auto* drawData = ImGui::GetDrawData();

    if (!drawData || drawData->TotalVtxCount == 0) return;

    int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fbWidth <= 0 || fbHeight <= 0) return;

    GraphicsDevice* device = GetDevice();

    // Allocate GPU memory for vertices and indices
    const uint64_t vbSize = sizeof(ImDrawVert) * drawData->TotalVtxCount;
    const uint64_t ibSize = sizeof(ImDrawIdx) * drawData->TotalIdxCount;
    auto vertexAlloc = device->AllocateGPU(vbSize, cmd);
    auto indexAlloc = device->AllocateGPU(ibSize, cmd);

    // Copy vertex/index data to GPU
    ImDrawVert* vtxDst = reinterpret_cast<ImDrawVert*>(vertexAlloc.data);
    ImDrawIdx* idxDst = reinterpret_cast<ImDrawIdx*>(indexAlloc.data);
    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* list = drawData->CmdLists[n];
        memcpy(vtxDst, list->VtxBuffer.Data, list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, list->IdxBuffer.Data, list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += list->VtxBuffer.Size;
        idxDst += list->IdxBuffer.Size;
    }

    // Orthographic projection matrix
    struct { float mvp[4][4]; } constants;
    float L = drawData->DisplayPos.x;
    float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    float T = drawData->DisplayPos.y;
    float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
    float mvp[4][4] = {
        {2.0f / (R - L),       0.0f,              0.0f, 0.0f},
        {0.0f,                 2.0f / (T - B),    0.0f, 0.0f},
        {0.0f,                 0.0f,              0.5f, 0.0f},
        {(R + L) / (L - R),   (T + B) / (B - T), 0.5f, 1.0f},
    };
    memcpy(&constants.mvp, mvp, sizeof(mvp));
    device->BindDynamicConstantBuffer(constants, 0, cmd);

    // Bind vertex/index buffers
    const GPUBuffer* vbs[] = {&vertexAlloc.buffer};
    const uint32_t strides[] = {sizeof(ImDrawVert)};
    const uint64_t offsets[] = {vertexAlloc.offset};
    device->BindVertexBuffers(vbs, 0, 1, strides, offsets, cmd);
    device->BindIndexBuffer(&indexAlloc.buffer, IndexBufferFormat::UINT16, indexAlloc.offset, cmd);

    // Set viewport
    Viewport viewport;
    viewport.width = (float)fbWidth;
    viewport.height = (float)fbHeight;
    device->BindViewports(1, &viewport, cmd);

    // Bind pipeline and sampler
    device->BindPipelineState(&imguiPSO, cmd);
    device->BindSampler(&imguiSampler, 0, cmd);

    // Render draw lists
    ImVec2 clipOff = drawData->DisplayPos;
    ImVec2 clipScale = drawData->FramebufferScale;
    int32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* drawList = drawData->CmdLists[n];
        for (int ci = 0; ci < drawList->CmdBuffer.Size; ci++) {
            const ImDrawCmd* pcmd = &drawList->CmdBuffer[ci];

            if (pcmd->UserCallback) {
                if (pcmd->UserCallback != ImDrawCallback_ResetRenderState)
                    pcmd->UserCallback(drawList, pcmd);
            } else {
                ImVec2 clipMin(pcmd->ClipRect.x - clipOff.x, pcmd->ClipRect.y - clipOff.y);
                ImVec2 clipMax(pcmd->ClipRect.z - clipOff.x, pcmd->ClipRect.w - clipOff.y);
                if (clipMax.x < clipMin.x || clipMax.y < clipMin.y) continue;

                Rect scissor;
                scissor.left = (int32_t)clipMin.x;
                scissor.top = (int32_t)clipMin.y;
                scissor.right = (int32_t)clipMax.x;
                scissor.bottom = (int32_t)clipMax.y;
                device->BindScissorRects(1, &scissor, cmd);

                const Texture* texture = (const Texture*)pcmd->TextureId;
                device->BindResource(texture, 0, cmd);
                device->DrawIndexed(pcmd->ElemCount, indexOffset, vertexOffset, cmd);
            }
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += drawList->VtxBuffer.Size;
    }
}

bool ImGuiWantsKeyboard() {
    if (!initialized) return false;
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiWantsMouse() {
    if (!initialized) return false;
    return ImGui::GetIO().WantCaptureMouse;
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
