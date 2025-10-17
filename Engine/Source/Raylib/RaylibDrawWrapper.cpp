#include "RaylibDrawWrapper.h"
#include "ThirdParty/Raylib/include/raylib.h"

void DrawRectangleWrapper(float posX, float posY, float sizeX, float sizeY, float rotation, unsigned char colorR, unsigned char colorG, unsigned char colorB, unsigned char colorA)
{
    DrawRectanglePro(
        {
            posX,
            posY,
            sizeX,
            sizeY
        },
        {
            sizeX / 2,
            sizeY / 2
         },
        rotation,
        { colorR, colorG, colorB, colorA });
}

void DrawCircleWrapper(float posX, float posY, float radius, unsigned char colorR, unsigned char colorG, unsigned char colorB, unsigned char colorA)
{
    DrawCircleV({ posX, posY }, radius, { colorR, colorG, colorB, colorA });
}

//void DrawTextureWrapper(float posX, float posY, float sizeX, float sizeY, float rotation, unsigned char colorR, unsigned char colorG, unsigned char colorB, unsigned char colorA) // Moved to RaylibWrapper
//{
//}