#ifndef VULKAN_H_
#define VULKAN_H_ 1

/*
** Copyright 2015-2025 The Khronos Group Inc.
**
** SPDX-License-Identifier: Apache-2.0
*/

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan_android.h>
#endif

#ifdef VK_USE_PLATFORM_FUCHSIA
#include <vulkan/zircon/types.h>
#include <vulkan/vulkan_fuchsia.h>
#endif

#ifdef VK_USE_PLATFORM_IOS_MVK
#include <vulkan/vulkan_ios.h>
#endif


#ifdef VK_USE_PLATFORM_MACOS_MVK
#include <vulkan/vulkan_macos.h>
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan_metal.h>
#endif

#ifdef VK_USE_PLATFORM_VI_NN
#include <vulkan/vulkan_vi.h>
#endif


#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan_wayland.h>
#endif


#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/windows.h>
#include <vulkan/vulkan_win32.h>
#endif


#ifdef VK_USE_PLATFORM_XCB_KHR
#include <vulkan/xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#endif


#ifdef VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif


#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
#include <vulkan/directfb.h>
#include <vulkan/vulkan_directfb.h>
#endif


#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
#include <vulkan/X11/Xlib.h>
#include <vulkan/X11/extensions/Xrandr.h>
#include <vulkan/vulkan_xlib_xrandr.h>
#endif


#ifdef VK_USE_PLATFORM_GGP
#include <vulkan/ggp_c/vulkan_types.h>
#include <vulkan/vulkan_ggp.h>
#endif


#ifdef VK_USE_PLATFORM_SCREEN_QNX
#include <vulkan/screen/screen.h>
#include <vulkan/vulkan_screen.h>
#endif


#ifdef VK_USE_PLATFORM_SCI
#include <vulkan/nvscisync.h>
#include <vulkan/nvscibuf.h>
#include <vulkan/vulkan_sci.h>
#endif


#ifdef VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan_beta.h>
#endif

#ifdef VK_USE_PLATFORM_OHOS
#include <vulkan/vulkan_ohos.h>
#endif

#endif // VULKAN_H_
