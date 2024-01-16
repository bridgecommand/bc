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

// This file is mostly derived from https://gitlab.freedesktop.org/monado/demos/openxr-simple-example/ at 94f1a764dd736b23657ff01464ec1518771e8cdc

#define _CRT_SECURE_NO_WARNINGS //FIXME: Temporary fix

#include "VRInterface.hpp"
#include <iostream>
#include <cstdarg>

// Constructor
VRInterface::VRInterface(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver) {
    this->smgr = smgr;
    this->driver = driver;
	identity_pose.orientation.x = 0;
	identity_pose.orientation.y = 0;
	identity_pose.orientation.z = 0;
	identity_pose.orientation.w = 1.0;
	identity_pose.position.x = 0;
	identity_pose.position.y = 0;
	identity_pose.position.z = 0;
}

// Destructor
VRInterface::~VRInterface() {
}

int VRInterface::load() {
#if defined _WIN64 || defined __linux__
	
	// Load required OpenGL extensions
	#if defined _WIN32
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
	if (glGenFramebuffers == 0) {
		std::cout << "glGenFramebuffers not available" << std::endl;
		return 1;
	}
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
	if (glGenRenderbuffers == 0) {
		std::cout << "glGenRenderbuffers not available" << std::endl;
		return 1;
	}
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
	if (glBindFramebuffer == 0) {
		std::cout << "glBindFramebuffer not available" << std::endl;
		return 1;
	}
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
	if (glBindRenderbuffer == 0) {
		std::cout << "glBindRenderbuffer not available" << std::endl;
		return 1;
	}
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
	if (glFramebufferTexture2D == 0) {
		std::cout << "glFramebufferTexture2D not available" << std::endl;
		return 1;
	}
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
	if (glCheckFramebufferStatus == 0) {
		std::cout << "glCheckFramebufferStatus not available" << std::endl;
		return 1;
	}
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
	if (glRenderbufferStorage == 0) {
		std::cout << "glRenderbufferStorage not available" << std::endl;
		return 1;
	}
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
	if (glFramebufferRenderbuffer == 0) {
		std::cout << "glFramebufferRenderbuffer not available" << std::endl;
		return 1;
	}
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
	if (glDeleteFramebuffers == 0) {
		std::cout << "glDeleteFramebuffers not available" << std::endl;
		return 1;
	}
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
	if (glDeleteRenderbuffers == 0) {
		std::cout << "glDeleteRenderbuffers not available" << std::endl;
		return 1;
	}
	#elif defined __linux__
	// glXGetProcAddress never returns Null, so no point in checking
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenFramebuffers");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenRenderbuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindFramebuffer");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindRenderbuffer");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glXGetProcAddress((const GLubyte *)"glFramebufferTexture2D");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glXGetProcAddress((const GLubyte *)"glCheckFramebufferStatus");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)glXGetProcAddress((const GLubyte *)"glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glFramebufferRenderbuffer");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteFramebuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteRenderbuffers");
	#endif

	// Changing to HANDHELD_DISPLAY or a future form factor may work, but has not been tested.
	XrFormFactor form_factor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	// Changing the form_factor may require changing the view_type too.
	view_type = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	// Typically STAGE for room scale/standing, LOCAL for seated
	XrReferenceSpaceType play_space_type = XR_REFERENCE_SPACE_TYPE_LOCAL;
	play_space = XR_NULL_HANDLE;

	// the instance handle can be thought of as the basic connection to the OpenXR runtime
	instance = XR_NULL_HANDLE;
	// the system represents an (opaque) set of XR devices in use, managed by the runtime
	XrSystemId system_id = XR_NULL_SYSTEM_ID;
	// the session deals with the renderloop submitting frames to the runtime
	session = XR_NULL_HANDLE;

	// each graphics API requires the use of a specialized struct
#ifdef _WIN32
	//XrGraphicsBindingOpenGLWin32KHR graphics_binding_gl = { 0 };
	XrGraphicsBindingOpenGLWin32KHR graphics_binding_gl;
	graphics_binding_gl.type = XR_TYPE_UNKNOWN;
	graphics_binding_gl.next = NULL;
	graphics_binding_gl.hDC = NULL;
	graphics_binding_gl.hGLRC = NULL;
#else
	// The runtime interacts with the OpenGL images (textures) via a Swapchain.
	XrGraphicsBindingOpenGLXlibKHR graphics_binding_gl;
	graphics_binding_gl.type = XR_TYPE_UNKNOWN;
	graphics_binding_gl.next = NULL;
	graphics_binding_gl.xDisplay = NULL;
	graphics_binding_gl.visualid = 0;
	graphics_binding_gl.glxFBConfig = 0;
	graphics_binding_gl.glxDrawable = 0;
	graphics_binding_gl.glxContext = 0;
