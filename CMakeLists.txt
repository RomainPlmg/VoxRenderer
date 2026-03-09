cmake_minimum_required(VERSION 3.25)
project(svo_demo)

# ============================================================================
# C++ Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Define executable
# ============================================================================
file(GLOB_RECURSE SVO_SRC src/*.cpp)
add_executable(svo_demo ${SVO_SRC})
target_include_directories(svo_demo PRIVATE include)

target_compile_definitions(svo_demo PRIVATE
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/"
)

# ============================================================================
# Dependencies
# ============================================================================
find_package(Vulkan REQUIRED)

include(FetchContent)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
        SYSTEM
)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
        SYSTEM
)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE
        SYSTEM
)
FetchContent_Declare(
        vk_bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
        GIT_TAG v1.4.344
        GIT_SHALLOW TRUE
        SYSTEM
)
FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.3.0
        GIT_SHALLOW TRUE
        SYSTEM
)

FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(vk_bootstrap)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

target_link_libraries(svo_demo PRIVATE
        spdlog::spdlog
        glfw
        glm::glm-header-only
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
        Vulkan::Vulkan
)

# ============================================================================
# Compiler options
# ============================================================================
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wno-missing-field-initializers # Due tu Vulkan structs, too severe
)