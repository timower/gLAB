/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <glad/glad.h>

#include <GLFW/glfw3.h>

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

// If you get an error please report on github. You may try different GL context
// version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc) {
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr,
                "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to "
                "compile %s!\n",
                desc);
    if (log_length > 1) {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

// If you get an error please report on GitHub. You may try different GL context
// version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc) {
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr,
                "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link "
                "%s! (with GLSL '%s')\n",
                desc, "410");
    if (log_length > 1) {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

struct OcornutImguiContext {
    void render(ImDrawData* draw_data) {
        const ImGuiIO& io = ImGui::GetIO();
        const float width = io.DisplaySize.x;
        const float height = io.DisplaySize.y;

        // Avoid rendering when minimized, scale coordinates for retina displays
        // (screen coordinates != framebuffer coordinates)
        int fb_width =
            (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
        int fb_height =
            (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
        if (fb_width <= 0 || fb_height <= 0) return;

        // Backup GL state
        GLenum last_active_texture;
        glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
        glActiveTexture(GL_TEXTURE0);
        GLint last_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
        GLint last_sampler;
        glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
        GLint last_array_buffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
        GLint last_vertex_array_object;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array_object);
#endif
#ifdef GL_POLYGON_MODE
        GLint last_polygon_mode[2];
        glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
        GLint last_viewport[4];
        glGetIntegerv(GL_VIEWPORT, last_viewport);
        GLint last_scissor_box[4];
        glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
        GLenum last_blend_src_rgb;
        glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
        GLenum last_blend_dst_rgb;
        glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
        GLenum last_blend_src_alpha;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
        GLenum last_blend_dst_alpha;
        glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
        GLenum last_blend_equation_rgb;
        glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
        GLenum last_blend_equation_alpha;
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA,
                      (GLint*)&last_blend_equation_alpha);
        GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
        GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
        GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
        GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
        bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
        GLenum last_clip_origin = 0;
        glGetIntegerv(
            GL_CLIP_ORIGIN,
            (GLint*)&last_clip_origin);  // Support for GL 4.5's
                                         // glClipControl(GL_UPPER_LEFT)
        if (last_clip_origin == GL_UPPER_LEFT) clip_origin_lower_left = false;
#endif

        // Setup desired GL state
        // Recreate the VAO every time (this is to easily allow multiple GL
        // contexts to be rendered to. VAO are not shared among GL contexts) The
        // renderer would actually work without any VAO bound, but then our
        // VertexAttrib calls would overwrite the default one currently bound.
        GLuint vertex_array_object = 0;
#ifndef IMGUI_IMPL_OPENGL_ES2
        glGenVertexArrays(1, &vertex_array_object);
#endif
        // Setup render state: alpha-blending enabled, no face culling, no depth
        // testing, scissor enabled, polygon fill
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

        // Setup viewport, orthographic projection matrix
        // Our visible imgui space lies from draw_data->DisplayPos (top left) to
        // draw_data->DisplayPos+data_data->DisplaySize (bottom right).
        // DisplayPos is (0,0) for single viewport apps.
        glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        const float ortho_projection[4][4] = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
        };
        glUseProgram(g_ShaderHandle);
        glUniform1i(g_AttribLocationTex, 0);
        glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE,
                           &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
        glBindSampler(0,
                      0);  // We use combined texture/sampler state.
                           // Applications using GL 3.3 may set that otherwise.
#endif

        (void)vertex_array_object;
#ifndef IMGUI_IMPL_OPENGL_ES2
        glBindVertexArray(vertex_array_object);
#endif

        // Bind vertex/index buffers and setup attributes for ImDrawVert
        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        glEnableVertexAttribArray(g_AttribLocationVtxPos);
        glEnableVertexAttribArray(g_AttribLocationVtxUV);
        glEnableVertexAttribArray(g_AttribLocationVtxColor);
        glVertexAttribPointer(g_AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE,
                              sizeof(ImDrawVert),
                              (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        glVertexAttribPointer(g_AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE,
                              sizeof(ImDrawVert),
                              (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
        glVertexAttribPointer(g_AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE,
                              GL_TRUE, sizeof(ImDrawVert),
                              (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

        // Will project scissor/clipping rectangles into framebuffer space
        ImVec2 clip_off =
            draw_data->DisplayPos;  // (0,0) unless using multi-viewports
        ImVec2 clip_scale =
            draw_data->FramebufferScale;  // (1,1) unless using retina display
                                          // which are often (2,2)

        // Render command lists
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];

            // Upload vertex/index buffers
            glBufferData(
                GL_ARRAY_BUFFER,
                (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL) {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback
                    // value used by the user to request the renderer to reset
                    // render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        /*ImGui_ImplOpenGL3_SetupRenderState(draw_data,
                           fb_width, fb_height,*/
                        printf("TODO");
                    else
                        pcmd->UserCallback(cmd_list, pcmd);
                } else {
                    // Project scissor/clipping rectangles into framebuffer
                    // space
                    ImVec4 clip_rect;
                    clip_rect.x =
                        (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                    clip_rect.y =
                        (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                    clip_rect.z =
                        (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                    clip_rect.w =
                        (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                    if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
                        clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
                        // Apply scissor/clipping rectangle
                        if (clip_origin_lower_left)
                            glScissor((int)clip_rect.x,
                                      (int)(fb_height - clip_rect.w),
                                      (int)(clip_rect.z - clip_rect.x),
                                      (int)(clip_rect.w - clip_rect.y));
                        else
                            glScissor(
                                (int)clip_rect.x, (int)clip_rect.y,
                                (int)clip_rect.z,
                                (int)clip_rect
                                    .w);  // Support for GL 4.5 rarely used
                                          // glClipControl(GL_UPPER_LEFT)

                        // Bind texture, Draw
                        glBindTexture(GL_TEXTURE_2D,
                                      (GLuint)(intptr_t)pcmd->TextureId);
#if IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX
                        glDrawElementsBaseVertex(
                            GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT
                                                   : GL_UNSIGNED_INT,
                            (void*)(intptr_t)(pcmd->IdxOffset *
                                              sizeof(ImDrawIdx)),
                            (GLint)pcmd->VtxOffset);
#else
                        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                                       sizeof(ImDrawIdx) == 2
                                           ? GL_UNSIGNED_SHORT
                                           : GL_UNSIGNED_INT,
                                       (void*)(intptr_t)(pcmd->IdxOffset *
                                                         sizeof(ImDrawIdx)));
#endif
                    }
                }
            }
        }

        // Destroy the temporary VAO
#ifndef IMGUI_IMPL_OPENGL_ES2
        glDeleteVertexArrays(1, &vertex_array_object);
#endif

        // Restore modified GL state
        glUseProgram(last_program);
        glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
        glBindSampler(0, last_sampler);
#endif
        glActiveTexture(last_active_texture);
#ifndef IMGUI_IMPL_OPENGL_ES2
        glBindVertexArray(last_vertex_array_object);
#endif
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBlendEquationSeparate(last_blend_equation_rgb,
                                last_blend_equation_alpha);
        glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb,
                            last_blend_src_alpha, last_blend_dst_alpha);
        if (last_enable_blend)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
        if (last_enable_cull_face)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
        if (last_enable_depth_test)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        if (last_enable_scissor_test)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
        glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
        glViewport(last_viewport[0], last_viewport[1],
                   (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
        glScissor(last_scissor_box[0], last_scissor_box[1],
                  (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
    }

    void create(GLFWwindow* window, float _fontSize) {
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        io.DisplaySize = ImVec2(width, height);
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

        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        const char* glsl_version = "#version 410 core";
        strcpy(g_GlslVersionString, glsl_version);
        strcat(g_GlslVersionString, "\n");

        // Backup GL state
        GLint last_texture, last_array_buffer;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

        // Parse GLSL version string

        const GLchar* vertex_shader_glsl_410_core =
            "layout (location = 0) in vec2 Position;\n"
            "layout (location = 1) in vec2 UV;\n"
            "layout (location = 2) in vec4 Color;\n"
            "uniform mat4 ProjMtx;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        const GLchar* fragment_shader_glsl_410_core =
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "uniform sampler2D Texture;\n"
            "layout (location = 0) out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

        // Select shaders matching our GLSL versions
        const GLchar* vertex_shader = NULL;
        const GLchar* fragment_shader = NULL;

        vertex_shader = vertex_shader_glsl_410_core;
        fragment_shader = fragment_shader_glsl_410_core;

        // Create shaders
        const GLchar* vertex_shader_with_version[2] = {g_GlslVersionString,
                                                       vertex_shader};
        g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
        glCompileShader(g_VertHandle);
        CheckShader(g_VertHandle, "vertex shader");

        const GLchar* fragment_shader_with_version[2] = {g_GlslVersionString,
                                                         fragment_shader};
        g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
        glCompileShader(g_FragHandle);
        CheckShader(g_FragHandle, "fragment shader");

        g_ShaderHandle = glCreateProgram();
        glAttachShader(g_ShaderHandle, g_VertHandle);
        glAttachShader(g_ShaderHandle, g_FragHandle);
        glLinkProgram(g_ShaderHandle);
        CheckProgram(g_ShaderHandle, "shader program");

        g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
        g_AttribLocationProjMtx =
            glGetUniformLocation(g_ShaderHandle, "ProjMtx");
        g_AttribLocationVtxPos =
            glGetAttribLocation(g_ShaderHandle, "Position");
        g_AttribLocationVtxUV = glGetAttribLocation(g_ShaderHandle, "UV");
        g_AttribLocationVtxColor = glGetAttribLocation(g_ShaderHandle, "Color");

        // Create buffers
        glGenBuffers(1, &g_VboHandle);
        glGenBuffers(1, &g_ElementsHandle);

        // Fonts texture:
        // Build texture atlas
        // ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        // int width, height;
        io.Fonts->GetTexDataAsRGBA32(
            &pixels, &width,
            &height);  // Load as RGBA 32-bits (75% of the memory is wasted, but
                       // default font is so small) because it is more likely to
                       // be compatible with user's existing shaders. If your
                       // ImTextureId represent a higher-level concept than just
                       // a GL texture id, consider calling GetTexDataAsAlpha8()
                       // instead to save on GPU memory.

        // Upload texture to graphics system

        glGenTextures(1, &g_FontTexture);
        glBindTexture(GL_TEXTURE_2D, g_FontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, pixels);

        // Store our identifier
        io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

        // Restore modified GL state
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    }

    void destroy() {
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

    void beginFrame() {
        ImGuiIO& io = ImGui::GetIO();

        const double now = glfwGetTime();
        const double frameTime = now - m_last;
        m_last = now;
        io.DeltaTime = float(frameTime);

        ImGui::NewFrame();
    }

    void endFrame() {
        ImGui::Render();
        render(ImGui::GetDrawData());
    }

    ImFont* m_font[ImGui::Font::Count];
    float m_last;

    // OpenGL Data
    char g_GlslVersionString[32] = "";
    GLuint g_FontTexture = 0;
    GLuint g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
    int g_AttribLocationTex = 0,
        g_AttribLocationProjMtx = 0;  // Uniforms location
    int g_AttribLocationVtxPos = 0, g_AttribLocationVtxUV = 0,
        g_AttribLocationVtxColor = 0;  // Vertex attributes location
    unsigned int g_VboHandle = 0, g_ElementsHandle = 0;
};

static OcornutImguiContext s_ctx;

void imguiCreate(GLFWwindow* window, float _fontSize) {
    s_ctx.create(window, _fontSize);
}

void imguiDestroy() {
    s_ctx.destroy();
}

void imguiBeginFrame() {
    s_ctx.beginFrame();
}

void imguiEndFrame() {
    s_ctx.endFrame();
}

static const GLFWcallbacks* nextCallbacks;

static void windowSizeCb(GLFWwindow* window, int w, int h) {
    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(w), float(h));

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    if (w > 0 && h > 0)
        io.DisplayFramebufferScale =
            ImVec2((float)display_w / w, (float)display_h / h);

    glViewport(0, 0, display_w, display_h);

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