#endif

	// each physical Display/Eye is described by a view.
	// view_count usually depends on the form_factor / view_type.
	// dynamically allocating all view related structs instead of assuming 2
	// hopefully allows this app to scale easily to different view_counts.
	view_count = 0;
	// the viewconfiguration views contain information like resolution about each view
	viewconfig_views = NULL;

	// array of view_count containers for submitting swapchains with rendered VR frames
	projection_views = NULL;
	// array of view_count views, filled by the runtime with current HMD display pose
	views = NULL;

	// array of view_count handles for swapchains.
	// it is possible to use imageRect to render all views to different areas of the
	// same texture, but in this example we use one swapchain per view
	swapchains = NULL;
	// array of view_count ints, storing the length of swapchains
	swapchain_lengths = NULL;
	// array of view_count array of swapchain_length containers holding an OpenGL texture
	// that is allocated by the runtime
	images = NULL;

	// depth swapchain equivalent to the VR color swapchains
	XrSwapchain* depth_swapchains = NULL;
	uint32_t* depth_swapchain_lengths = NULL;
	XrSwapchainImageOpenGLKHR** depth_images = NULL;

	XrPath hand_paths[HAND_COUNT];

	struct
	{
		// supporting depth layers is *optional* for runtimes
		bool supported;
		XrCompositionLayerDepthInfoKHR* infos;
	} depth;

	struct
	{
		// To render into a texture we need a framebuffer (one per texture to make it easy)
		GLuint** framebuffers;

		float near_z;
		float far_z;

		GLuint shader_program_id;
		GLuint VAO;
	} gl_rendering;
	gl_rendering.near_z = 0.01f;
	gl_rendering.far_z = 100.0f;


	// reuse this variable for all our OpenXR return codes
	result = XR_SUCCESS;

	print_api_layers();

	// xrEnumerate*() functions are usually called once with CapacityInput = 0.
	// The function will write the required amount into CountOutput. We then have
	// to allocate an array to hold CountOutput elements and call the function
	// with CountOutput as CapacityInput.
	uint32_t ext_count = 0;
	result = xrEnumerateInstanceExtensionProperties(NULL, 0, &ext_count, NULL);

	/* TODO: instance null will not be able to convert XrResult to string */
	if (!xr_check(NULL, result, "Failed to enumerate number of extension properties")) {
		return 1;
	}

	//XrExtensionProperties* ext_props = malloc(sizeof(XrExtensionProperties) * ext_count);
	XrExtensionProperties* ext_props = new XrExtensionProperties[ext_count];

	for (uint16_t i = 0; i < ext_count; i++) {
		// we usually have to fill in the type (for validation) and set
		// next to NULL (or a pointer to an extension specific struct)
		ext_props[i].type = XR_TYPE_EXTENSION_PROPERTIES;
		ext_props[i].next = NULL;
	}

	result = xrEnumerateInstanceExtensionProperties(NULL, ext_count, &ext_count, ext_props);
	if (!xr_check(NULL, result, "Failed to enumerate extension properties")) {
		return 1;
	}

	bool opengl_supported = false;

	printf("Runtime supports %d extensions\n", ext_count);
	for (uint32_t i = 0; i < ext_count; i++) {
		printf("\t%s v%d\n", ext_props[i].extensionName, ext_props[i].extensionVersion);
		if (strcmp(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME, ext_props[i].extensionName) == 0) {
			opengl_supported = true;
		}

		if (strcmp(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, ext_props[i].extensionName) == 0) {
			depth.supported = true;
		}
	}
	//free(ext_props);
	delete[] ext_props;

	// A graphics extension like OpenGL is required to draw anything in VR
	if (!opengl_supported) {
		printf("Runtime does not support OpenGL extension!\n");
		return 1;
	}

	// --- Create XrInstance
	int enabled_ext_count = 1;
	const char* enabled_exts[1] = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
	// same can be done for API layers, but API layers can also be enabled by env var

	XrInstanceCreateInfo instance_create_info;
	instance_create_info.type = XR_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.next = NULL;
	instance_create_info.createFlags = 0;
	instance_create_info.enabledExtensionCount = enabled_ext_count;
	instance_create_info.enabledExtensionNames = enabled_exts;
	instance_create_info.enabledApiLayerCount = 0;
	instance_create_info.enabledApiLayerNames = NULL;
	instance_create_info.applicationInfo.applicationVersion = 1;
	instance_create_info.applicationInfo.engineVersion = 0;
	instance_create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	strncpy(instance_create_info.applicationInfo.applicationName, "Bridge Command",
		XR_MAX_APPLICATION_NAME_SIZE);
	strncpy(instance_create_info.applicationInfo.engineName, "Irrlicht Custom", XR_MAX_ENGINE_NAME_SIZE);

	result = xrCreateInstance(&instance_create_info, &instance);
	if (!xr_check(NULL, result, "Failed to create XR instance."))
		return 1;

	static PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = NULL;
	result = xrGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR",
			(PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR);
	if (!xr_check(instance, result, "Failed to get OpenGL graphics requirements function!"))
		return 1;

	// Optionally get runtime name and version
	print_instance_properties(instance);

	// --- Get XrSystemId
	XrSystemGetInfo system_get_info; 
	system_get_info.type = XR_TYPE_SYSTEM_GET_INFO;
	system_get_info.formFactor = form_factor;
	system_get_info.next = NULL;

	result = xrGetSystem(instance, &system_get_info, &system_id);
	if (!xr_check(instance, result, "Failed to get system for HMD form factor."))
		return 1;

	printf("Successfully got XrSystem with id %lu for HMD form factor\n", system_id);

	XrSystemProperties system_props; 
	system_props.type = XR_TYPE_SYSTEM_PROPERTIES;
	system_props.next = NULL;

	result = xrGetSystemProperties(instance, system_id, &system_props);
	if (!xr_check(instance, result, "Failed to get System properties"))
		return 1;

	print_system_properties(&system_props);

	result = xrEnumerateViewConfigurationViews(instance, system_id, view_type, 0, &view_count, NULL);
	if (!xr_check(instance, result, "Failed to get view configuration view count!"))
		return 1;

	//viewconfig_views = malloc(sizeof(XrViewConfigurationView) * view_count);
	viewconfig_views = new XrViewConfigurationView[view_count];

	for (uint32_t i = 0; i < view_count; i++) {
		viewconfig_views[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
		viewconfig_views[i].next = NULL;
	}

	result = xrEnumerateViewConfigurationViews(instance, system_id, view_type, view_count,
		&view_count, viewconfig_views);
	if (!xr_check(instance, result, "Failed to enumerate view configuration views!"))
		return 1;
	print_viewconfig_view_info(view_count, viewconfig_views);

	// OpenXR requires checking graphics requirements before creating a session.
	XrGraphicsRequirementsOpenGLKHR opengl_reqs;
	opengl_reqs.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
	opengl_reqs.next = NULL;

	// this function pointer was loaded with xrGetInstanceProcAddr
	result = pfnGetOpenGLGraphicsRequirementsKHR(instance, system_id, &opengl_reqs);
	if (!xr_check(instance, result, "Failed to get OpenGL graphics requirements!"))
		return 1;

	/* Checking opengl_reqs.minApiVersionSupported and opengl_reqs.maxApiVersionSupported
	 * is not very useful, compatibility will depend on the OpenGL implementation and the
	 * OpenXR runtime much more than the OpenGL version.
	 * Other APIs have more useful verifiable requirements. */

	 // --- Create session
#ifdef _WIN32
	graphics_binding_gl.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;
	graphics_binding_gl.hDC = (HDC)(driver->getExposedVideoData().OpenGLWin32.HDc);
	graphics_binding_gl.hGLRC = (HGLRC)(driver->getExposedVideoData().OpenGLWin32.HRc);
#else
	graphics_binding_gl.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
	#ifdef __linux__
	// Get equivalents for Linux, instead uses xDisplay, visualid, glxFBConfig, glxDrawable, glxContext
	// TODO: Note that visualid and glxFBConfig are not set, but this does not seem to matter?
	getContextInformation(&graphics_binding_gl.xDisplay, &graphics_binding_gl.visualid,
	                     &graphics_binding_gl.glxFBConfig, &graphics_binding_gl.glxDrawable,
	                     &graphics_binding_gl.glxContext);
	#endif
#endif

	//printf("Using OpenGL version: %s\n", glGetString(GL_VERSION));
	//printf("Using OpenGL Renderer: %s\n", glGetString(GL_RENDERER));

	XrSessionCreateInfo session_create_info;
	session_create_info.type = XR_TYPE_SESSION_CREATE_INFO;
	session_create_info.next = &graphics_binding_gl;
	session_create_info.systemId = system_id;
	session_create_info.createFlags = 0;

	result = xrCreateSession(instance, &session_create_info, &session);
	if (!xr_check(instance, result, "Failed to create session"))
		return 1;

	printf("Successfully created a session with OpenGL!\n");

	/* Many runtimes support at least STAGE and LOCAL but not all do.
	 * Sophisticated apps might check with xrEnumerateReferenceSpaces() if the
	 * chosen one is supported and try another one if not.
	 * Here we will get an error from xrCreateReferenceSpace() and exit. */
	XrReferenceSpaceCreateInfo play_space_create_info; 
	play_space_create_info.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO; 
	play_space_create_info.next = NULL; 
	play_space_create_info.referenceSpaceType = play_space_type;
	play_space_create_info.poseInReferenceSpace = identity_pose;

	result = xrCreateReferenceSpace(session, &play_space_create_info, &play_space);
	if (!xr_check(instance, result, "Failed to create play space!"))
		return 1;

	// --- Create Swapchains
	uint32_t swapchain_format_count;
	result = xrEnumerateSwapchainFormats(session, 0, &swapchain_format_count, NULL);
	if (!xr_check(instance, result, "Failed to get number of supported swapchain formats"))
		return 1;

	printf("Runtime supports %d swapchain formats\n", swapchain_format_count);
	int64_t* swapchain_formats = new int64_t[swapchain_format_count];
	result = xrEnumerateSwapchainFormats(session, swapchain_format_count, &swapchain_format_count,
		swapchain_formats);
	if (!xr_check(instance, result, "Failed to enumerate swapchain formats"))
		return 1;

	// SRGB is usually a better choice than linear
	// a more sophisticated approach would iterate supported swapchain formats and choose from them
	int64_t color_format = get_swapchain_format(instance, session, GL_SRGB8_ALPHA8_EXT, true);

	// --- Create swapchain for main VR rendering

	// In the frame loop we render into OpenGL textures we receive from the runtime here.
	//swapchains = malloc(sizeof(XrSwapchain) * view_count);
	swapchains = new XrSwapchain[view_count];
	//swapchain_lengths = malloc(sizeof(uint32_t) * view_count);
	swapchain_lengths = new uint32_t[view_count];
	//images = malloc(sizeof(XrSwapchainImageOpenGLKHR*) * view_count);
	images = new XrSwapchainImageOpenGLKHR*[view_count];
	for (uint32_t i = 0; i < view_count; i++) {
		XrSwapchainCreateInfo swapchain_create_info;
		swapchain_create_info.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
		swapchain_create_info.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.createFlags = 0;
		swapchain_create_info.format = color_format;
		swapchain_create_info.sampleCount = viewconfig_views[i].recommendedSwapchainSampleCount;
		swapchain_create_info.width = viewconfig_views[i].recommendedImageRectWidth;
		swapchain_create_info.height = viewconfig_views[i].recommendedImageRectHeight;
		swapchain_create_info.faceCount = 1;
		swapchain_create_info.arraySize = 1;
		swapchain_create_info.mipCount = 1;
		swapchain_create_info.next = NULL;

		result = xrCreateSwapchain(session, &swapchain_create_info, &swapchains[i]);
		if (!xr_check(instance, result, "Failed to create swapchain %d!", i))
			return 1;

		// The runtime controls how many textures we have to be able to render to
		// (e.g. "triple buffering")
		result = xrEnumerateSwapchainImages(swapchains[i], 0, &swapchain_lengths[i], NULL);
		if (!xr_check(instance, result, "Failed to enumerate swapchains"))
			return 1;

		//images[i] = malloc(sizeof(XrSwapchainImageOpenGLKHR) * swapchain_lengths[i]);
		images[i] = new XrSwapchainImageOpenGLKHR[swapchain_lengths[i]];
		for (uint32_t j = 0; j < swapchain_lengths[i]; j++) {
			images[i][j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
			images[i][j].next = NULL;
		}
		result =
			xrEnumerateSwapchainImages(swapchains[i], swapchain_lengths[i], &swapchain_lengths[i],
				(XrSwapchainImageBaseHeader*)images[i]);
		if (!xr_check(instance, result, "Failed to enumerate swapchain images"))
			return 1;

		// Store image width and height, assumed to be same for both views
		swapchainImageWidth = swapchain_create_info.width;
		swapchainImageHeight = swapchain_create_info.height;

	}

	// Do not allocate these every frame to save some resources
	//views = (XrView*)malloc(sizeof(XrView) * view_count);
	views = new XrView[view_count];
	for (uint32_t i = 0; i < view_count; i++) {
		views[i].type = XR_TYPE_VIEW;
		views[i].next = NULL;
	}

	//projection_views = (XrCompositionLayerProjectionView*)malloc(
	//	sizeof(XrCompositionLayerProjectionView) * view_count);
	projection_views = new XrCompositionLayerProjectionView[view_count];
	for (uint32_t i = 0; i < view_count; i++) {
		projection_views[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		projection_views[i].next = NULL;

		projection_views[i].subImage.swapchain = swapchains[i];
		projection_views[i].subImage.imageArrayIndex = 0;
		projection_views[i].subImage.imageRect.offset.x = 0;
		projection_views[i].subImage.imageRect.offset.y = 0;
		projection_views[i].subImage.imageRect.extent.width =
			viewconfig_views[i].recommendedImageRectWidth;
		projection_views[i].subImage.imageRect.extent.height =
			viewconfig_views[i].recommendedImageRectHeight;

		// projection_views[i].{pose, fov} have to be filled every frame in frame loop
	};

	// Create framebuffers
	framebuffers = new GLuint* [view_count];
	depthbuffers = new GLuint * [view_count];
	for (uint32_t i = 0; i < view_count; i++) {
		framebuffers[i] = new GLuint[swapchain_lengths[i]];
		depthbuffers[i] = new GLuint[swapchain_lengths[i]];
		glGenFramebuffers(swapchain_lengths[i], framebuffers[i]);
		glGenRenderbuffers(swapchain_lengths[i], depthbuffers[i]);
	}

	state = XR_SESSION_STATE_UNKNOWN;
	quit_mainloop = false;
	session_running = false; // to avoid beginning an already running session
	run_framecycle = false;  // for some session states skip the frame cycle

	// swapchain_formats was allocated locally with new, so delete here
	delete[] swapchain_formats;

	// If successfull, return 0
	return 0;
#else
	std::cout << "VR interface not implemented" << std::endl;
	return 1;
#endif
}

#ifdef __linux__
void VRInterface::getContextInformation(Display** xDisplay,
                uint32_t* visualid,
                GLXFBConfig* glxFBConfig,
                GLXDrawable* glxDrawable,
                GLXContext* glxContext)
{
    *xDisplay = XOpenDisplay(NULL);
    *glxContext = glXGetCurrentContext();
    *glxDrawable = glXGetCurrentDrawable();
}
#endif

float VRInterface::getAspectRatio() {
	if (swapchainImageHeight > 0) {
		return (float)swapchainImageWidth / (float)swapchainImageHeight;
	}
	else {
		return 1.0;
	}
}

void VRInterface::unload() {
#if defined _WIN64 || defined __linux__
	for (uint32_t i = 0; i < view_count; i++) {
		delete[] images[i];

		glDeleteFramebuffers(swapchain_lengths[i], framebuffers[i]);
		delete[] framebuffers[i];

		glDeleteRenderbuffers(swapchain_lengths[i], depthbuffers[i]);
		delete[] depthbuffers[i];
	}

	xrDestroyInstance(instance);

	delete[] viewconfig_views;
	delete[] projection_views;
	delete[] views;
	delete[] swapchains;
	delete[] images;
	delete[] framebuffers;
	delete[] depthbuffers;
	delete[] swapchain_lengths;
#endif
}

int VRInterface::runtimeEvents() {
#if defined _WIN64 || defined __linux__
	if (quit_mainloop) {
		return 1;
	}

	// TODO, shutdown gracefully using xrRequestExitSession(session);

	// --- Handle runtime Events
		// we do this before xrWaitFrame() so we can go idle or
		// break out of the main render loop as early as possible and don't have to
		// uselessly render or submit one. Calling xrWaitFrame commits you to
		// calling xrBeginFrame eventually.
	XrEventDataBuffer runtime_event;
	runtime_event.type = XR_TYPE_EVENT_DATA_BUFFER;
	runtime_event.next = NULL;
	XrResult poll_result = xrPollEvent(instance, &runtime_event);
	while (poll_result == XR_SUCCESS) {
		switch (runtime_event.type) {
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
			XrEventDataInstanceLossPending* event = (XrEventDataInstanceLossPending*)&runtime_event;
			printf("EVENT: instance loss pending at %lu! Destroying instance.\n", event->lossTime);
			quit_mainloop = true;
			continue;
		}
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
			XrEventDataSessionStateChanged* event = (XrEventDataSessionStateChanged*)&runtime_event;
			printf("EVENT: session state changed from %d to %d\n", state, event->state);
			state = event->state;

			/*
			 * react to session state changes, see OpenXR spec 9.3 diagram. What we need to react to:
			 *
			 * * READY -> xrBeginSession STOPPING -> xrEndSession (note that the same session can be
			 * restarted)
			 * * EXITING -> xrDestroySession (EXITING only happens after we went through STOPPING and
			 * called xrEndSession)
			 *
			 * After exiting it is still possible to create a new session but we don't do that here.
			 *
			 * * IDLE -> don't run render loop, but keep polling for events
			 * * SYNCHRONIZED, VISIBLE, FOCUSED -> run render loop
			 */
			switch (state) {
				// skip render loop, keep polling
			case XR_SESSION_STATE_MAX_ENUM: // must be a bug
			case XR_SESSION_STATE_IDLE:
			case XR_SESSION_STATE_UNKNOWN: {
				run_framecycle = false;

				break; // state handling switch
			}

										 // do nothing, run render loop normally
			case XR_SESSION_STATE_FOCUSED:
			case XR_SESSION_STATE_SYNCHRONIZED:
			case XR_SESSION_STATE_VISIBLE: {
				run_framecycle = true;

				break; // state handling switch
			}

										 // begin session and then run render loop
			case XR_SESSION_STATE_READY: {
				// start session only if it is not running, i.e. not when we already called xrBeginSession
				// but the runtime did not switch to the next state yet
				if (!session_running) {
					XrSessionBeginInfo session_begin_info;
					session_begin_info.type = XR_TYPE_SESSION_BEGIN_INFO;
					session_begin_info.next = NULL;
					session_begin_info.primaryViewConfigurationType = view_type;
					result = xrBeginSession(session, &session_begin_info);
					if (!xr_check(instance, result, "Failed to begin session!"))
						return 1;
					printf("Session started!\n");
					session_running = true;
				}
				// after beginning the session, run render loop
				run_framecycle = true;

				break; // state handling switch
			}

									   // end session, skip render loop, keep polling for next state change
			case XR_SESSION_STATE_STOPPING: {
				// end session only if it is running, i.e. not when we already called xrEndSession but the
				// runtime did not switch to the next state yet
				if (session_running) {
					result = xrEndSession(session);
					if (!xr_check(instance, result, "Failed to end session!"))
						return 1;
					session_running = false;
				}
				// after ending the session, don't run render loop
				run_framecycle = false;

				break; // state handling switch
			}

										  // destroy session, skip render loop, exit render loop and quit
			case XR_SESSION_STATE_LOSS_PENDING:
			case XR_SESSION_STATE_EXITING:
				result = xrDestroySession(session);
				if (!xr_check(instance, result, "Failed to destroy session!"))
					return 1;
				quit_mainloop = true;
				run_framecycle = false;

				break; // state handling switch
			}
			break; // session event handling switch
		}
		// TODO: Controller handling removed here, could be re-added
		default: printf("Unhandled event (type %d)\n", runtime_event.type);
		}

		runtime_event.type = XR_TYPE_EVENT_DATA_BUFFER;
		poll_result = xrPollEvent(instance, &runtime_event);
	}
	if (poll_result == XR_EVENT_UNAVAILABLE) {
		// processed all events in the queue
	}
	else {
		printf("Failed to poll events!\n");
		return 1; // TODO: Different codes?
	}

	return 0;
#else
	std::cout << "VR interface not implemented" << std::endl;
	return 1;
#endif
}

