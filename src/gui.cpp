/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <imgui.h>

#include <gLAB/gui.h>
#include <gLAB/utils.h>

#include "gLAB/gui/fs_imgui_image.bin.h"
#include "gLAB/gui/fs_ocornut_imgui.bin.h"
#include "gLAB/gui/vs_imgui_image.bin.h"
#include "gLAB/gui/vs_ocornut_imgui.bin.h"

#include "gLAB/gui/icons_font_awesome.ttf.h"
#include "gLAB/gui/icons_kenney.ttf.h"
#include "gLAB/gui/roboto_regular.ttf.h"
#include "gLAB/gui/robotomono_regular.ttf.h"

static const bgfx::EmbeddedShader s_embeddedShaders[] = {
    BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
    BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
    BGFX_EMBEDDED_SHADER(vs_imgui_image), BGFX_EMBEDDED_SHADER(fs_imgui_image),

    BGFX_EMBEDDED_SHADER_END()};

struct FontRangeMerge {
    const void* data;
    size_t size;
    ImWchar ranges[3];
};

static FontRangeMerge s_fontRangeMerge[] = {
    {s_iconsKenneyTtf, sizeof(s_iconsKenneyTtf), {ICON_MIN_KI, ICON_MAX_KI, 0}},
    {s_iconsFontAwesomeTtf,
     sizeof(s_iconsFontAwesomeTtf),
     {ICON_MIN_FA, ICON_MAX_FA, 0}},
};

