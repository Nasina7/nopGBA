#include "input.hpp"
uint32_t screenTexture[160][240];
uint16_t colorWrite;
std::bitset<16> colorWriteInvert;
std::bitset<32> finalColor = 0xFF000000;
uint8_t currentDispMode;
std::bitset<3> currentDispMode2;
int xPos = 0;
int yPos = 0;
void basicRenderMode4()
{
    xPos = 0;
    yPos = 0;
    uint8_t currentByteRender;
    while(yPos != 160)
    {
        while(xPos != 240)
        {
            currentByteRender = gbaRAM.VideoRAM[(yPos * 240) + xPos];
            colorWriteInvert = gbaRAM.Bg_ObjPalRam[(currentByteRender * 2)] << 8 | gbaRAM.Bg_ObjPalRam[(currentByteRender * 2) + 1];
            //colorWriteInvert.flip();
            finalColor[23] = colorWriteInvert[14];
            finalColor[22] = colorWriteInvert[13];
            finalColor[21] = colorWriteInvert[12];
            finalColor[20] = colorWriteInvert[11];
            finalColor[19] = colorWriteInvert[10];
            finalColor[15] = colorWriteInvert[9];
            finalColor[14] = colorWriteInvert[8];
            finalColor[13] = colorWriteInvert[7];
            finalColor[12] = colorWriteInvert[6];
            finalColor[11] = colorWriteInvert[5];
            finalColor[7] = colorWriteInvert[4];
            finalColor[6] = colorWriteInvert[3];
            finalColor[5] = colorWriteInvert[2];
            finalColor[4] = colorWriteInvert[1];
            finalColor[3] = colorWriteInvert[0];
            if(currentByteRender == 0)
            {
                //SDL_SetRenderDrawColor(mainRenderer,0x00,0x00,0x00,0x00);
                screenTexture[yPos][xPos] = 0x00000000;
            }
            if(currentByteRender != 0)
            {
                //SDL_SetRenderDrawColor(mainRenderer,0xFF,0xFF,0xFF,0x00);
                screenTexture[yPos][xPos] = 0xFFFFFFFF;
            }
            //SDL_RenderDrawPoint(mainRenderer,xPos,yPos);
            xPos++;
        }
        xPos = 0;
        yPos++;
    }
    xPos = 0;
    yPos = 0;
    //SDL_RenderPresent(mainRenderer);
}
uint16_t currentByteRender2;
uint8_t tileCordY;
uint8_t tileCordX;
uint32_t currentTileX;
uint32_t currentTileY;
void basicRenderMode1()
{
    currentTileX = 0;
    currentTileY = 0;
    xPos = 0;
    yPos = 0;
    tileCordX = 0;
    tileCordY = 0;
    while(currentTileY != 0x14)
    {
        while(currentTileX != 0x1E)
        {
            currentByteRender2 = gbaRAM.VideoRAM[0x06005000 + (currentTileX * 2) + (currentTileY * 0x40)] << 8 |
                                 gbaRAM.VideoRAM[(0x06005000 + 1) + (currentTileX * 2) + (currentTileY * 0x40)];
            tileCordY = 0;
            tileCordX = 0;
            while(tileCordY != 8)
            {
                while(tileCordX != 8)
                {
                    screenTexture[yPos + tileCordY][xPos + tileCordX] = 0xFF << 24 | currentByteRender2 << 8 | 0x00;
                    tileCordX += 1;
                }
                tileCordX = 0;
                tileCordY += 1;
            }
            xPos += 8;
            tileCordX = 0;
            tileCordY = 0;
            currentTileX += 1;
        }
        yPos += 8;
        currentTileX = 0;
        currentTileY += 1;
    }
    //printf("currTileY: 0x%X\n\n\n\n\n",currentTileY);
    yPos = 0;
    xPos = 0;
    tileCordX = 0;
    tileCordY = 0;
    currentTileX = 0;
    currentTileY = 0;
}
void handleRendering()
{
    currentDispMode2 = gbaREG.lcdControl;
    currentDispMode = currentDispMode2.to_ulong();
    switch(currentDispMode)
    {
        /*
        case 1:
            basicRenderMode1();
            //printf("MODE 1\n\n\n\n\n\n\n\n\n\n\n");
        break;
        */

        case 4:
            basicRenderMode4();
        break;

        default:
            printf("Unimplemented Render Mode 0x%X!\nDefaulting to Mode 4!\n");
            //basicRenderMode4();
        break;
    }

    glBindTexture(GL_TEXTURE_2D, screenTexGL);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,240,160,0,GL_RGBA,GL_UNSIGNED_BYTE,*screenTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    handleDebugWindow();
}