int VRInterface::render(SimulationModel* model) {
#if defined _WIN64 || defined __linux__
	if (!run_framecycle) {
		return 0;
	}

	irr::core::quaternion quat = irr::core::quaternion(0, 0, 0, 1);

	// --- Wait for our turn to do head-pose dependent computation and render a frame
	XrFrameState frame_state;
	frame_state.type = XR_TYPE_FRAME_STATE;
	frame_state.next = NULL;
	XrFrameWaitInfo frame_wait_info;
	frame_wait_info.type = XR_TYPE_FRAME_WAIT_INFO;
	frame_wait_info.next = NULL;
	result = xrWaitFrame(session, &frame_wait_info, &frame_state);
	if (!xr_check(instance, result, "xrWaitFrame() was not successful, exiting..."))
		return 1;

	// --- Create projection matrices and view matrices for each eye
	XrViewLocateInfo view_locate_info;
	view_locate_info.type = XR_TYPE_VIEW_LOCATE_INFO;
	view_locate_info.next = NULL;
	view_locate_info.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	view_locate_info.displayTime = frame_state.predictedDisplayTime;
	view_locate_info.space = play_space;

	XrViewState view_state;
	view_state.type = XR_TYPE_VIEW_STATE;
	view_state.next = NULL;

	result = xrLocateViews(session, &view_locate_info, &view_state, view_count, &view_count, views);
	if (!xr_check(instance, result, "Could not locate views"))
		return 1;

	// TODO: Controller/hand actions would go here

	// --- Begin frame
	XrFrameBeginInfo frame_begin_info;
	frame_begin_info.type = XR_TYPE_FRAME_BEGIN_INFO;
	frame_begin_info.next = NULL;

	result = xrBeginFrame(session, &frame_begin_info);
	if (!xr_check(instance, result, "failed to begin frame!"))
		return 1;

	// render each eye and fill projection_views with the result
	for (uint32_t i = 0; i < view_count; i++) {

		if (!frame_state.shouldRender) {
			printf("shouldRender = false, Skipping rendering work\n");
			continue;
		}

		XrSwapchainImageAcquireInfo acquire_info;
		acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
		acquire_info.next = NULL;

		uint32_t acquired_index;
		result = xrAcquireSwapchainImage(swapchains[i], &acquire_info, &acquired_index);
		if (!xr_check(instance, result, "failed to acquire swapchain image!"))
			break;

		XrSwapchainImageWaitInfo wait_info;
		wait_info.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
		wait_info.next = NULL;
		wait_info.timeout = 1000;

		result = xrWaitSwapchainImage(swapchains[i], &wait_info);
		if (!xr_check(instance, result, "failed to wait for swapchain image!"))
			break;

		projection_views[i].pose = views[i].pose;
		projection_views[i].fov = views[i].fov;

		// TODO: Shouldn't need to do this every loop!
		model->setViewAngle(irr::core::radToDeg(views[i].fov.angleRight - views[i].fov.angleLeft));

		// Binding to Irrlicht views
		irr::core::vector3df eyePos = irr::core::vector3df(projection_views[i].pose.position.x, projection_views[i].pose.position.y, -1.0 * projection_views[i].pose.position.z);
		quat.X = projection_views[i].pose.orientation.x;
		quat.Y = projection_views[i].pose.orientation.y;
		quat.Z = -1.0 * projection_views[i].pose.orientation.z;
		quat.W = -1.0 * projection_views[i].pose.orientation.w;
		
		// Find lens shift, as left and right FOV may not be symmetrical
		// TODO: Probably doesn't need calculating each frame
		float tanLeft = tan(views[i].fov.angleLeft);
		float tanRight = tan(views[i].fov.angleRight);
		float tanUp = tan(views[i].fov.angleUp);
		float tanDown = tan(views[i].fov.angleDown);
		float horizontalShift = -1.0*(tanRight + tanLeft) / (tanRight - tanLeft);
		float verticalShift = -1.0 * (tanUp + tanDown) / (tanUp - tanDown);
		irr::core::vector2df lensShift = irr::core::vector2df(horizontalShift, verticalShift);

		// Send this to the camera
		model->updateCameraVRPos(quat, eyePos, lensShift); // TODO: Check if this is relative to the correct origin

		int w = viewconfig_views[i].recommendedImageRectWidth;
		int h = viewconfig_views[i].recommendedImageRectHeight;

		// Render into swapchain images here (for left or right eye), into images[i][acquired_index].image
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i][acquired_index]);
		
		glBindRenderbuffer(GL_RENDERBUFFER, depthbuffers[0][acquired_index]); // TODO: Why does it only work with [0], not [i]
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffers[0][acquired_index]); // TODO: Why does it only work with [0], not [i]

		glViewport(0, 0, w, h);
		glScissor(0, 0, w, h);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, images[i][acquired_index].image, 0);
		glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Check
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "glCheckFramebufferStatus: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
		}

		// Render
		smgr->drawAll();

		// Return to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		XrSwapchainImageReleaseInfo release_info;
		release_info.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
		release_info.next = NULL;
		result = xrReleaseSwapchainImage(swapchains[i], &release_info);
		if (!xr_check(instance, result, "failed to release swapchain image!"))
			break;
	}

	XrCompositionLayerProjection projection_layer;
	projection_layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
	projection_layer.next = NULL;
	projection_layer.layerFlags = 0;
	projection_layer.space = play_space;
	projection_layer.viewCount = view_count;
	projection_layer.views = projection_views;

	int submitted_layer_count = 1;
	const XrCompositionLayerBaseHeader* submitted_layers[1] = {
		(const XrCompositionLayerBaseHeader* const)&projection_layer };

	if ((view_state.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0) {
		printf("submitting 0 layers because orientation is invalid\n");
		submitted_layer_count = 0;
	}

	if (!frame_state.shouldRender) {
		printf("submitting 0 layers because shouldRender = false\n");
		submitted_layer_count = 0;
	}

	XrFrameEndInfo frameEndInfo;
	frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
	frameEndInfo.displayTime = frame_state.predictedDisplayTime;
	frameEndInfo.layerCount = submitted_layer_count;
	frameEndInfo.layers = submitted_layers;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.next = NULL;
	result = xrEndFrame(session, &frameEndInfo);
	if (!xr_check(instance, result, "failed to end frame!"))
		return 1;

	// Return 0 on success
	return 0;
#else
	std::cout << "VR interface not implemented" << std::endl;
	return 1;
#endif
}

