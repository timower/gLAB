#pragma once

#ifdef DLL_IMPORT
#define IMGUI_API __declspec(dllimport)
#else
#define IMGUI_API __declspec(dllexport)
#endif

#include "imgui/imgui.h"
#include "glfw/include/GLFW/glfw3.h"
#include "bgfx.cmake/bgfx/include/bgfx/bgfx.h"
#include "IconFontCppHeaders/IconsFontAwesome4.h"
#include "IconFontCppHeaders/IconsKenney.h"
