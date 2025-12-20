
#include "include_core.h"

vulkan_context* g_vulkan = nullptr;

namespace dyn
{
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
}

VKAPI_ATTR VKAPI_CALL
PROC vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data ) -> VkBool32
{
    PROFILE_SCOPE_FUNCTION();
    TYON_BASE_LOGF( "Vulkan Validation", "{}", callback_data->pMessage );

    return VK_FALSE;
}

PROC vulkan_allocator_create_callbacks( i_allocator* allocator )
{
    PROFILE_SCOPE_FUNCTION();
    VkAllocationCallbacks result = {};
    result.pUserData = allocator;

    result.pfnAllocation = [] (
        void* context,
        size_t bytes,
        size_t alignment,
        VkSystemAllocationScope scope
    )
    {
        PROFILE_SCOPE_FUNCTION();
        i_allocator* impl = ptr_cast<i_allocator*>( context );
        void* result_ = impl->allocate_raw( bytes, alignment );
        VULKAN_LOGF( "Allocated {} bytes in driver", bytes );
        VULKAN_LOGF( "\tAllocated Bytes: {} ", string_VkSystemAllocationScope(scope) );
        VULKAN_LOGF( "Allocation Address: {} ", result_ );
        // VULKAN_LOGF( "Memory Statistics {}", impl->get_memory_statistics() );

        return result_;
    };

    result.pfnReallocation = [] (
        void* context,
        void* original,
        size_t bytes,
        size_t alignment,
        VkSystemAllocationScope scope
    )
    {   i_allocator* impl = ptr_cast<i_allocator*>( context );
        // TYON_BREAK();
        void* result_ = impl->allocate_relocate( original, alignment );
        VULKAN_LOGF( "Reallocated {} bytes in driver", bytes );
        VULKAN_LOGF( "\tReallocated Bytes: {} ", string_VkSystemAllocationScope(scope) );
        VULKAN_LOGF( "Re-allocation Address: {} ", original );
        return result_;
    };

    result.pfnFree = [] (
        void* context,
        void* address
    )
    {   i_allocator* impl = ptr_cast<i_allocator*>( context );
        impl->deallocate( address );
    };

    // Allocation Notification
    result.pfnInternalAllocation = [](
        void* context,
        size_t bytes,
        VkInternalAllocationType type,
        VkSystemAllocationScope scope
    )
    {
        VULKAN_LOG( "Allocation Request Event" );
        VULKAN_LOGF( "\tAllocator Address: {}", (void*)(context) );
        VULKAN_LOGF( "\tAllocation Type: {} ", string_VkInternalAllocationType(type) );
        VULKAN_LOGF( "\tAllocated Bytes: {} ", string_VkSystemAllocationScope(scope) );
    };

    result.pfnInternalFree = [](
        void* context,
        size_t bytes,
        VkInternalAllocationType type,
        VkSystemAllocationScope scope
    )
    {
        VULKAN_LOG( "Deallocation Request Event" );
        VULKAN_LOGF( "\tAllocator Address: {}", (void*)(context) );
        VULKAN_LOGF( "\tAllocation Type: {} ", string_VkInternalAllocationType(type) );
        VULKAN_LOGF( "\tAllocated Bytes: {} ", string_VkSystemAllocationScope(scope) );
    };
    return result;
}

PROC vulkan_label_object( u64 handle, VkObjectType type, fstring name ) -> void
{
    // NOTE: This is the depreceated version of the struct/function, this crashed when I used it.
    // VkDebugMarkerObjectNameInfoEXT name_args {};
    char* s = memory_allocate_raw( name.size() );
    name.copy( s, name.size(), 0 );
    VkDebugUtilsObjectNameInfoEXT name_args {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = s,
    };
    dyn::vkSetDebugUtilsObjectNameEXT( g_vulkan->logical_device, &name_args );
}

template <typename t_return, typename t_source>
PROC as( t_source arg ) -> t_return;

/** Narrowing cast for VkExtent2D */
template<>
PROC as( v2 arg ) -> VkExtent2D
{
    return VkExtent2D { u32(arg.x), u32(arg.y) };
}

PROC vulkan_extent_2d_cast( v2 arg ) -> VkExtent2D
{
    return VkExtent2D { static_cast<u32>(arg.x), u32(arg.y) };
}

PROC vulkan_shader_init( vulkan_shader* arg ) -> fresult
{
    PROFILE_SCOPE_FUNCTION();
    ERROR_GUARD( arg->id.valid() == false,
                 "Using init on a object that's already initialized can't possible make sense." );
    ERROR_GUARD( arg->code_binary, "Code provided must be binary SPIR-V currently" );
    arg->code = file_load_binary( arg->code.filename );
    ERROR_GUARD( arg->code.file_loaded, "Failed to load file" );

    arg->id = uuid_generate();

    VkShaderModuleCreateInfo shader_args{};
    shader_args.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_args.codeSize = arg->code.memory.size;
    shader_args.pCode = arg->code.memory.data;

    VkShaderModule& platform_module = arg->platform_module;
    auto module_ok = vkCreateShaderModule(
        g_vulkan->logical_device, &shader_args, g_vulkan->vk_allocator, &platform_module );
    if (module_ok != VK_SUCCESS)
    {
        VULKAN_ERRORF( "Failed to create shader module: {}", arg->name );
        return false;
    }
    vulkan_label_object(
        (u64)platform_module, VK_OBJECT_TYPE_SHADER_MODULE, arg->name + "_shader" );
    fstring name = arg->name;
    g_vulkan->resources.push_cleanup( [name, platform_module]{
        VULKAN_LOG( "Destroying shader module:", name );
        vkDestroyShaderModule(
            g_vulkan->logical_device, platform_module, g_vulkan->vk_allocator ); } );

    VULKAN_LOGF( "Created shader module: {}", arg->name );
    return true;
}