// true if XrResult is a success code, else print error message and return false
bool VRInterface::xr_check(XrInstance instance, XrResult result, const char* format, ...)
{
#if defined _WIN64 || defined __linux__
	if (XR_SUCCEEDED(result))
		return true;

	char resultString[XR_MAX_RESULT_STRING_SIZE];
	xrResultToString(instance, result, resultString);

	char formatRes[XR_MAX_RESULT_STRING_SIZE + 1024];
	snprintf(formatRes, XR_MAX_RESULT_STRING_SIZE + 1023, "%s [%s] (%d)\n", format, resultString,
		result);

	va_list args;
	va_start(args, format);
	vprintf(formatRes, args);
	va_end(args);

	return false;
#else
	std::cout << "VR interface not implemented" << std::endl;
	return false;
#endif
}

void VRInterface::print_api_layers()
{
#if defined _WIN64 || defined __linux__
	uint32_t count = 0;
	XrResult result = xrEnumerateApiLayerProperties(0, &count, NULL);
	if (!xr_check(NULL, result, "Failed to enumerate api layer count"))
		return;

	if (count == 0)
		return;

	//XrApiLayerProperties* props = malloc(count * sizeof(XrApiLayerProperties));
	XrApiLayerProperties* props = new XrApiLayerProperties[count];
	for (uint32_t i = 0; i < count; i++) {
		props[i].type = XR_TYPE_API_LAYER_PROPERTIES;
		props[i].next = NULL;
	}

	result = xrEnumerateApiLayerProperties(count, &count, props);
	if (!xr_check(NULL, result, "Failed to enumerate api layers"))
		return;

	printf("API layers:\n");
	for (uint32_t i = 0; i < count; i++) {
		printf("\t%s v%d: %s\n", props[i].layerName, props[i].layerVersion, props[i].description);
	}

	//free(props)
	delete[] props;
#else
	std::cout << "VR interface not implemented" << std::endl;
#endif
}