struct OcornutImguiContext {
    void render(ImDrawData* _drawData) {
        const ImGuiIO& io = ImGui::GetIO();
        const float width = io.DisplaySize.x;
        const float height = io.DisplaySize.y;

        bgfx::setViewName(m_viewId, "ImGui");
        bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

        const bgfx::Caps* caps = bgfx::getCaps();
        {
            float ortho[16];
            bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, 0.0f, 1000.0f, 0.0f,
                         caps->homogeneousDepth);
            bgfx::setViewTransform(m_viewId, nullptr, ortho);
            bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width),
                              uint16_t(height));
        }

        // Render command lists
        for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii) {
            bgfx::TransientVertexBuffer tvb;
            bgfx::TransientIndexBuffer tib;

            const ImDrawList* drawList = _drawData->CmdLists[ii];
            uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
            uint32_t numIndices = (uint32_t)drawList->IdxBuffer.size();

            if (!checkAvailTransientBuffers(numVertices, m_decl, numIndices)) {
                // not enough space in transient buffer just quit drawing the
                // rest...
                break;
            }

            bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_decl);
            bgfx::allocTransientIndexBuffer(&tib, numIndices);

            ImDrawVert* verts = (ImDrawVert*)tvb.data;
            bx::memCopy(verts, drawList->VtxBuffer.begin(),
                        numVertices * sizeof(ImDrawVert));

            ImDrawIdx* indices = (ImDrawIdx*)tib.data;
            bx::memCopy(indices, drawList->IdxBuffer.begin(),
                        numIndices * sizeof(ImDrawIdx));

            uint32_t offset = 0;
            for (const ImDrawCmd *cmd = drawList->CmdBuffer.begin(),
                                 *cmdEnd = drawList->CmdBuffer.end();
                 cmd != cmdEnd; ++cmd) {
                if (cmd->UserCallback) {
                    cmd->UserCallback(drawList, cmd);
                } else if (0 != cmd->ElemCount) {
                    uint64_t state = 0 | BGFX_STATE_WRITE_RGB |
                                     BGFX_STATE_WRITE_A | BGFX_STATE_MSAA;

                    bgfx::TextureHandle th = m_texture;
                    bgfx::ProgramHandle program = m_program;

                    if (nullptr != cmd->TextureId) {
                        union {
                            ImTextureID ptr;
                            struct {
                                bgfx::TextureHandle handle;
                                uint8_t flags;
                                uint8_t mip;
                            } s;
                        } texture = {cmd->TextureId};
                        state |=
                            0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
                                ? BGFX_STATE_BLEND_FUNC(
                                      BGFX_STATE_BLEND_SRC_ALPHA,
                                      BGFX_STATE_BLEND_INV_SRC_ALPHA)
                                : BGFX_STATE_NONE;
                        th = texture.s.handle;
                        if (0 != texture.s.mip) {
                            const float lodEnabled[4] = {float(texture.s.mip),
                                                         1.0f, 0.0f, 0.0f};
                            bgfx::setUniform(u_imageLodEnabled, lodEnabled);
                            program = m_imageProgram;
                        }
                    } else {
                        state |= BGFX_STATE_BLEND_FUNC(
                            BGFX_STATE_BLEND_SRC_ALPHA,
                            BGFX_STATE_BLEND_INV_SRC_ALPHA);
                    }

                    const uint16_t xx =
                        uint16_t(bx::max(cmd->ClipRect.x, 0.0f));
                    const uint16_t yy =
                        uint16_t(bx::max(cmd->ClipRect.y, 0.0f));
                    bgfx::setScissor(
                        xx, yy,
                        uint16_t(bx::min(cmd->ClipRect.z, 65535.0f) - xx),
                        uint16_t(bx::min(cmd->ClipRect.w, 65535.0f) - yy));

                    bgfx::setState(state);
                    bgfx::setTexture(0, s_tex, th);
                    bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
                    bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
                    bgfx::submit(m_viewId, program);
                }

                offset += cmd->ElemCount;
            }
        }
    }

    void create(float _fontSize) {
        m_viewId = 255;
        m_lastScroll = 0;
        m_last = bx::getHPCounter();

        // ImGui::SetAllocatorFunctions(memAlloc, memFree, nullptr);

        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2(1280.0f, 720.0f);
        io.DeltaTime = 1.0f / 60.0f;
        io.IniFilename = nullptr;

        setupStyle(true);

        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

        bgfx::RendererType::Enum type = bgfx::getRendererType();
        m_program = bgfx::createProgram(
            bgfx::createEmbeddedShader(s_embeddedShaders, type,
                                       "vs_ocornut_imgui"),
            bgfx::createEmbeddedShader(s_embeddedShaders, type,
                                       "fs_ocornut_imgui"),
            true);

        u_imageLodEnabled =
            bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
        m_imageProgram =
            bgfx::createProgram(bgfx::createEmbeddedShader(
                                    s_embeddedShaders, type, "vs_imgui_image"),
                                bgfx::createEmbeddedShader(
                                    s_embeddedShaders, type, "fs_imgui_image"),
                                true);

        m_decl.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

        uint8_t* data;
        int32_t width;
        int32_t height;
        {
            ImFontConfig config;
            config.FontDataOwnedByAtlas = false;
            config.MergeMode = false;
            //			config.MergeGlyphCenterV = true;

            const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();
            m_font[ImGui::Font::Regular] = io.Fonts->AddFontFromMemoryTTF(
                (void*)s_robotoRegularTtf, sizeof(s_robotoRegularTtf),
                _fontSize, &config, ranges);
            m_font[ImGui::Font::Mono] = io.Fonts->AddFontFromMemoryTTF(
                (void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf),
                _fontSize - 3.0f, &config, ranges);

            config.MergeMode = true;
            config.DstFont = m_font[ImGui::Font::Regular];

            for (uint32_t ii = 0; ii < BX_COUNTOF(s_fontRangeMerge); ++ii) {
                const FontRangeMerge& frm = s_fontRangeMerge[ii];

                io.Fonts->AddFontFromMemoryTTF((void*)frm.data, (int)frm.size,
                                               _fontSize - 3.0f, &config,
                                               frm.ranges);
            }
        }

        io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

        m_texture =
            bgfx::createTexture2D((uint16_t)width, (uint16_t)height, false, 1,
                                  bgfx::TextureFormat::BGRA8, 0,
                                  bgfx::copy(data, width * height * 4));

        // ImGui::InitDockContext();
    }

    void destroy() {
        // ImGui::ShutdownDockContext();
        ImGui::DestroyContext();

        bgfx::destroy(s_tex);
        bgfx::destroy(m_texture);

        bgfx::destroy(u_imageLodEnabled);
        bgfx::destroy(m_imageProgram);
        bgfx::destroy(m_program);
    }

    void setupStyle(bool _dark) {
        // Doug Binks' darl color scheme
        // https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
        ImGuiStyle& style = ImGui::GetStyle();
        if (_dark) {
            ImGui::StyleColorsDark(&style);
        } else {
            ImGui::StyleColorsLight(&style);
        }

        style.FrameRounding = 4.0f;
        style.WindowBorderSize = 0.0f;
    }

    void beginFrame(bgfx::ViewId _viewId) {
        m_viewId = _viewId;

        ImGuiIO& io = ImGui::GetIO();

        // TODO: use glfw time to be consistent
        const double now = glfwGetTime();
        const double frameTime = now - m_last;
        m_last = now;
        io.DeltaTime = float(frameTime);

        // io.MouseWheel = (float)(_scroll - m_lastScroll);
        // m_lastScroll = _scroll;

        /*
        uint8_t modifiers = inputGetModifiersState();
        io.KeyShift = 0 != (modifiers & (entry::Modifier::LeftShift |
                                         entry::Modifier::RightShift));
        io.KeyCtrl = 0 != (modifiers & (entry::Modifier::LeftCtrl |
                                        entry::Modifier::RightCtrl));
        io.KeyAlt = 0 != (modifiers & (entry::Modifier::LeftAlt |
                                       entry::Modifier::RightAlt));
        for (int32_t ii = 0; ii < (int32_t)entry::Key::Count; ++ii) {
            io.KeysDown[ii] = inputGetKeyState(entry::Key::Enum(ii));
        }
        */

        ImGui::NewFrame();

        // ImGuizmo::BeginFrame();
    }

    void endFrame() {
        ImGui::Render();
        render(ImGui::GetDrawData());
    }

    bgfx::VertexDecl m_decl;
    bgfx::ProgramHandle m_program;
    bgfx::ProgramHandle m_imageProgram;
    bgfx::TextureHandle m_texture;
    bgfx::UniformHandle s_tex;
    bgfx::UniformHandle u_imageLodEnabled;
    ImFont* m_font[ImGui::Font::Count];
    double m_last;
    int32_t m_lastScroll;
    bgfx::ViewId m_viewId;
};

