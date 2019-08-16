/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#pragma once

#include "../lib/glabexternal.h"

#include "window.h"

#define IMGUI_MBUT_LEFT 0x01
#define IMGUI_MBUT_RIGHT 0x02
#define IMGUI_MBUT_MIDDLE 0x04

inline uint32_t imguiRGBA(uint8_t _r, uint8_t _g, uint8_t _b,
                          uint8_t _a = 255) {
    return 0 | (uint32_t(_r) << 0) | (uint32_t(_g) << 8) |
           (uint32_t(_b) << 16) | (uint32_t(_a) << 24);
}

void imguiCreate(float _fontSize = 18.0f);
void imguiDestroy();

void imguiBeginFrame(bgfx::ViewId _view = 255);
void imguiEndFrame();

const GLFWcallbacks& getImguiCallbacks(const GLFWcallbacks* callbacks);

namespace ImGui {
#define IMGUI_FLAGS_NONE UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

struct Font {
    enum Enum {
        Regular,
        Mono,

        Count
    };
};

void PushFont(Font::Enum _font);

// Helper function for passing bgfx::TextureHandle to ImGui::Image.
inline void Image(bgfx::TextureHandle _handle, uint8_t _flags, uint8_t _mip,
                  const ImVec2& _size, const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                  const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                  const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                  const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f)) {
    union {
        struct {
            bgfx::TextureHandle handle;
            uint8_t flags;
            uint8_t mip;
        } s;
        ImTextureID ptr;
    } texture;
    texture.s.handle = _handle;
    texture.s.flags = _flags;
    texture.s.mip = _mip;
    Image(texture.ptr, _size, _uv0, _uv1, _tintCol, _borderCol);
}

// Helper function for passing bgfx::TextureHandle to ImGui::Image.
inline void Image(bgfx::TextureHandle _handle, const ImVec2& _size,
                  const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                  const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                  const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                  const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f)) {
    Image(_handle, IMGUI_FLAGS_ALPHA_BLEND, 0, _size, _uv0, _uv1, _tintCol,
          _borderCol);
}

// Helper function for passing bgfx::TextureHandle to ImGui::ImageButton.
inline bool ImageButton(
    bgfx::TextureHandle _handle, uint8_t _flags, uint8_t _mip,
    const ImVec2& _size, const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
    const ImVec2& _uv1 = ImVec2(1.0f, 1.0f), int _framePadding = -1,
    const ImVec4& _bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
    const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
    union {
        struct {
            bgfx::TextureHandle handle;
            uint8_t flags;
            uint8_t mip;
        } s;
        ImTextureID ptr;
    } texture;
    texture.s.handle = _handle;
    texture.s.flags = _flags;
    texture.s.mip = _mip;
    return ImageButton(texture.ptr, _size, _uv0, _uv1, _framePadding, _bgCol,
                       _tintCol);
}

// Helper function for passing bgfx::TextureHandle to ImGui::ImageButton.
inline bool ImageButton(bgfx::TextureHandle _handle, const ImVec2& _size,
                        const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                        const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                        int _framePadding = -1,
                        const ImVec4& _bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
                        const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f,
                                                        1.0f)) {
    return ImageButton(_handle, IMGUI_FLAGS_ALPHA_BLEND, 0, _size, _uv0, _uv1,
                       _framePadding, _bgCol, _tintCol);
}

inline void NextLine() {
    SetCursorPosY(GetCursorPosY() + GetTextLineHeightWithSpacing());
}

inline bool MouseOverArea() {
    return false || ImGui::IsAnyItemHovered() ||
           ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

}  // namespace ImGui