void VRInterface::print_instance_properties(XrInstance instance)
{
#if defined _WIN64 || defined __linux__
	XrResult result;
	XrInstanceProperties instance_props;
	instance_props.type = XR_TYPE_INSTANCE_PROPERTIES;
	instance_props.next = NULL;

	result = xrGetInstanceProperties(instance, &instance_props);
	if (!xr_check(NULL, result, "Failed to get instance info"))
		return;

	printf("Runtime Name: %s\n", instance_props.runtimeName);
	printf("Runtime Version: %d.%d.%d\n", XR_VERSION_MAJOR(instance_props.runtimeVersion),
		XR_VERSION_MINOR(instance_props.runtimeVersion),
		XR_VERSION_PATCH(instance_props.runtimeVersion));
#else
	std::cout << "VR interface not implemented" << std::endl;
#endif
}

void VRInterface::print_system_properties(XrSystemProperties* system_properties)
{
#if defined _WIN64 || defined __linux__
	printf("System properties for system %lu: \"%s\", vendor ID %d\n", system_properties->systemId,
		system_properties->systemName, system_properties->vendorId);
	printf("\tMax layers          : %d\n", system_properties->graphicsProperties.maxLayerCount);
	printf("\tMax swapchain height: %d\n",
		system_properties->graphicsProperties.maxSwapchainImageHeight);
	printf("\tMax swapchain width : %d\n",
		system_properties->graphicsProperties.maxSwapchainImageWidth);
	printf("\tOrientation Tracking: %d\n", system_properties->trackingProperties.orientationTracking);
	printf("\tPosition Tracking   : %d\n", system_properties->trackingProperties.positionTracking);
#else
	std::cout << "VR interface not implemented" << std::endl;
#endif
}