static OcornutImguiContext s_ctx;

void imguiCreate(float _fontSize) {
    s_ctx.create(_fontSize);
}

void imguiDestroy() {
    s_ctx.destroy();
}

void imguiBeginFrame(bgfx::ViewId _viewId) {
    s_ctx.beginFrame(_viewId);
}

void imguiEndFrame() {
    s_ctx.endFrame();
}

static const GLFWcallbacks* nextCallbacks;

static void windowSizeCb(GLFWwindow* window, int w, int h) {
    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(w), float(h));
    if (nextCallbacks != nullptr && nextCallbacks->windowSizeCb)
        nextCallbacks->windowSizeCb(window, w, h);
}

static void cursorPosCB(GLFWwindow* window, double x, double y) {
    auto& io = ImGui::GetIO();
    io.MousePos.x = float(x);
    io.MousePos.y = float(y);
    if (nextCallbacks != nullptr && nextCallbacks->cursorPosCb)
        nextCallbacks->cursorPosCb(window, x, y);
}

static void charCb(GLFWwindow* window, unsigned int codepoint) {
    auto& io = ImGui::GetIO();
    io.AddInputCharacter(codepoint);
    if (nextCallbacks != nullptr && nextCallbacks->charCb)
        nextCallbacks->charCb(window, codepoint);
}

static void mouseButonCb(GLFWwindow* window, int button, int state, int mods) {
    auto& io = ImGui::GetIO();

    for (auto i = 0u; i < IM_ARRAYSIZE(io.MouseDown); i++) {
        io.MouseDown[i] = state == GLFW_PRESS;
    }

    if (nextCallbacks != nullptr && nextCallbacks->mouseButtonCb)
        nextCallbacks->mouseButtonCb(window, button, state, mods);
}

static void keyCb(GLFWwindow* window, int key, int scancode, int state,
                  int mods) {
    auto& io = ImGui::GetIO();
    if (state == GLFW_PRESS)
        io.KeysDown[key] = true;
    else if (state == GLFW_RELEASE)
        io.KeysDown[key] = false;

    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] ||
                 io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift =
        io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt =
        io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper =
        io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

    if (nextCallbacks != nullptr && nextCallbacks->keyCb)
        nextCallbacks->keyCb(window, key, scancode, state, mods);
}

static void scrollCb(GLFWwindow* window, double x, double y) {
    auto& io = ImGui::GetIO();
    io.MouseWheelH += float(x);
    io.MouseWheel += float(y);

    if (nextCallbacks != nullptr && nextCallbacks->scrollCb)
        nextCallbacks->scrollCb(window, x, y);
}

static void cursorEnterCb(GLFWwindow* window, int active) {
    if (nextCallbacks != nullptr && nextCallbacks->cursorEnterCb)
        nextCallbacks->cursorEnterCb(window, active);
}

static const GLFWcallbacks imguiCallbacks = {
    windowSizeCb, cursorPosCB, charCb,       mouseButonCb,
    keyCb,        scrollCb,    cursorEnterCb};

const GLFWcallbacks& getImguiCallbacks(const GLFWcallbacks* callbacks) {
    nextCallbacks = callbacks;
    return imguiCallbacks;
}

namespace ImGui {
void PushFont(Font::Enum _font) {
    PushFont(s_ctx.m_font[_font]);
}
}  // namespace ImGui

/*
#define STB_RECT_PACK_IMPLEMENTATION qsdf
#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
*/
