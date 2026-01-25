#define TINYGLTF_IMPLEMENTATION

#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_RAPIDJSON
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE

#include <stb_image.h>
#include <stb_image_write.h>
#include <nlohmann/json.hpp>

#include <tiny_gltf.h>