void VRInterface::print_viewconfig_view_info(uint32_t view_count, XrViewConfigurationView* viewconfig_views)
{
#if defined _WIN64 || defined __linux__
	for (uint32_t i = 0; i < view_count; i++) {
		printf("View Configuration View %d:\n", i);
		printf("\tResolution       : Recommended %dx%d, Max: %dx%d\n",
			viewconfig_views[0].recommendedImageRectWidth,
			viewconfig_views[0].recommendedImageRectHeight, viewconfig_views[0].maxImageRectWidth,
			viewconfig_views[0].maxImageRectHeight);
		printf("\tSwapchain Samples: Recommended: %d, Max: %d)\n",
			viewconfig_views[0].recommendedSwapchainSampleCount,
			viewconfig_views[0].maxSwapchainSampleCount);
	}
#else
	std::cout << "VR interface not implemented" << std::endl;
#endif
}

// returns the preferred swapchain format if it is supported
// else:
// - if fallback is true, return the first supported format
// - if fallback is false, return -1
int64_t VRInterface::get_swapchain_format(XrInstance instance,
	XrSession session,
	int64_t preferred_format,
	bool fallback)
{
#if defined _WIN64 || defined __linux__
	XrResult result;

	uint32_t swapchain_format_count;
	result = xrEnumerateSwapchainFormats(session, 0, &swapchain_format_count, NULL);
	if (!xr_check(instance, result, "Failed to get number of supported swapchain formats"))
		return -1;

	printf("Runtime supports %d swapchain formats\n", swapchain_format_count);
	//int64_t* swapchain_formats = malloc(sizeof(int64_t) * swapchain_format_count);
	int64_t* swapchain_formats = new int64_t[swapchain_format_count];
	result = xrEnumerateSwapchainFormats(session, swapchain_format_count, &swapchain_format_count,
		swapchain_formats);
	if (!xr_check(instance, result, "Failed to enumerate swapchain formats"))
		return -1;

	int64_t chosen_format = fallback ? swapchain_formats[0] : -1;

	for (uint32_t i = 0; i < swapchain_format_count; i++) {
		printf("Supported GL format: %#lx\n", swapchain_formats[i]);
		if (swapchain_formats[i] == preferred_format) {
			chosen_format = swapchain_formats[i];
			printf("Using preferred swapchain format %#lx\n", chosen_format);
			break;
		}
	}
	if (fallback && chosen_format != preferred_format) {
		printf("Falling back to non preferred swapchain format %#lx\n", chosen_format);
	}

	//free(swapchain_formats);
	delete[] swapchain_formats;

	return chosen_format;
#else
	std::cout << "VR interface not implemented" << std::endl;
	return -1;
#endif
}
