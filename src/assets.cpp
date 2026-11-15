// The installed raylib build omits JPEG. Use its bundled decoder privately.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include "external/stb_image.h"
#include "game.hpp"
namespace mission {
Texture2D LoadPhoto(const char* path){
    int width=0,height=0,channels=0;
    unsigned char* pixels=stbi_load(path,&width,&height,&channels,4);
    if(!pixels)return {};
    Image image{pixels,width,height,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture=LoadTextureFromImage(image);
    stbi_image_free(pixels);
    if(texture.id)SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
    return texture;
}
}