// Mesh specific pipeline initialization
PROC vulkan_mesh_pipeline_init( vulkan_pipeline* arg ) -> fresult
{
    if (arg->id.valid() == false)
    {   VULKAN_ERRORF("{} Using init on a object that's already initialized doesn't make any sense.",
                      arg->name );
        return false;
    }
    if (arg->shaders.size() <= 0)
    {   VULKAN_ERRORF( "{} Tried to create a shader pipeline with no shaders attached.",
                       arg->name );
        return false;
    }

    arg->id = uuid_generate();

    array<VkPipelineShaderStageCreateInfo> stages;
    stages.change_allocation( pipeline.shaders.size() );
    for (i64 i=0; i < pipeline.shaders.size(); ++i)
    {
        auto& x_shader = pipeline.shaders[i];
        stages.push_tail( VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = x_shader.stage_flag,
                .module_ = x_shader.platform_module,
                .pName = x_shader.entry_point.c_str()
            });
    }

    /* Colour Format

     VkFormatProperties here describes the colour format that will be used for
     simple diffuse texturing of a mesh that this pipeline will process. This is
     speific to mesh pipelining but the mesh is generic enough that it can be
     used for many different renderable objects. */
    VkFormatProperties format_props {};
    vkGetPhysicalDeviceFormatProperties(
        g_vulkan->device, VK_FORMAT_B8G8R8A8_UNORM, &format_props );
    // Mandated by the spec when setting vertex bindings
    bool vertex_buffer_ok = format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
    if (!vertex_buffer_ok)
    { VULKAN_ERROR( "Format feature vertex buffer not supported for specified format" ); }

    /* SECTION: Create vertex buffer for describing a mesh

     Here we create device memory to hold the data describing a mesh, like
     vertecies and texture data.

     As per the Vulkan Specification Glossary- "memory", is a handle to the
     actual physical memory or a memory allocation we are talking about.

     A "buffer", is "a resource that represents a linear array of data in device
     memory. Represented by a VkBuffer" object. A memory object must be bound to
     a buffer to be used properly.

     TODO: This will have to be removed in future to account for mesh swapping */
    VkDeviceMemory& vertex_memory = g_vulkan->vertex_memory;
    VkBufferCreateInfo buffer_args {};
    buffer_args.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_args.size = sizeof(f32) * self->test_triangle_data.size();
    buffer_args.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_args.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto triangle_buffer_bad = vkCreateBuffer(
        self->logical_device, &buffer_args, g_vulkan->vk_allocator, &self->test_triangle_buffer);
    if (triangle_buffer_bad)
    {   VULKAN_ERROR( "Failed to create test triangle buffer" );
        return false;
    }
    vulkan_label_object(
        (u64)self->test_triangle_buffer, VK_OBJECT_TYPE_BUFFER, "test_triangle_buffer" );
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying test triangle buffer" );
        vkDestroyBuffer( g_vulkan->logical_device, g_vulkan->test_triangle_buffer,
                         g_vulkan->vk_allocator );
    } );

    /* Before we can allocate memory we need to get some information about what
     memory we can allocate. And we'll also how we want the driver to manage the
     code */
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements( self->logical_device, self->test_triangle_buffer,
                                   &memory_requirements );

    VkPhysicalDeviceMemoryProperties memory_props {};
    vkGetPhysicalDeviceMemoryProperties( self->device, &memory_props );

    VkMemoryPropertyFlags memory_filter = (
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    bool match = false;
    i32 memory_type = 0;
    for (i32 i=0; i < memory_props.memoryTypeCount; ++i)
    {
        // TYON_BREAK();
        match = false;
        bool requirement_fulfilled = memory_requirements.memoryTypeBits & (1 << i);
        bool filter_match = (memory_props.memoryTypes[i].propertyFlags & memory_filter);
        match = (requirement_fulfilled && filter_match);
        if (match) { memory_type = i; }
    }
    if (match == false)
    {   VULKAN_ERROR( "Couldn't find suitible memory type for test_triangle_buffer" );
        return false;
    }

    VkMemoryAllocateInfo memory_args {};
    memory_args.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_args.allocationSize = memory_requirements.size;
    memory_args.memoryTypeIndex = memory_type;
    auto memory_bad = vkAllocateMemory(
        self->logical_device, &memory_args, g_vulkan->vk_allocator, &g_vulkan->vertex_memory);
    if (memory_bad)
    {   VULKAN_ERROR( "Failed to allocate memory for triangle buffer" );
        return false;
    }
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying memory allocated for test triangle" );
        vkFreeMemory( g_vulkan->logical_device, g_vulkan->vertex_memory,
                      g_vulkan->vk_allocator );
    });
    vkBindBufferMemory( self->logical_device, self->test_triangle_buffer, vertex_memory, 0 );

    void* data;
    vkMapMemory( self->logical_device, vertex_memory, 0, buffer_args.size, 0, &data );
    memcpy( data, self->test_triangle_data.data, size_t(buffer_args.size) );
    // TODO: Pretty sure we can just unmap this immediately right?
    // self->resources.push_cleanup([=] {
    vkUnmapMemory( self->logical_device, vertex_memory );
    // });

    // Flush memory to make sure its used
    VkMappedMemoryRange range {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = vertex_memory,
        .offset = 0,
        .size = buffer_args.size,
    };
    // TODO: remove before flight
    // vkFlushMappedMemoryRanges( self->logical_device, 1, range );

/* Create swapchain before pipeline so we can pass present surface current extent
   But after render pass... because it gets passed in the swapchain */
    g_vulkan->swapchain.name = "version_0";
    vulkan_swapchain_init( &g_vulkan->swapchain, VK_NULL_HANDLE );


    array<VkVertexInputBindingDescription> bindings {
        // {   // Normal binding
        //     .binding = 0, // vertex attribute binding/slot. leave as 0
        //     .stride = 4 * 6, // 3 32-bit vertexes + 3 colours 32-bit
        //     // Not sure what this is. think it means pulling from instance wide stuffs?
        //     .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
        // },
        {   // Vertex Binding
            .binding = 0, // vertex attribute binding/slot. leave as 0
            .stride = 4 * 6, // 3 32-bit vertexes + 3 colours 32-bit
            // Not sure what this is. think it means pulling from instance wide stuffs?
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    array<VkVertexInputAttributeDescription> vertex_attributes {
        {
            .location = 2, // shader specific binding location
            .binding = 0,
            // This uses the color format for some strange reason. This is a 32-bit vec3
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0, // 3 32-bit normals to vertex data
        },
        {
            .location = 1, // shader specific binding location
            .binding = 0,
            // This uses the color format for some strange reason. This is a 32-bit vec3
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 4 * 3, // 3 32-bit normals to vertex data
        }
    };

    // Mesh vertex input args
    VkPipelineVertexInputStateCreateInfo vertex_args {};
    vertex_args.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_args.pVertexBindingDescriptions =  bindings.data;
    vertex_args.vertexBindingDescriptionCount = bindings.size();
    vertex_args.pVertexAttributeDescriptions = vertex_attributes.data;
    vertex_args.vertexAttributeDescriptionCount = vertex_attributes.size();

    // Mesh rendering settings
    VkPipelineInputAssemblyStateCreateInfo input_args {};
    input_args.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_args.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_args.primitiveRestartEnable = VK_FALSE;

    // Rasterizer settings
    VkPipelineRasterizationStateCreateInfo raster_args {};
    raster_args.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_args.polygonMode = VK_POLYGON_MODE_FILL;
    raster_args.lineWidth = 1.0f;
    raster_args.cullMode = VK_CULL_MODE_BACK_BIT;
    raster_args.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_args.depthBiasEnable = VK_FALSE;
    raster_args.depthBiasConstantFactor = 0.0f;         // Optional
    raster_args.depthBiasClamp = 0.0f;                  // Optional
    raster_args.depthBiasSlopeFactor = 0.0f;            // Optional

    VkViewport viewport_config {
        // Upper left coordinates
        .x = 0,
        .y = 0,
        .width = float(self->swapchain.present_size.width),
        .height = float(self->swapchain.present_size.height),
        // Configurable viewport depth, can configurable but usually between 0 and 1
        .minDepth = 0.0,
        .maxDepth = 1.0
    };

    // Only render into a certain portion of the viewport with scissors
    VkRect2D scissor_config {
        VkOffset2D { 0, 0 },
        self->swapchain.present_size
    };

    VkPipelineViewportStateCreateInfo viewport_args {};
    viewport_args.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_args.pViewports = &viewport_config;
    // viewportCount and scissorCount must be the same
    viewport_args.viewportCount = 1;
    viewport_args.pScissors = &scissor_config;
    viewport_args.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisample_args {};
    multisample_args.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_args.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_args.sampleShadingEnable = true;
    multisample_args.minSampleShading = 0.2f;
    // wut is this
    multisample_args.pSampleMask = nullptr;
    multisample_args.alphaToCoverageEnable = false;
    multisample_args.alphaToOneEnable = false;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = (
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo color_blend_args{};
    color_blend_args.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_args.logicOpEnable = VK_FALSE;
    color_blend_args.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blend_args.attachmentCount = 1;
    color_blend_args.pAttachments = &color_blend_attachment;
    color_blend_args.blendConstants[0] = 0.0f; // Optional
    color_blend_args.blendConstants[1] = 0.0f; // Optional
    color_blend_args.blendConstants[2] = 0.0f; // Optional
    color_blend_args.blendConstants[3] = 0.0f; // Optional

    /* NOTE: A pipeline can be set to have some of it's state become dynamic
       after creation.  Which may be a performance benefit for tasks its
       relevant to. This setion describes what state can be dynamic instead of
       static.*/
    array<VkDynamicState> dynamic_states_selected = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_args {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = cast<u32>(dynamic_states_selected.size()),
        .pDynamicStates = dynamic_states_selected.data
    };

    VkPipelineLayout& layout = g_vulkan->pipeline_layout;
    VkDescriptorSetLayout& descriptor_layout = g_vulkan->descriptor_layout;

    // Set pipeline bindings here
    VkDescriptorSetLayoutCreateInfo descriptor_layout_args {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = 0x0,
        .bindingCount = 0,
        .pBindings = nullptr,
    };

    /* Pipeline Layout: "An object defining the set of resources (via a
       collection of descriptor set layouts) and push constants used by
       pipelines that are created using the layout. Used when creating a
       pipeline and when binding descriptor sets and setting push constant
       values. Represented by a VkPipelineLayout object." */
    VkPipelineLayoutCreateInfo layout_args {};
    layout_args.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_args.pSetLayouts = &descriptor_layout;
    layout_args.setLayoutCount = 1;
    layout_args.pushConstantRangeCount = 0;
    layout_args.pPushConstantRanges = nullptr;

    auto descriptor_layout_ok = vkCreateDescriptorSetLayout(
        g_vulkan->logical_device, &descriptor_layout_args, g_vulkan->vk_allocator, &descriptor_layout );
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying descriptor layout set" );
        vkDestroyDescriptorSetLayout( g_vulkan->logical_device, g_vulkan->descriptor_layout,
                                      g_vulkan->vk_allocator );
    });

    auto layout_bad = vkCreatePipelineLayout(
        g_vulkan->logical_device, &layout_args, g_vulkan->vk_allocator, &layout );
    if (layout_bad)
    {   VULKAN_ERROR( "Faled to create pipeline layout" );
        return false;
    }
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying pipeline layout" );
        vkDestroyPipelineLayout( g_vulkan->logical_device, g_vulkan->pipeline_layout,
                                 g_vulkan->vk_allocator );
    });

    /* Section: Pipeline creation args
     *
     * Now We have all the pipeline information set we can assemble it into
     * creation args */
    VkGraphicsPipelineCreateInfo pipeline_args {};
    pipeline_args.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_args.pStages = stages.data;
    pipeline_args.stageCount = stages.size();
    pipeline_args.pVertexInputState = &vertex_args;
    pipeline_args.pInputAssemblyState = &input_args;
    pipeline_args.pViewportState = &viewport_args;
    pipeline_args.pRasterizationState = &raster_args;
    pipeline_args.pMultisampleState = &multisample_args;
    // pipeline_args.pDepthStencilState = nullptr; // Optional
    pipeline_args.pColorBlendState = &color_blend_args;
    pipeline_args.pDynamicState = &dynamic_state_args;
    pipeline_args.layout = layout;
    pipeline_args.renderPass = render_pass;
    pipeline_args.subpass = 0;
    // pipeline_args.basePipelineHandle = VK_NULL_HANDLE; // Optional
    // pipeline_args.basePipelineIndex = -1; // Optional

    // Provide pipeline cache here if relevant
    auto pipeline_ok = vkCreateGraphicsPipelines(
        g_vulkan->logical_device,
        VK_NULL_HANDLE,
        1,
        &pipeline_args,
        g_vulkan->vk_allocator,
        &g_vulkan->pipeline );
    if (pipeline_ok)
    {   VULKAN_ERROR( "Failed to create graphics pipeline" ); return false; }
    VULKAN_LOG( "Created graphics pipeline" );
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying graphics pipeline" );
        vkDestroyPipeline( g_vulkan->logical_device, g_vulkan->pipeline,
                           g_vulkan->vk_allocator );
    });

    return false;
}

