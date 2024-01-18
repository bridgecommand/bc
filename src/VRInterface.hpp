/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2023 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef __VRINTERFACE_HPP_INCLUDED__
#define __VRINTERFACE_HPP_INCLUDED__

#include "irrlicht.h"
#include "SimulationModel.hpp"
#include <cstdint> // For int64_t

#if defined _WIN64
#include <Unknwn.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/GL.h>
#include "libs/Irrlicht/irrlicht-svn/source/Irrlicht/glext.h"
#define XR_USE_GRAPHICS_API_OPENGL
#define XR_USE_PLATFORM_WIN32
#include "libs/OpenXR/OpenXR-SDK-main/include/openxr/openxr.h"
#include "libs/OpenXR/OpenXR-SDK-main/include/openxr/openxr_platform.h"
#include "libs/OpenXR/OpenXR-SDK-main/include/openxr/openxr_reflection.h"
#elif defined __linux__
#include <GL/gl.h>
#include "libs/Irrlicht/irrlicht-svn/source/Irrlicht/glext.h"
#include <X11/Xlib.h>
#include <GL/glx.h>
#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include "libs/OpenXR/OpenXR-SDK-main/include/openxr/openxr.h"
#include "libs/OpenXR/OpenXR-SDK-main/include/openxr/openxr_platform.h"
#else
// Not windows 64 bit or linux, just include required headers for interface, functionality will not be used
#ifdef _WIN32
#include <GL/GL.h>
#else
#include <OpenGL/gl.h>
#endif
#define XR_USE_GRAPHICS_API_OPENGL
#include "libs/OpenXR/OpenXR-SDK-main/include/openxr/openxr.h"
#include "libs/OpenXR/OpenXR-SDK-main/include/openxr/openxr_platform.h"
#endif

#define HAND_LEFT_INDEX 0
#define HAND_RIGHT_INDEX 1
#define HAND_COUNT 2

class VRInterface {
public:
    VRInterface(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);
    ~VRInterface();
    int load();
    void unload();
    float getAspectRatio();
    int runtimeEvents();
    int render(SimulationModel* model);

private:
    static bool xr_check(XrInstance instance, XrResult result, const char* format, ...);
    static void print_api_layers();
    static void print_instance_properties(XrInstance instance);
    static void print_system_properties(XrSystemProperties* system_properties);
    static void print_viewconfig_view_info(uint32_t view_count, XrViewConfigurationView* viewconfig_views);
    static int64_t get_swapchain_format(XrInstance instance,
        XrSession session,
        int64_t preferred_format,
        bool fallback);
#ifdef __linux__
    void getContextInformation(Display** xDisplay,
                uint32_t* visualid,
                GLXFBConfig* glxFBConfig,
                GLXDrawable* glxDrawable,
                GLXContext* glxContext);
#endif
    irr::scene::ISceneManager* smgr;
    irr::video::IVideoDriver* driver;
    XrPosef identity_pose;
    XrSessionState state;
    XrSession session;
    XrInstance instance;
    XrViewConfigurationType view_type;
    XrSpace play_space;
    uint32_t view_count;
    XrViewConfigurationView* viewconfig_views;
    XrView* views;
    XrSwapchain* swapchains;
    uint32_t* swapchain_lengths;
    XrSwapchainImageOpenGLKHR** images;
    XrCompositionLayerProjectionView* projection_views;
    GLuint** framebuffers;
    GLuint** depthbuffers;
    XrResult result;

    int swapchainImageWidth;
    int swapchainImageHeight;

    #if defined _WIN64 || defined __linux__
    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
    PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
    PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
    PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
    PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
    #endif

    bool quit_mainloop;
    bool session_running; // to avoid beginning an already running session
    bool run_framecycle;  // for some session states skip the frame cycle
};

#endif

