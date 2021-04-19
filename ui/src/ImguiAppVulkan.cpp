#include "ImguiApp.hpp"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // abort
#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

namespace ui
{
  namespace
  {
    void check_vk_result(VkResult err)
    {
      if (err == 0)
        return;
      fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
      if (err < 0)
        abort();
    }

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags,
                                                VkDebugReportObjectTypeEXT objectType,
                                                uint64_t object,
                                                size_t location,
                                                int32_t messageCode,
                                                const char* pLayerPrefix,
                                                const char* pMessage,
                                                void* pUserData)
    {
      (void)flags;
      (void)object;
      (void)location;
      (void)messageCode;
      (void)pUserData;
      (void)pLayerPrefix; // Unused arguments
      fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
      return VK_FALSE;
    }
#endif // IMGUI_VULKAN_DEBUG_REPORT
  }    // namespace

  struct ImguiApp::Data
  {
    VkAllocationCallbacks* g_Allocator = NULL;
    VkInstance g_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice g_Device = VK_NULL_HANDLE;
    uint32_t g_QueueFamily = (uint32_t)-1;
    VkQueue g_Queue = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
    VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
    VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

    ImGui_ImplVulkanH_Window g_MainWindowData;
    const uint32_t g_MinImageCount = 2;

    SDL_Window* m_window;

    void SetupVulkan(const char** extensions, uint32_t extensions_count);
    void SetupVulkanWindow(VkSurfaceKHR surface, int width, int height);
    void CleanupVulkan();
    void CleanupVulkanWindow();

    void FrameRender(ImDrawData* draw_data);
    void FramePresent();
  };

  void ImguiApp::Data::SetupVulkan(const char** extensions, uint32_t extensions_count)
  {
    VkResult err;

    // Create Vulkan Instance
    {
      VkInstanceCreateInfo create_info = {};
      create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      create_info.enabledExtensionCount = extensions_count;
      create_info.ppEnabledExtensionNames = extensions;

#ifdef IMGUI_VULKAN_DEBUG_REPORT
      // Enabling multiple validation layers grouped as LunarG standard validation
      const char* layers[] = {"VK_LAYER_LUNARG_standard_validation"};
      create_info.enabledLayerCount = 1;
      create_info.ppEnabledLayerNames = layers;

      // Enable debug report extension (we need additional storage, so we duplicate the user array to add our new
      // extension to it)
      const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
      memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
      extensions_ext[extensions_count] = "VK_EXT_debug_report";
      create_info.enabledExtensionCount = extensions_count + 1;
      create_info.ppEnabledExtensionNames = extensions_ext;

      // Create Vulkan Instance
      err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
      check_vk_result(err);
      free(extensions_ext);

      // Get the function pointer (required for any extensions)
      auto vkCreateDebugReportCallbackEXT =
          (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
      IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

      // Setup the debug report callback
      VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
      debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
      debug_report_ci.flags =
          VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
      debug_report_ci.pfnCallback = debug_report;
      debug_report_ci.pUserData = NULL;
      err = vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
      check_vk_result(err);
#else
      // Create Vulkan Instance without any debug feature
      err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
      check_vk_result(err);
      IM_UNUSED(g_DebugReport);
#endif
    }

    // Select GPU
    {
      uint32_t gpu_count;
      err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
      check_vk_result(err);
      IM_ASSERT(gpu_count > 0);

      VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
      err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);
      check_vk_result(err);

      // If a number >1 of GPUs got reported, you should find the best fit GPU for your purpose
      // e.g. VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU if available, or with the greatest memory available, etc.
      // for sake of simplicity we'll just take the first one, assuming it has a graphics queue family.
      g_PhysicalDevice = gpus[0];
      free(gpus);
    }

    // Select graphics queue family
    {
      uint32_t count;
      vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
      VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
      vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
      for (uint32_t i = 0; i < count; i++)
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          g_QueueFamily = i;
          break;
        }
      free(queues);
      IM_ASSERT(g_QueueFamily != (uint32_t)-1);
    }

    // Create Logical Device (with 1 queue)
    {
      int device_extension_count = 1;
      const char* device_extensions[] = {"VK_KHR_swapchain"};
      const float queue_priority[] = {1.0f};
      VkDeviceQueueCreateInfo queue_info[1] = {};
      queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_info[0].queueFamilyIndex = g_QueueFamily;
      queue_info[0].queueCount = 1;
      queue_info[0].pQueuePriorities = queue_priority;
      VkDeviceCreateInfo create_info = {};
      create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
      create_info.pQueueCreateInfos = queue_info;
      create_info.enabledExtensionCount = device_extension_count;
      create_info.ppEnabledExtensionNames = device_extensions;
      err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
      check_vk_result(err);
      vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // Create Descriptor Pool
    {
      VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                           {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                           {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                           {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                           {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                           {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                           {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                           {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                           {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                           {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                           {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
      VkDescriptorPoolCreateInfo pool_info = {};
      pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
      pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
      pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
      pool_info.pPoolSizes = pool_sizes;
      err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
      check_vk_result(err);
    }
  }

  // All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
  // Your real engine/app may not use them.
  void ImguiApp::Data::SetupVulkanWindow(VkSurfaceKHR surface, int width, int height)
  {
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
      fprintf(stderr, "Error no WSI support on physical device 0\n");
      exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat,
                                                              (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
                                                              requestSurfaceColorSpace);

    // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
                                        VK_PRESENT_MODE_FIFO_KHR};
#else
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0],
                                                          IM_ARRAYSIZE(present_modes));
    // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator,
                                           width, height, g_MinImageCount);
  }

  void ImguiApp::Data::CleanupVulkan()
  {
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
  }

  void ImguiApp::Data::CleanupVulkanWindow()
  {
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
  }

  void ImguiApp::Data::FrameRender(ImDrawData* draw_data)
  {
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    VkResult err;

    VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE,
                                &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
      return;
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
      err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE,
                            UINT64_MAX); // wait indefinitely instead of periodically checking
      check_vk_result(err);

      err = vkResetFences(g_Device, 1, &fd->Fence);
      check_vk_result(err);
    }
    {
      err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
      check_vk_result(err);
      VkCommandBufferBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
      check_vk_result(err);
    }
    {
      VkRenderPassBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      info.renderPass = wd->RenderPass;
      info.framebuffer = fd->Framebuffer;
      info.renderArea.extent.width = wd->Width;
      info.renderArea.extent.height = wd->Height;
      info.clearValueCount = 1;
      info.pClearValues = &wd->ClearValue;
      vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
      VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkSubmitInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      info.waitSemaphoreCount = 1;
      info.pWaitSemaphores = &image_acquired_semaphore;
      info.pWaitDstStageMask = &wait_stage;
      info.commandBufferCount = 1;
      info.pCommandBuffers = &fd->CommandBuffer;
      info.signalSemaphoreCount = 1;
      info.pSignalSemaphores = &render_complete_semaphore;

      err = vkEndCommandBuffer(fd->CommandBuffer);
      check_vk_result(err);
      err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
      check_vk_result(err);
    }
  }

  void ImguiApp::Data::FramePresent()
  {
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;

    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
      return;
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
  }

  ImguiApp::ImguiApp(const std::string& windowTitle)
    : background_color(0.45f, 0.55f, 0.60f, 1.00f)
    , data(std::make_unique<Data>())
  {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
      throw std::runtime_error(SDL_GetError());

    // Setup window
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    data->m_window =
        SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

    // Setup Vulkan
    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(data->m_window, &extensions_count, NULL);
    const char** extensions = new const char*[extensions_count];
    SDL_Vulkan_GetInstanceExtensions(data->m_window, &extensions_count, extensions);
    data->SetupVulkan(extensions, extensions_count);
    delete[] extensions;

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err;
    if (SDL_Vulkan_CreateSurface(data->m_window, data->g_Instance, &surface) == 0)
      throw std::runtime_error("Failed to create Vulkan surface.\n");

    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(data->m_window, &w, &h);
    data->SetupVulkanWindow(surface, w, h);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForVulkan(data->m_window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = data->g_Instance;
    init_info.PhysicalDevice = data->g_PhysicalDevice;
    init_info.Device = data->g_Device;
    init_info.QueueFamily = data->g_QueueFamily;
    init_info.Queue = data->g_Queue;
    init_info.PipelineCache = data->g_PipelineCache;
    init_info.DescriptorPool = data->g_DescriptorPool;
    init_info.Allocator = data->g_Allocator;
    init_info.MinImageCount = data->g_MinImageCount;
    init_info.ImageCount = data->g_MainWindowData.ImageCount;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, data->g_MainWindowData.RenderPass);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
    // ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application
    // (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double
    // backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL,
    // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

    // Upload Fonts
    {
      // Use any command queue
      const auto& frames = data->g_MainWindowData.Frames[data->g_MainWindowData.FrameIndex];
      VkCommandPool command_pool = frames.CommandPool;
      VkCommandBuffer command_buffer = frames.CommandBuffer;

      err = vkResetCommandPool(data->g_Device, command_pool, 0);
      check_vk_result(err);
      VkCommandBufferBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      err = vkBeginCommandBuffer(command_buffer, &begin_info);
      check_vk_result(err);

      ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

      VkSubmitInfo end_info = {};
      end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      end_info.commandBufferCount = 1;
      end_info.pCommandBuffers = &command_buffer;
      err = vkEndCommandBuffer(command_buffer);
      check_vk_result(err);
      err = vkQueueSubmit(data->g_Queue, 1, &end_info, VK_NULL_HANDLE);
      check_vk_result(err);

      err = vkDeviceWaitIdle(data->g_Device);
      check_vk_result(err);
      ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
  }

  ImguiApp::~ImguiApp()
  {
    VkResult err = vkDeviceWaitIdle(data->g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    data->CleanupVulkanWindow();
    data->CleanupVulkan();

    SDL_DestroyWindow(data->m_window);
    SDL_Quit();
  }

  void ImguiApp::run()
  {
    // Main loop
    bool done = false;
    while (!done)
    {
      // Poll and handle events (inputs, window resize, etc.)
      // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
      // inputs.
      // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
      // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
      // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two
      // flags.
      SDL_Event event;
      std::optional<std::pair<int32_t, int32_t>> newSize;
      while (SDL_PollEvent(&event))
      {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
          done = true;
        else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED &&
                 event.window.windowID == SDL_GetWindowID(data->m_window))
        {
          const auto width = event.window.data1;
          const auto height = event.window.data2;
          if (width > 0 && height > 0)
            newSize = {width, height};
        }
        else
          // Custom event handling
          processEvent(event);
      }

      // Resize swap chain?
      if (newSize.has_value())
      {
        ImGui_ImplVulkan_SetMinImageCount(data->g_MinImageCount);
        ImGui_ImplVulkanH_CreateOrResizeWindow(data->g_Instance, data->g_PhysicalDevice, data->g_Device,
                                               &data->g_MainWindowData, data->g_QueueFamily, data->g_Allocator,
                                               newSize->first, newSize->second, data->g_MinImageCount);
        data->g_MainWindowData.FrameIndex = 0;
      }

      // Start the Dear ImGui frame
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplSDL2_NewFrame(data->m_window);
      ImGui::NewFrame();

      // Custom UI code
      populateFrame();

      // Rendering
      ImGui::Render();
      ImDrawData* draw_data = ImGui::GetDrawData();
      const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
      if (!is_minimized)
      {
        memcpy(&data->g_MainWindowData.ClearValue.color.float32[0], &background_color, 4 * sizeof(float));
        data->FrameRender(draw_data);
        data->FramePresent();
      }
    }
  }
} // namespace ui