PROC vulkan_swapchain_init( vulkan_swapchain* arg, VkSwapchainKHR reuse_swapchain )
    -> fresult
{
    TIME_SCOPED_FUNCTION();
    TracyCZoneN( zone_1, "Zone 1", true );
    ERROR_GUARD( arg->id.valid(),
                 "Using init on a object that's already initialized can't possible make sense." );
    ERROR_GUARD( arg->initialized, "Called init on an already initialized swapchain" );
    auto self = g_vulkan;
    arg->id = uuid_generate();

    VkAllocationCallbacks allocator = vulkan_allocator_create_callbacks(
        g_vulkan->allocator.get() );

    /* We need to know the capabilities of the surface associated with the physical device
       So we retreive those capabilities */
    VkSurfaceCapabilitiesKHR surface_capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        g_vulkan->device, g_vulkan->surface, &surface_capabilities );
    arg->present_size = surface_capabilities.currentExtent;
    VkExtent2D min = surface_capabilities.minImageExtent;
    VkExtent2D max = surface_capabilities.maxImageExtent;
    VkExtent2D current = surface_capabilities.currentExtent;
    VULKAN_LOGF(
        "Present surface/image extent min: {} {} max: {} {} current {} {}",
        min.width, min.height, max.width, max.height, current.width, current.height
    );
    VULKAN_LOG( "Current X11 window size: {}", g_render->window_size );

    TracyCZoneEnd( zone_1 );
    TracyCZoneN( zone_2, "Zone 2", true );
    // Also diagnostics for device formats
    array<VkSurfaceFormatKHR> surface_formats;
    u32 n_surfaces {};
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        g_vulkan->device, g_vulkan->surface, &n_surfaces, nullptr );
    surface_formats.resize( n_surfaces );
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        g_vulkan->device, g_vulkan->surface, &n_surfaces, surface_formats.data );

    TracyCZoneEnd( zone_2 );
    TracyCZoneNC( zone_3, "Zone 3", 0xA04040, true );

    // --  Create swapchan for presentation of images to windows --

    /* NOTE: noooooooooo, my platform doesn't support swapchain maintainence
       extension. The spec was made after my current nVIDIA driver */
    // VkSwapchainPresentScalingCreateInfoKHR swapchain_present_scaling_args {
    //     .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_KHR
    // };

    VkSwapchainCreateInfoKHR swapchain_args {};
    swapchain_args.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_args.minImageCount = g_vulkan->frame_max_inflight;
    swapchain_args.surface = g_vulkan->surface;
    swapchain_args.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    // The format must match the physical surface formats
    swapchain_args.imageFormat = self->swapchain_image_format;
    swapchain_args.imageExtent = arg->present_size;
    swapchain_args.imageArrayLayers = 1; // More than 1 if a stereoscopic application
    swapchain_args.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_args.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_args.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchain_args.queueFamilyIndexCount = 2;
    u32 family_indexes[] = { u32(self->graphics_queue_family), u32(self->present_queue_family) };
    swapchain_args.pQueueFamilyIndices = family_indexes;
    swapchain_args.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_args.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_args.oldSwapchain = reuse_swapchain;

    auto swapchain_ok = vkCreateSwapchainKHR(
        g_vulkan->logical_device,
        &swapchain_args,
        g_vulkan->vk_allocator,
        &arg->platform_swapchain
    );
    if (swapchain_ok != VK_SUCCESS)
    {
        VULKAN_ERRORF( "Failed to initialize swapchain {}", string_VkResult(swapchain_ok) );
        return false;
    }
    VULKAN_LOG( "Initialized presentation swapchain" );
    arg->initialized = true;

    TracyCZoneEnd( zone_3 );
    TracyCZoneNC( zone_4, "Zone 4", 0xA040A0, true );
    array<VkImage>& swapchain_images = g_vulkan->swapchain_images;
    u32 n_swapchain_images = 0;
    vkGetSwapchainImagesKHR( g_vulkan->logical_device, arg->platform_swapchain,
                             &n_swapchain_images, nullptr );
    swapchain_images.resize( n_swapchain_images );
    vkGetSwapchainImagesKHR( g_vulkan->logical_device, arg->platform_swapchain,
                             &n_swapchain_images, swapchain_images.data );

