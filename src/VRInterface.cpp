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

#define _CRT_SECURE_NO_WARNINGS //FIXME: Temporary fix

#include "VRInterface.hpp"

// Constructor
VRInterface::VRInterface(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver) {
    this->smgr = smgr;
    this->driver = driver;
}

// Destructor
VRInterface::~VRInterface() {
}

int VRInterface::load() {
	
	// Changing to HANDHELD_DISPLAY or a future form factor may work, but has not been tested.
	XrFormFactor form_factor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	// Changing the form_factor may require changing the view_type too.
	XrViewConfigurationType view_type = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	// Typically STAGE for room scale/standing, LOCAL for seated
	XrReferenceSpaceType play_space_type = XR_REFERENCE_SPACE_TYPE_LOCAL;
	XrSpace play_space = XR_NULL_HANDLE;

	// the instance handle can be thought of as the basic connection to the OpenXR runtime
	XrInstance instance = XR_NULL_HANDLE;
	// the system represents an (opaque) set of XR devices in use, managed by the runtime
	XrSystemId system_id = XR_NULL_SYSTEM_ID;
	// the session deals with the renderloop submitting frames to the runtime
	XrSession session = XR_NULL_HANDLE;

	// each graphics API requires the use of a specialized struct
#ifdef _WIN32
	//XrGraphicsBindingOpenGLWin32KHR graphics_binding_gl = { 0 };
	XrGraphicsBindingOpenGLWin32KHR graphics_binding_gl = { XR_TYPE_UNKNOWN };
#else
	// The runtime interacts with the OpenGL images (textures) via a Swapchain.
	XrGraphicsBindingOpenGLXlibKHR graphics_binding_gl = { 0 };
#endif

	// each physical Display/Eye is described by a view.
	// view_count usually depends on the form_factor / view_type.
	// dynamically allocating all view related structs instead of assuming 2
	// hopefully allows this app to scale easily to different view_counts.
	uint32_t view_count = 0;
	// the viewconfiguration views contain information like resolution about each view
	XrViewConfigurationView* viewconfig_views = NULL;

	// array of view_count containers for submitting swapchains with rendered VR frames
	XrCompositionLayerProjectionView* projection_views = NULL;
	// array of view_count views, filled by the runtime with current HMD display pose
	XrView* views = NULL;

	// array of view_count handles for swapchains.
	// it is possible to use imageRect to render all views to different areas of the
	// same texture, but in this example we use one swapchain per view
	XrSwapchain* swapchains = NULL;
	// array of view_count ints, storing the length of swapchains
	uint32_t* swapchain_lengths = NULL;
	// array of view_count array of swapchain_length containers holding an OpenGL texture
	// that is allocated by the runtime
	XrSwapchainImageOpenGLKHR** images = NULL;

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
	XrResult result = XR_SUCCESS;

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

	// If successfull, return 0
	return 0;
}

// true if XrResult is a success code, else print error message and return false
bool VRInterface::xr_check(XrInstance instance, XrResult result, const char* format, ...)
{
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
}

void VRInterface::print_api_layers()
{
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
}

void VRInterface::print_instance_properties(XrInstance instance)
{
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
}