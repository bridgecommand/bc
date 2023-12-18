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

#ifdef _WIN32
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
#endif

#define HAND_LEFT_INDEX 0
#define HAND_RIGHT_INDEX 1
#define HAND_COUNT 2

// TODO: Equivalent block for linux etc

class VRInterface {
public:
    VRInterface(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);
    ~VRInterface();

private:
    static bool xr_check(XrInstance instance, XrResult result, const char* format, ...);
    static void print_api_layers();
    irr::scene::ISceneManager* smgr;
    irr::video::IVideoDriver* driver;
};

#endif


