#pragma once

#include "VkContext.hpp"

struct WindowResizeEvent {
    int width;
    int height;
};

struct StorageImageRecreatedEvent {
    VkImageView imageView = VK_NULL_HANDLE;
};