/** Views describe how to interpret VkImage's, VkImages are related to
    textures and framebuffers */
    array<VkImageView>& swapchain_image_views = g_vulkan->swapchain_image_views;
    array<VkFramebuffer>& swapchain_buffers = g_vulkan->swapchain_framebuffers;
    array<VkResult> view_errors;
    array<VkResult> framebuffer_errors;
    array<VkResult> fence_errors;
    swapchain_image_views.resize( n_swapchain_images );
    swapchain_buffers.resize( n_swapchain_images );

    view_errors.resize( n_swapchain_images );
    framebuffer_errors.resize( n_swapchain_images );
    fence_errors.resize( n_swapchain_images );

    // Don't forget to setup object arrays as well
    arg->frame_end_fences.resize( n_swapchain_images );
    TracyCZoneEnd( zone_4 );
    TracyCZoneNC( zone_5, "Zone 5", 0xA0A040, true );
    for (i32 i = 0; i < n_swapchain_images; i++)
    {
        // Make synchronization primitives
        VkFenceCreateInfo fence_args {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            // Start signalled
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        fence_errors[i] = vkCreateFence(
            g_vulkan->logical_device, &fence_args, g_vulkan->vk_allocator, arg->frame_end_fences.address(i) );
        vulkan_label_object( (u64)arg->frame_end_fences[i], VK_OBJECT_TYPE_FENCE,
                             fmt::format( "{}_frame_end_fence_{}", arg->name, i ) );


        VkImageViewCreateInfo view_args{};
        view_args.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_args.image = swapchain_images[i];
        view_args.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_args.format = swapchain_args.imageFormat;
        view_args.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_args.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_args.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_args.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_args.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_args.subresourceRange.baseMipLevel = 0;
        view_args.subresourceRange.levelCount = 1;
        view_args.subresourceRange.baseArrayLayer = 0;
        view_args.subresourceRange.layerCount = 1;
        view_errors[i] = vkCreateImageView(
            g_vulkan->logical_device, &view_args, g_vulkan->vk_allocator, &swapchain_image_views[i] );
        vulkan_label_object( (u64)swapchain_image_views[i], VK_OBJECT_TYPE_IMAGE_VIEW,
                             fmt::format( "{}_swapchain_image_view_{}", arg->name, i ) );

        VkImageView attachments[] = {
            swapchain_image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_args{};
        framebuffer_args.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_args.renderPass = self->render_pass;
        framebuffer_args.attachmentCount = 1;
        framebuffer_args.pAttachments = attachments;
        framebuffer_args.width = arg->present_size.width;
        framebuffer_args.height = arg->present_size.height;
        framebuffer_args.layers = 1;

        framebuffer_errors[i] = vkCreateFramebuffer(
            g_vulkan->logical_device, &framebuffer_args,
            g_vulkan->vk_allocator, &swapchain_buffers[i] );
        vulkan_label_object( (u64)swapchain_buffers[i], VK_OBJECT_TYPE_FRAMEBUFFER,
                             fmt::format( "{}_swapchain_buffer_{}", arg->name, i ) );
        ERROR_GUARD(view_errors[i] == VK_SUCCESS , "view creation error" );
        ERROR_GUARD(framebuffer_errors[i] == VK_SUCCESS , "fraembuffer creation error" );
    }
    TracyCZoneEnd( zone_5 );
    ERROR_GUARD( arg->id.valid(), "All entities have an  UUID" );
    ERROR_GUARD( arg->platform_swapchain, "Function ended with null swapchain handle" );
    return false;
}

PROC vulkan_swapchain_destroy( vulkan_swapchain* arg ) -> void
{
    TIME_SCOPED_FUNCTION();
    VkAllocationCallbacks allocator = vulkan_allocator_create_callbacks(
        g_vulkan->allocator.get() );

    // Can't destroy resources that are still in use
    vkDeviceWaitIdle( g_vulkan->logical_device );
    vkDestroyFramebuffer(
        g_vulkan->logical_device,
        arg->platform_framebuffer,
        g_vulkan->vk_allocator
    );
    // TODO: Switch size to using n_images
    for (i32 i=0; i < g_vulkan->swapchain_image_views.size(); ++i)
    {
        vkDestroyFramebuffer(
            g_vulkan->logical_device, g_vulkan->swapchain_framebuffers[i], g_vulkan->vk_allocator );
        // TODO: Make sure this are associated with swapchain instead later for
        // multi-swapchain support
        vkDestroyImageView(
            g_vulkan->logical_device, g_vulkan->swapchain_image_views[i], g_vulkan->vk_allocator );
        vkDestroyFence( g_vulkan->logical_device, arg->frame_end_fences[i], g_vulkan->vk_allocator );
    }
    // Lazy destroy swapcarch vmhain because the handle still needs to be reused by next swapchain
    auto swapchain = arg->platform_swapchain;
    g_vulkan->resources.push_cleanup( [=]
    {
        vkDestroySwapchainKHR(
            g_vulkan->logical_device,
            swapchain,
            g_vulkan->vk_allocator
        );
    });
    *arg = vulkan_swapchain {};
}

PROC vulkan_init() -> fresult
{
    PROFILE_SCOPE_FUNCTION();
    fresult result = false;
    g_vulkan = memory_allocate<vulkan_context>( 1 );
    auto self = g_vulkan;
    auto& instance = g_vulkan->instance;
    VkInstanceCreateInfo instance_args = {};
    VkApplicationInfo app_info = {};
    VkXlibSurfaceCreateInfoKHR surface_args = {};
    i32 graphics_queue_family = self->graphics_queue_family;
    i32 present_queue_family = self->present_queue_family;

    g_vulkan->allocator_callback = vulkan_allocator_create_callbacks(
        g_vulkan->allocator.get() );
    // TODO: Remove Vulkan allocaotr temporarily
    g_vulkan->vk_allocator = nullptr;

    defer_procedure _exit = [&result] {
        if (result)
        { VULKAN_LOG( "Vulkan Initialized" ); }
        else
        {
            VULKAN_ERROR( "Vulkan initialization failure, cleaning up resources." );
            g_vulkan->resources.~resource_arena();
        }
    };


    // Enumerate intance layers
    u32 n_layers = 0;
    vkEnumerateInstanceLayerProperties( &n_layers, nullptr );

    array<VkLayerProperties> layers;
    layers.change_allocation( n_layers );
    VkResult enumerate_layer_ok = vkEnumerateInstanceLayerProperties( &n_layers, layers.data );

    // -- Setup extensions and layers --
    array<cstring> enabled_layers = {
        "VK_LAYER_KHRONOS_validation", // debug validaiton layer
    };
    array<cstring> enabled_extensions = {
        // Surface for common window and compositing tasks
        VK_KHR_SURFACE_EXTENSION_NAME,
        // xlib windowing extension
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        // message callback and debug stuff
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        // Dependency for 'vkDebugMarkerSetObjectNameEXT'
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        // Promoted to debug_utils, might still required for older versions, but not modern nvidia
        // VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
    };
    app_info.pApplicationName = "Tachyon Engine";
    app_info.applicationVersion = VK_MAKE_API_VERSION( 0, 0, 1, 0 );
    app_info.pEngineName = "Tachyon Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION( 0, 0, 1, 0 );
    app_info.apiVersion = 0;

    VkDebugUtilsMessengerCreateInfoEXT messenger_args {};
    messenger_args.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger_args.messageSeverity = (
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    );
    messenger_args.messageType = (
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    );
    messenger_args.pfnUserCallback = vulkan_debug_callback;

    instance_args.pApplicationInfo = &app_info;
    instance_args.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_args.enabledLayerCount = enabled_layers.size();
    instance_args.ppEnabledLayerNames = enabled_layers.data;
    instance_args.enabledExtensionCount = enabled_extensions.size();
    instance_args.ppEnabledExtensionNames = enabled_extensions.data;
    instance_args.pNext = &messenger_args;


    VkResult instance_ok = vkCreateInstance(
        &instance_args, g_vulkan->vk_allocator, &g_vulkan->instance );
    if (instance_ok != VK_SUCCESS)
    {
        VULKAN_ERROR( "Failed to create Vulkan instance" );
        return false;
    }
    g_vulkan->resources.push_cleanup( []{
        VULKAN_LOG( "Destroying Vulkan instance" );
        vkDestroyInstance( g_vulkan->instance, g_vulkan->vk_allocator );
        g_vulkan->instance = VK_NULL_HANDLE;
    });

    // need to do this to get validation layer callbacks aparently?
    auto dyn_vkCreateDebugUtilsMessengerEXT = PFN_vkCreateDebugUtilsMessengerEXT(
        vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" ) );
    dyn::vkDestroyDebugUtilsMessengerEXT = PFN_vkDestroyDebugUtilsMessengerEXT(
        vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" ) );

    VkDebugUtilsMessengerEXT debug_messenger {};
    dyn_vkCreateDebugUtilsMessengerEXT(
        instance, &messenger_args, g_vulkan->vk_allocator, &debug_messenger );
    if (debug_messenger)
    {   g_vulkan->resources.push_cleanup( [debug_messenger]
        {   dyn::vkDestroyDebugUtilsMessengerEXT(
                g_vulkan->instance, debug_messenger, g_vulkan->vk_allocator );
        } );
    }

    // For debug naming
    dyn::vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)
    vkGetInstanceProcAddr( g_vulkan->instance, "vkSetDebugUtilsObjectNameEXT" );

    // -- Setup default window surfaces --
    if (g_x11->server && g_x11->window)
    {
        // TODO: Need to chain a present struct to try scaling options
        surface_args.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surface_args.dpy = g_x11->server;
        surface_args.flags = 0x0;
        surface_args.window = g_x11->window;

        VULKAN_LOG( "Creating Vulkan/Xlib drawing surface" );
        VkResult surface_ok = vkCreateXlibSurfaceKHR(
            g_vulkan->instance,
            &surface_args,
            g_vulkan->vk_allocator,
            &g_vulkan->surface
        );
        ERROR_GUARD( surface_ok == VK_SUCCESS, "Vulkan/xlib Surface creation error" );
        VULKAN_LOG( "Tachyon Vulkan", "Vulkan Instance created" );
        g_vulkan->resources.push_cleanup( [] {
            VULKAN_LOGF( "Destroying surface 0x{:x}", u64(g_vulkan->surface) );
            vkDestroySurfaceKHR(
                g_vulkan->instance, g_vulkan->surface, g_vulkan->vk_allocator );
        });
    }
    else
    { VULKAN_LOG( "Vulkan surface could not be created for platform Xlib" ); }

    // -- Device Enumeration and Selection --
    array<VkPhysicalDevice> devices;
    u32 n_devices = 0;
    vkEnumeratePhysicalDevices( instance, &n_devices, nullptr );
    devices.resize( n_devices );
    vkEnumeratePhysicalDevices( instance, &n_devices, devices.data );

    if (n_devices <= 0)
    {
        VULKAN_ERROR( "No physical devices found. Bailing Vulkan initialization" );
        return false;
    }
    for (i32 i=0; i < devices.size(); ++i)
    {
        VkPhysicalDevice x_device = devices[i];
        VkPhysicalDeviceProperties props;
        bool suitible = true;
        bool dedicated_graphics = false;
        vkGetPhysicalDeviceProperties( x_device, &props );
        VULKAN_LOGF( "Enumerated physical device: {} | {:x}:{:x}",
                     props.deviceName, props.vendorID, props.deviceID );
        /* The driver version is literally trash. It's a vendor specific bitmask
         * so you could never hope to get it a coherent number without a complex
         * codepath figuring out which bitmask to use for each card and also
         * somehow finding all the different vendor bitmasks in history. */
        VULKAN_LOGF(
            "    Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(props.apiVersion),
            VK_VERSION_MINOR(props.apiVersion), VK_VERSION_PATCH(props.apiVersion) );
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            dedicated_graphics = true;
            VULKAN_LOG( "    Device Type: Discrete GPU");
        }

        array<VkQueueFamilyProperties> families;
        u32 n_families = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( x_device, &n_families, nullptr );
        families.resize( n_families );
        vkGetPhysicalDeviceQueueFamilyProperties( x_device, &n_families, families.data );
        VULKAN_LOG( "Queue Family Count: ", n_families );

        // HACK: Just autofill first queue family
        // if (n_families)
        // {   self->graphics_queue_family = 0;
        //     self->present_queue_family = 0;
        // }

        /* Seriously... Why. you have to reference the queue family by an
         * arbitrary index. you get whilst looping through */
        for (int i_queue=0; i_queue < families.size(); ++i_queue)
        {
            VkBool32 present_support = false;
            bool graphics_queue = (families[ i_queue ].queueFlags & VK_QUEUE_GRAPHICS_BIT);
            vkGetPhysicalDeviceSurfaceSupportKHR(
                x_device, i_queue, g_vulkan->surface, &present_support );
            if (graphics_queue && graphics_queue_family < 0)
            {   suitible &= true;
                graphics_queue_family = i_queue;
            }
            else if (present_support && present_queue_family < 0)
            {   suitible &= true;
                present_queue_family = i_queue;
            }
        }
        if (graphics_queue_family < 0 || graphics_queue_family < 0)
        {   VULKAN_ERROR( "Failed to find suitible queue family for device"
                          "Vulkan initialized failed" );
            return false;
        }

        if (global->graphics_llvmpipe)
        {
        }

        // Use llvmpipe exclusively if selected, otherwise prefer dedicated
        // graphics, otherwise use anything
        fstring device_name = props.deviceName;
        // std::for_each( device_name.begin(), device_name.end(), [](char x) { return std::tolower(x); } );
        std::ranges::for_each( device_name, [](char x) { return std::tolower(x); } );
        bool detected_llvmpipe = (device_name.find( "llvmpipe") != std::string::npos);
        bool prefer_dedicated = (global->graphics_llvmpipe == false && dedicated_graphics);
        bool prefer_llvm = (global->graphics_llvmpipe == true && detected_llvmpipe);
        bool more_preferred_device = (prefer_llvm || prefer_dedicated);
        bool select_device = (
            suitible && (more_preferred_device || g_vulkan->device == VK_NULL_HANDLE));
        if (select_device)
        {
            g_vulkan->device = x_device;
            VULKAN_LOG( "Changed primary graphics device to", props.deviceName );
        }
        VULKAN_LOG( "" );
    }
    if (g_vulkan->device == VK_NULL_HANDLE)
    {   VULKAN_ERROR( "Could not find a suitible graphics device for some reason."
                      "Vulkan initialization failed. " );
        return false;
    }

    // -- Create Device and Device Queue --

    // Create some 'queue arg' structs and then pass it to the 'logical device' creation struct
    VkQueue& graphics_queue = g_vulkan->graphics_queue;
    VkQueue& present_queue = g_vulkan->present_queue;
    f32 queue_priority = 1.0f;
    array<VkDeviceQueueCreateInfo> queues;

    VkDeviceQueueCreateInfo graphics_queue_args{};
    graphics_queue_args.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_args.queueFamilyIndex = graphics_queue_family;
    graphics_queue_args.queueCount = 1;
    graphics_queue_args.pQueuePriorities = &queue_priority;
    queues.push_tail( graphics_queue_args );

    VkDeviceQueueCreateInfo present_queue_args{};
    present_queue_args.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    present_queue_args.queueFamilyIndex = present_queue_family;
    present_queue_args.queueCount = 1;
    present_queue_args.pQueuePriorities = &queue_priority;
    queues.push_tail( present_queue_args );

    VkPhysicalDeviceFeatures device_features {
        // for multisampling support
        .sampleRateShading = true,
    };

    // Setup the final logical device args struct
    VkDeviceCreateInfo device_args = {};
    device_args.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_args.pQueueCreateInfos = queues.data;
    device_args.queueCreateInfoCount = queues.size();
    device_args.pEnabledFeatures = &device_features;

    array<cstring> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Presentation swapchain extension
        // Interferes with debugging
        // VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, // Multiple pixels per fragment
    };
    device_args.ppEnabledLayerNames = enabled_layers.data;
    device_args.enabledLayerCount = enabled_layers.size();
    device_args.ppEnabledExtensionNames = device_extensions.data;
    device_args.enabledExtensionCount = device_extensions.size();

    // Actually create logical device
    auto device_ok = vkCreateDevice(
        g_vulkan->device,
        &device_args,
        g_vulkan->vk_allocator,
        &g_vulkan->logical_device
    );
    if (device_ok) { TYON_ERROR( "Device creation error" ); return false; }

    g_vulkan->resources.push_cleanup( []{
        VULKAN_LOGF( "Destroy Logical Device 0x{:x}", u64(g_vulkan->logical_device) );
        vkDestroyDevice( g_vulkan->logical_device, g_vulkan->vk_allocator );
        g_vulkan->logical_device =  VK_NULL_HANDLE;
    } );
    vkGetDeviceQueue( g_vulkan->logical_device, graphics_queue_family, 0, &graphics_queue );
    vkGetDeviceQueue( g_vulkan->logical_device, graphics_queue_family, 0, &present_queue );

    // Create Threading Primitives
    VkFenceCreateInfo fence_args {};
    fence_args.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    bool fence_ok = true;
    fence_ok &= VK_SUCCESS == vkCreateFence(
        g_vulkan->logical_device, &fence_args, g_vulkan->vk_allocator, &g_vulkan->frame_acquire_fence );
    // Signal the begin fence to prevent it hanging
    fence_args.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fence_ok &= VK_SUCCESS == vkCreateFence(
        g_vulkan->logical_device, &fence_args, g_vulkan->vk_allocator, &g_vulkan->frame_begin_fence );
    if (fence_ok == false) { return false; }
    vulkan_label_object(
        (u64)self->frame_begin_fence, VK_OBJECT_TYPE_FENCE, "frame_begin_fence" );
    vulkan_label_object(
        (u64)self->frame_acquire_fence, VK_OBJECT_TYPE_FENCE, "frame_acquire_fence" );

    VkSemaphoreCreateInfo semaphore_args {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    bool semaphore_ok = true;
    semaphore_ok &= VK_SUCCESS == vkCreateSemaphore(
        self->logical_device, &semaphore_args, g_vulkan->vk_allocator,
        &self->frame_end_semaphore );
    semaphore_ok &= VK_SUCCESS == vkCreateSemaphore(
        self->logical_device, &semaphore_args, g_vulkan->vk_allocator,
        &self->queue_submit_semaphore );
    vulkan_label_object(
        (u64)self->frame_end_semaphore, VK_OBJECT_TYPE_SEMAPHORE, "frame_end_semaphore" );
    vulkan_label_object(
        (u64)self->queue_submit_semaphore, VK_OBJECT_TYPE_SEMAPHORE, "queue_submit_semaphore" );

    VkCommandPool& command_pool = g_vulkan->command_pool;
    VkCommandPoolCreateInfo pool_args {};
    pool_args.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_args.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_args.queueFamilyIndex = graphics_queue_family;
    VkResult pool_ok = vkCreateCommandPool(
        g_vulkan->logical_device, &pool_args, g_vulkan->vk_allocator, &command_pool );
    if (pool_ok)
    {   VULKAN_ERROR( "Failed to create command pool" );
        return false;
    }
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying command pool" );
        vkDestroyCommandPool(
            g_vulkan->logical_device, g_vulkan->command_pool, g_vulkan->vk_allocator );
    } );

    // Create a command buffer
    VkCommandBufferAllocateInfo command_args{};
    command_args.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_args.commandPool = command_pool;
    command_args.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_args.commandBufferCount = self->frame_max_inflight;

    self->commands.resize( self->frame_max_inflight );
    VkResult command_buffer_bad = vkAllocateCommandBuffers(
        g_vulkan->logical_device, &command_args, g_vulkan->commands.data );
    if (command_buffer_bad)
    {
        VULKAN_ERROR( "Failed to allocate command buffers" );
        return false;
    }
    g_vulkan->resources.push_cleanup( [] {
        vkFreeCommandBuffers(
            g_vulkan->logical_device, g_vulkan->command_pool,
            g_vulkan->frame_max_inflight, g_vulkan->commands.data
        );
    } );

    // -- Create graphics pipeline --
    // Create shaders of pipeline
    vulkan_shader vertex_shader {};
    vulkan_shader fragment_shader {};

    vertex_shader.name = "test_triangle";
    vertex_shader.code.filename = "shaders/test_triangle.vert.spv";
    vertex_shader.code_binary = true;
    vertex_shader.stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
    fragment_shader.name = "test_triangle";
    fragment_shader.code.filename = "shaders/test_triangle.frag.spv";
    fragment_shader.code_binary = true;
    fragment_shader.stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;

    vulkan_shader_init( &vertex_shader );
    vulkan_shader_init( &fragment_shader );

    vulkan_pipeline pipeline {};
    pipeline.shaders.push_tail( vertex_shader );
    pipeline.shaders.push_tail( fragment_shader );

    array<VkPipelineShaderStageCreateInfo> stages;
    stages.change_allocation( pipeline.shaders.size() );
    for (i64 i=0; i < pipeline.shaders.size(); ++i)
    {
        auto& x_shader = pipeline.shaders[i];
        stages.push_tail( VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = x_shader.stage_flag,
                .module_ = x_shader.platform_module,
                .pName = x_shader.entry_point.c_str()
            });
    }

    VkFormatProperties format_props {};
    vkGetPhysicalDeviceFormatProperties(
        g_vulkan->device, VK_FORMAT_B8G8R8A8_UNORM, &format_props );
    // Mandated by the spec when setting vertex bindings
    bool vertex_buffer_ok = format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
    if (!vertex_buffer_ok)
    { VULKAN_ERROR( "Format feature vertex buffer not supported for specified format" ); }

    // Create vertex buffer for describing a mesh
    VkDeviceMemory& vertex_memory = g_vulkan->vertex_memory;
    VkBufferCreateInfo buffer_args {};
    buffer_args.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_args.size = sizeof(f32) * self->test_triangle_data.size();
    buffer_args.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_args.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto triangle_buffer_bad = vkCreateBuffer(
        self->logical_device, &buffer_args, g_vulkan->vk_allocator, &self->test_triangle_buffer);
    if (triangle_buffer_bad)
    {   VULKAN_ERROR( "Failed to create test triangle buffer" );
        return false;
    }
    vulkan_label_object(
        (u64)self->test_triangle_buffer, VK_OBJECT_TYPE_BUFFER, "test_triangle_buffer" );
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying test triangle buffer" );
        vkDestroyBuffer( g_vulkan->logical_device, g_vulkan->test_triangle_buffer,
            g_vulkan->vk_allocator );
    } );

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements( self->logical_device, self->test_triangle_buffer,
        &memory_requirements );

    VkPhysicalDeviceMemoryProperties memory_props {};
    vkGetPhysicalDeviceMemoryProperties( self->device, &memory_props );

    VkMemoryPropertyFlags memory_filter = (
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    bool match = false;
    i32 memory_type = 0;
    for (i32 i=0; i < memory_props.memoryTypeCount; ++i)
    {
        // TYON_BREAK();
        match = false;
        bool requirement_fulfilled = memory_requirements.memoryTypeBits & (1 << i);
        bool filter_match = (memory_props.memoryTypes[i].propertyFlags & memory_filter);
        match = (requirement_fulfilled && filter_match);
        if (match) { memory_type = i; }
    }
    if (match == false)
    {   VULKAN_ERROR( "Couldn't find suitible memory type for test_triangle_buffer" );
        return false;
    }

    VkMemoryAllocateInfo memory_args {};
    memory_args.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_args.allocationSize = memory_requirements.size;
    memory_args.memoryTypeIndex = memory_type;
    auto memory_bad = vkAllocateMemory(
        self->logical_device, &memory_args, g_vulkan->vk_allocator, &g_vulkan->vertex_memory);
    if (memory_bad)
    {   VULKAN_ERROR( "Failed to allocate memory for triangle buffer" );
        return false;
    }
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying memory allocated for test triangle" );
        vkFreeMemory( g_vulkan->logical_device, g_vulkan->vertex_memory,
            g_vulkan->vk_allocator );
    });
    vkBindBufferMemory( self->logical_device, self->test_triangle_buffer, vertex_memory, 0 );

    void* data;
    vkMapMemory( self->logical_device, vertex_memory, 0, buffer_args.size, 0, &data );
    memcpy( data, self->test_triangle_data.data, size_t(buffer_args.size) );
    // TODO: Pretty sure we can just unmap this immediately right?
    // self->resources.push_cleanup([=] {
    vkUnmapMemory( self->logical_device, vertex_memory );
    // });

    // Flush memory to make sure its used
    VkMappedMemoryRange range {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = vertex_memory,
        .offset = 0,
        .size = buffer_args.size,
    };
    // TODO: remove before flight
    // vkFlushMappedMemoryRanges( self->logical_device, 1, range );

    /* SECTION: Create Render Pass An object that represents a set of
       framebuffer attachments and phases of rendering using those
       attachments. Represented by a VkRenderPass object.

       A render pass describes framebuffers, each pipeline draws to one
       framebuffer, each pipeline could have its own or use a shared
       framebuffer. */

    // Create a render pass, will be passed to pipeline
    VkAttachmentDescription color_attachment {};
    color_attachment.format = self->swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Only need one color attachment?
    array<VkAttachmentReference> color_attachment_refs {
        VkAttachmentReference {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
    };

    // sub-pass first
    VkSubpassDescription sub_pass {};
    sub_pass.pColorAttachments = color_attachment_refs.data;
    sub_pass.colorAttachmentCount = 1;

    VkRenderPass& render_pass = g_vulkan->render_pass;
    VkRenderPassCreateInfo pass_args{};
    pass_args.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    pass_args.attachmentCount = 1;
    pass_args.pAttachments = &color_attachment;
    pass_args.subpassCount = 1;
    pass_args.pSubpasses = &sub_pass;

    auto pass_ok = vkCreateRenderPass(
        g_vulkan->logical_device, &pass_args, g_vulkan->vk_allocator, &render_pass );
    if (pass_ok)
    {   VULKAN_ERROR( "Failed to create render pass" ); return false; }
    VULKAN_LOG( "Created render pass" );
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying render pass" );
        vkDestroyRenderPass( g_vulkan->logical_device, g_vulkan->render_pass,
            g_vulkan->vk_allocator );
    });

    /* Create swapchain before pipeline so we can pass present surface current extent
    But after render pass... because it gets passed in the swapchain */
    g_vulkan->swapchain.name = "version_0";
    vulkan_swapchain_init( &g_vulkan->swapchain, VK_NULL_HANDLE );


    array<VkVertexInputBindingDescription> bindings {
        // {   // Normal binding
        //     .binding = 0, // vertex attribute binding/slot. leave as 0
        //     .stride = 4 * 6, // 3 32-bit vertexes + 3 colours 32-bit
        //     // Not sure what this is. think it means pulling from instance wide stuffs?
        //     .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
        // },
        {   // Vertex Binding
            .binding = 0, // vertex attribute binding/slot. leave as 0
            .stride = 4 * 6, // 3 32-bit vertexes + 3 colours 32-bit
            // Not sure what this is. think it means pulling from instance wide stuffs?
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    array<VkVertexInputAttributeDescription> vertex_attributes {
        {
            .location = 2, // shader specific binding location
            .binding = 0,
            // This uses the color format for some strange reason. This is a 32-bit vec3
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0, // 3 32-bit normals to vertex data
        },
        {
            .location = 1, // shader specific binding location
            .binding = 0,
            // This uses the color format for some strange reason. This is a 32-bit vec3
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 4 * 3, // 3 32-bit normals to vertex data
        }
    };

    // Mesh vertex input args
    VkPipelineVertexInputStateCreateInfo vertex_args {};
    vertex_args.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_args.pVertexBindingDescriptions =  bindings.data;
    vertex_args.vertexBindingDescriptionCount = bindings.size();
    vertex_args.pVertexAttributeDescriptions = vertex_attributes.data;
    vertex_args.vertexAttributeDescriptionCount = vertex_attributes.size();

    // Mesh rendering settings
    VkPipelineInputAssemblyStateCreateInfo input_args {};
    input_args.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_args.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_args.primitiveRestartEnable = VK_FALSE;

    // Rasterizer settings
    VkPipelineRasterizationStateCreateInfo raster_args {};
    raster_args.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_args.polygonMode = VK_POLYGON_MODE_FILL;
    raster_args.lineWidth = 1.0f;
    raster_args.cullMode = VK_CULL_MODE_BACK_BIT;
    raster_args.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_args.depthBiasEnable = VK_FALSE;
    raster_args.depthBiasConstantFactor = 0.0f;         // Optional
    raster_args.depthBiasClamp = 0.0f;                  // Optional
    raster_args.depthBiasSlopeFactor = 0.0f;            // Optional

    VkViewport viewport_config {
        // Upper left coordinates
        .x = 0,
        .y = 0,
        .width = float(self->swapchain.present_size.width),
        .height = float(self->swapchain.present_size.height),
        // Configurable viewport depth, can configurable but usually between 0 and 1
        .minDepth = 0.0,
        .maxDepth = 1.0
    };

    // Only render into a certain portion of the viewport with scissors
    VkRect2D scissor_config {
        VkOffset2D { 0, 0 },
        self->swapchain.present_size
    };

    VkPipelineViewportStateCreateInfo viewport_args {};
    viewport_args.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_args.pViewports = &viewport_config;
    // viewportCount and scissorCount must be the same
    viewport_args.viewportCount = 1;
    viewport_args.pScissors = &scissor_config;
    viewport_args.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisample_args {};
    multisample_args.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_args.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_args.sampleShadingEnable = true;
    multisample_args.minSampleShading = 0.2f;
    // wut is this
    multisample_args.pSampleMask = nullptr;
    multisample_args.alphaToCoverageEnable = false;
    multisample_args.alphaToOneEnable = false;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = (
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo color_blend_args{};
    color_blend_args.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_args.logicOpEnable = VK_FALSE;
    color_blend_args.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blend_args.attachmentCount = 1;
    color_blend_args.pAttachments = &color_blend_attachment;
    color_blend_args.blendConstants[0] = 0.0f; // Optional
    color_blend_args.blendConstants[1] = 0.0f; // Optional
    color_blend_args.blendConstants[2] = 0.0f; // Optional
    color_blend_args.blendConstants[3] = 0.0f; // Optional

    /* NOTE: A pipeline can be set to have some of it's state become dynamic
       after creation.  Which may be a performance benefit for tasks its
       relevant to. This setion describes what state can be dynamic instead of
       static.*/
    array<VkDynamicState> dynamic_states_selected = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_args {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = cast<u32>(dynamic_states_selected.size()),
        .pDynamicStates = dynamic_states_selected.data
    };

    VkPipelineLayout& layout = g_vulkan->pipeline_layout;
    VkDescriptorSetLayout& descriptor_layout = g_vulkan->descriptor_layout;

    // Set pipeline bindings here
    VkDescriptorSetLayoutCreateInfo descriptor_layout_args {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = 0x0,
        .bindingCount = 0,
        .pBindings = nullptr,
    };

    VkPipelineLayoutCreateInfo layout_args {};
    layout_args.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_args.pSetLayouts = &descriptor_layout;
    layout_args.setLayoutCount = 1;
    layout_args.pushConstantRangeCount = 0;
    layout_args.pPushConstantRanges = nullptr;

    auto descriptor_layout_ok = vkCreateDescriptorSetLayout(
        g_vulkan->logical_device, &descriptor_layout_args, g_vulkan->vk_allocator, &descriptor_layout );
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying descriptor layout set" );
        vkDestroyDescriptorSetLayout( g_vulkan->logical_device, g_vulkan->descriptor_layout,
            g_vulkan->vk_allocator );
    });

    auto layout_bad = vkCreatePipelineLayout(
        g_vulkan->logical_device, &layout_args, g_vulkan->vk_allocator, &layout );
    if (layout_bad)
    {   VULKAN_ERROR( "Faled to create pipeline layout" );
        return false;
    }
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying pipeline layout" );
        vkDestroyPipelineLayout( g_vulkan->logical_device, g_vulkan->pipeline_layout,
            g_vulkan->vk_allocator );
    });

    // Pipeline creation args
    VkGraphicsPipelineCreateInfo pipeline_args {};
    pipeline_args.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_args.pStages = stages.data;
    pipeline_args.stageCount = stages.size();
    pipeline_args.pVertexInputState = &vertex_args;
    pipeline_args.pInputAssemblyState = &input_args;
    pipeline_args.pViewportState = &viewport_args;
    pipeline_args.pRasterizationState = &raster_args;
    pipeline_args.pMultisampleState = &multisample_args;
    // pipeline_args.pDepthStencilState = nullptr; // Optional
    pipeline_args.pColorBlendState = &color_blend_args;
    pipeline_args.pDynamicState = &dynamic_state_args;
    pipeline_args.layout = layout;
    pipeline_args.renderPass = render_pass;
    pipeline_args.subpass = 0;
    // pipeline_args.basePipelineHandle = VK_NULL_HANDLE; // Optional
    // pipeline_args.basePipelineIndex = -1; // Optional

    // Provide pipeline cache here if relevant
    auto pipeline_ok = vkCreateGraphicsPipelines(
        g_vulkan->logical_device,
        VK_NULL_HANDLE,
        1,
        &pipeline_args,
        g_vulkan->vk_allocator,
        &g_vulkan->pipeline );
    if (pipeline_ok)
    {   VULKAN_ERROR( "Failed to create graphics pipeline" ); return false; }
    VULKAN_LOG( "Created graphics pipeline" );
    g_vulkan->resources.push_cleanup( [] {
        VULKAN_LOG( "Destroying graphics pipeline" );
        vkDestroyPipeline( g_vulkan->logical_device, g_vulkan->pipeline,
            g_vulkan->vk_allocator );
    });

    result = true;
    g_vulkan->initialized = true;
    return result;
}

