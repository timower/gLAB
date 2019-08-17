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

void imguiCreate(GLFWwindow* window, float _fontSize = 18.0f);
void imguiDestroy();

void imguiBeginFrame();
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

inline void NextLine() {
    SetCursorPosY(GetCursorPosY() + GetTextLineHeightWithSpacing());
}

inline bool MouseOverArea() {
    return false || ImGui::IsAnyItemHovered() ||
           ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

}  // namespace ImGui