PROC vulkan_destroy() -> void
{
    PROFILE_SCOPE_FUNCTION();
    if (g_vulkan == nullptr || g_vulkan->initialized == false) { return; }
    // Wait for device before attempting to cleanup
    auto device_wait_bad = vkDeviceWaitIdle( g_vulkan->logical_device );
    auto present_wait_bad = vkQueueWaitIdle( g_vulkan->present_queue );
    auto graphics_wait_bad = vkQueueWaitIdle( g_vulkan->graphics_queue );

    vulkan_swapchain_destroy( &g_vulkan->swapchain );
    // Might be needed one day, but not right now
    // g_vulkan->resources.run_cleanup();
    g_vulkan->~vulkan_context();
    g_vulkan = nullptr;
}

PROC vulkan_tick() -> void
{
    PROFILE_SCOPE_FUNCTION();

    bool restart_vulkan = false;
    if (restart_vulkan)
    {
        vulkan_destroy();
        vulkan_init();
    }


    vulkan_draw();
}

PROC vulkan_draw() -> void
{
    PROFILE_SCOPE_FUNCTION();
    // -- Function Setup
    auto self = g_vulkan;
    auto& swapchain = self->swapchain;
    if (g_vulkan->initialized == false) { return; }
    i64 current_frame = g_vulkan->frames_started;

    // -- Pre-Draw Start Setup and Reset Tasks
    // Clear acquire fence if we skipped the last frame
    vkResetFences( self->logical_device, 1, &g_vulkan->frame_acquire_fence );

    // Set draw commands
    u32 image_index {};
    auto acquire_bad = vkAcquireNextImageKHR(
        g_vulkan->logical_device,
        g_vulkan->swapchain.platform_swapchain,
        0,
        VK_NULL_HANDLE,
        g_vulkan->frame_acquire_fence,
        &image_index
    );

    if (acquire_bad == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // static time_periodic resize_delay( 16ms );
        // if (resize_delay.triggered() == false)
        // { return; }

        // Reset fence before using again VUID-vkResetFences-pFences-01123
        vkResetFences( self->logical_device, 1, &g_vulkan->frame_acquire_fence );
        // Window probably resized and invalidated the swapchain so we'll try again.
        // Wait for all frames to finish before messing with the swapchain
        vkWaitForFences(
            g_vulkan->logical_device,
            swapchain.frame_end_fences.size(),
            swapchain.frame_end_fences.data,
            true,
            3'000'000'000
        );
        VkSwapchainKHR reuse_swapchain = g_vulkan->swapchain.platform_swapchain;
        vulkan_swapchain_destroy( &g_vulkan->swapchain );
        g_vulkan->swapchain.name = fmt::format( "version_{}", current_frame );
        vulkan_swapchain_init( &g_vulkan->swapchain, reuse_swapchain );

        acquire_bad = vkAcquireNextImageKHR(
            g_vulkan->logical_device,
            g_vulkan->swapchain.platform_swapchain,
            0,
            VK_NULL_HANDLE,
            g_vulkan->frame_acquire_fence,
            &image_index
        );
    }

    // New new frame needs to be rendered yet, do something else
    if (acquire_bad == VK_NOT_READY)
    {   VULKAN_LOG( "No frame ready to begin from vkAcquireNextImageKHR 'VK_NOT_READY'" );
        return;
    }
    else if (acquire_bad)
    {   VULKAN_ERRORF( "Failed to acquire next presentable image '{}'",
                       string_VkResult(acquire_bad) );
        return;
    }

    VkFence frame_end_fence = swapchain.frame_end_fences[ image_index ];
    // Wait on 'frame_acquire_fence' before proceeding to reset the fence
    auto end_timeout = vkWaitForFences(
        g_vulkan->logical_device, 1, &frame_end_fence, true, 1'0000'000'000 );
    vkResetFences( self->logical_device, 1, &frame_end_fence );
    if (end_timeout == VK_TIMEOUT)
    {   VULKAN_ERRORF( "Huge hitch waiting on frame index {}", image_index ); return; }

    /* Wait for frame acquire before proceeding to resetting command buffer This
       sort of halts when the next frame is not completed or blocked so no more
       work can be done */
    auto frame_timeout = vkWaitForFences(
        g_vulkan->logical_device, 1, &self->frame_acquire_fence, true, 16'666'666 );
    if (frame_timeout == VK_TIMEOUT)
    {   VULKAN_ERRORF( "Frame: {}] | Missed frame!", current_frame ); return;
    }
    else if (frame_timeout == VK_ERROR_DEVICE_LOST)
    {   VULKAN_ERROR( "Something really horrible happened, "
                      "device was lost waiting on frame end, 'VK_DEVICE_LOST'" );
        return;
    }
    else if (frame_timeout == VK_SUCCESS)
    {   VULKAN_LOGF( "Frame: {} | Completed Frame.", current_frame ); }

    VkCommandBuffer command_buffer = self->commands[ image_index ];
    vkResetCommandBuffer( command_buffer, 0x0 );

    // -- Get started on new frame --
    ++g_vulkan->frames_started;
    // Start writing draw commands to command buffer
    VkCommandBufferBeginInfo begin_args {};
    begin_args.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_args.flags = 0; // Optional
    begin_args.pInheritanceInfo = nullptr; // Optional
    // Start recording commands into the command buffer

    VkResult command_ok = vkBeginCommandBuffer( command_buffer, &begin_args );

    // -- Start recording into first subpass.--
    // This is started after beginning a command buffer.
    // Set render pass start information
    VkClearValue clear_value {};
    clear_value.color = {{ 0.2f, 0.0f, 0.2f, 1.0f }};
    // VkClearValue clear_values[] = { clear_value, clear_value };
    VkRenderPassBeginInfo render_pass_args{};
    render_pass_args.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_args.renderPass = self->render_pass;
    render_pass_args.framebuffer = self->swapchain_framebuffers[ image_index ];
    render_pass_args.renderArea.offset = {0, 0};
    render_pass_args.renderArea.extent = swapchain.present_size;
    render_pass_args.clearValueCount = 1;
    render_pass_args.pClearValues = &clear_value;
    vkCmdBeginRenderPass( command_buffer, &render_pass_args, VK_SUBPASS_CONTENTS_INLINE );
    // Must bind pipeline before using it
    vkCmdBindPipeline(
        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, self->pipeline );

    // Resize viewport and scissor
    VkViewport viewport_config {
        // Upper left coordinates
        .x = 0,
        .y = 0,
        .width = float(swapchain.present_size.width),
        .height = float(swapchain.present_size.height),
        // Configurable viewport depth, can configurable but usually between 0 and 1
        .minDepth = 0.0,
        .maxDepth = 1.0
    };

    // Only render into a certain portion of the viewport with scissors
    VkRect2D scissor_config {
        VkOffset2D { 0, 0 },
        swapchain.present_size
    };
    vkCmdSetViewport( command_buffer, 0, 1, &viewport_config );
    vkCmdSetScissor( command_buffer, 0, 1, &scissor_config );

    // Bind vertex data to pipeline data slots
    VkBuffer vertex_buffers[] = { self->test_triangle_buffer };
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers( command_buffer, 0, 1, vertex_buffers, offsets );
    // Draw an actual mesh
    vkCmdDraw( command_buffer, 3, 1, 0, 0 );

    VkPipelineStageFlags wait_stages[] { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    // Finalize frame and submit all commands
    VkSubmitInfo submit_args {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        // No semaphores to wait on yet
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = wait_stages,
        // Just the one command buffer for now
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &self->queue_submit_semaphore
    };

    // VkResult sync_ok = vkWaitForFences(
    //     self->logical_device,
    //     1,
    //     &self->frame_begin_fence,
    //     true,
    //     16'666'666
    // );
    // vkResetFences( self->logical_device, 1, &self->frame_begin_fence );
    vkCmdEndRenderPass( command_buffer );
    vkEndCommandBuffer( command_buffer );

    // if (sync_ok != VK_SUCCESS)
    // { TYON_ERROR( "Failed to wait on frame start fence for some reason" ); }
    vkQueueSubmit( self->graphics_queue, 1, &submit_args, frame_end_fence );

    VkSwapchainKHR present_swapchains[] = { self->swapchain.platform_swapchain };
    VkPresentInfoKHR present_args {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &self->queue_submit_semaphore,
        .swapchainCount = 1,
        .pSwapchains = present_swapchains,
        .pImageIndices = &image_index,
        // This can be used for storing results from each individual swapchain
        .pResults = nullptr
    };
    VkResult present_bad = vkQueuePresentKHR( self->present_queue, &present_args );
    if (present_bad)
    {   VULKAN_ERRORF( "Fatal error '{}' with drawing and presentation 'VkQueuePresent'",
                       string_VkResult(present_bad) );
    }

    // TODO: Remove because useless now?
    // auto frame_timeout = vkWaitForFences(
    // self->logical_device, 1, &frame_end_fence, true, 1'000'000'000 );
}
