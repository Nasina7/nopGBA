#include "input.hpp"
uint32_t screenTexture[160][240];
uint32_t palTexture[0x1E * 2][0xF * 2];
uint32_t tiles1Texture[0x3E0][0x3E0];
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
            //screenTexture[yPos][xPos] = 0xFF << 24 |
                                        //readablePalRam[2][(currentByteRender & 0xF0) >> 4][currentByteRender & 0xF] << 16 |
                                        //readablePalRam[1][(currentByteRender & 0xF0) >> 4][currentByteRender & 0xF] << 8 |
                                        //readablePalRam[0][(currentByteRender & 0xF0) >> 4][currentByteRender & 0xF];
            /*
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
            */
            if(currentByteRender == 0)
            {
                //SDL_SetRenderDrawColor(mainRenderer,0x00,0x00,0x00,0x00);
                screenTexture[yPos][xPos] = 0xFF000000;
            }
            if(currentByteRender != 0)
            {
                //SDL_SetRenderDrawColor(mainRenderer,0xFF,0xFF,0xFF,0x00);
                screenTexture[yPos][xPos] = 0xFFFFFFFF;
            }
            screenTexture[yPos][xPos] = 0xFF << 24 |
                                        readablePalRam[2][(currentByteRender & 0xF0) >> 4][currentByteRender & 0xF] << 16 |
                                        readablePalRam[1][(currentByteRender & 0xF0) >> 4][currentByteRender & 0xF] << 8 |
                                        readablePalRam[0][(currentByteRender & 0xF0) >> 4][currentByteRender & 0xF];
            //screenTexture[yPos][xPos] = finalColor.to_ulong();
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
uint8_t xPosT;
uint8_t yPosT;
uint8_t xPosT2;
uint8_t yPosT2;
uint32_t xTile;
uint32_t yTile;
uint8_t bg0Screen;
uint8_t bg0Char;
uint8_t flipFix[8] = {7,6,5,4,3,2,1,0};
int basicMode0Render()
{
    bg0Screen = (gbaREG.bgCNT[0] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[0] >> 2) & 0x3;
    //breakpoint = true;
    printf("bg0S: 0x%X\n",bg0Screen);
    printf("bg0C: 0x%X\n",bg0Char);
    uint32_t tileLocate;
    uint32_t tileData;
    uint16_t charIndex;
    uint16_t charData;
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    uint8_t currentByteRender;
    tileData = readMem(2,tileLocate);
    while(yTile != 0x20)
    {
        while(xTile != 0x20)
        {
            while(yPosT != 8)
            {
                while(xPosT != 8)
                {
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                    if(xPosT != 0x7)
                    {
                        currentByteRender = tileData >> (28 - ((xPosT) * 4));
                    }
                    if(xPosT == 0x7)
                    {
                        currentByteRender = tileData;
                    }
                    if(swapPal == false)
                    {
                        screenBG0Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }
                    if(swapPal == true)
                    {
                        uint8_t loopRead = 0;
                        while (loopRead != 0x10)
                        {
                            readablePalRam[0][0][loopRead] = loopRead * 0x10;
                            readablePalRam[1][0][loopRead] = loopRead * 0x10;
                            readablePalRam[2][0][loopRead] = loopRead * 0x10;
                            loopRead++;
                        }

                        screenBG0Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }

                    xPosT++;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                yPosT++;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            xTile++;
            xPosT2 = xTile * 8;
            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
            charIndex = charData & 0x1FF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            //printf("tileLocate: 0x%X\n",tileLocate);
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        yTile++;
        yPosT = 0;
        yPosT2 = (yTile * 8);
    }
    xPosT = 0;
    yPosT = 0;



    bg0Screen = (gbaREG.bgCNT[1] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[1] >> 2) & 0x3;
    //breakpoint = true;
    printf("bg1S: 0x%X\n",bg0Screen);
    printf("bg1C: 0x%X\n",bg0Char);
    tileLocate = 0;
    tileData = 0;
    charIndex = 0;
    charData = 0;
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    currentByteRender = 0;
    tileData = readMem(2,tileLocate);
    while(yTile != 0x20)
    {
        while(xTile != 0x20)
        {
            while(yPosT != 8)
            {
                while(xPosT != 8)
                {
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                    if(xPosT != 0x7)
                    {
                        currentByteRender = tileData >> (28 - ((xPosT) * 4));
                    }
                    if(xPosT == 0x7)
                    {
                        currentByteRender = tileData;
                    }
                    if(swapPal == false)
                    {
                        screenBG1Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }
                    if(swapPal == true)
                    {
                        uint8_t loopRead = 0;
                        while (loopRead != 0x10)
                        {
                            readablePalRam[0][0][loopRead] = loopRead * 0x10;
                            readablePalRam[1][0][loopRead] = loopRead * 0x10;
                            readablePalRam[2][0][loopRead] = loopRead * 0x10;
                            loopRead++;
                        }

                        screenBG1Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }

                    xPosT++;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                yPosT++;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            xTile++;
            xPosT2 = xTile * 8;
            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
            charIndex = charData & 0x1FF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            //printf("tileLocate: 0x%X\n",tileLocate);
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        yTile++;
        yPosT = 0;
        yPosT2 = (yTile * 8);
    }
    xPosT = 0;
    yPosT = 0;




    bg0Screen = (gbaREG.bgCNT[2] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[2] >> 2) & 0x3;
    //breakpoint = true;
    printf("bg2S: 0x%X\n",bg0Screen);
    printf("bg2C: 0x%X\n",bg0Char);
    tileLocate = 0;
    tileData = 0;
    charIndex = 0;
    charData = 0;
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    currentByteRender = 0;
    tileData = readMem(2,tileLocate);
    while(yTile != 0x20)
    {
        while(xTile != 0x20)
        {
            while(yPosT != 8)
            {
                while(xPosT != 8)
                {
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                    if(xPosT != 0x7)
                    {
                        currentByteRender = tileData >> (28 - ((xPosT) * 4));
                    }
                    if(xPosT == 0x7)
                    {
                        currentByteRender = tileData;
                    }
                    if(swapPal == false)
                    {
                        screenBG2Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }
                    if(swapPal == true)
                    {
                        uint8_t loopRead = 0;
                        while (loopRead != 0x10)
                        {
                            readablePalRam[0][0][loopRead] = loopRead * 0x10;
                            readablePalRam[1][0][loopRead] = loopRead * 0x10;
                            readablePalRam[2][0][loopRead] = loopRead * 0x10;
                            loopRead++;
                        }

                        screenBG2Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }

                    xPosT++;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                yPosT++;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            xTile++;
            xPosT2 = xTile * 8;
            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
            charIndex = charData & 0x1FF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            //printf("tileLocate: 0x%X\n",tileLocate);
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        yTile++;
        yPosT = 0;
        yPosT2 = (yTile * 8);
    }
    xPosT = 0;
    yPosT = 0;
}
void basicTiles1Render()
{
    uint32_t tileLocate = 0x06000000;
    uint32_t tileData;
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    uint8_t currentByteRender;
    tileData = readMem(2,tileLocate);
    while(yTile != 0x20)
    {
        while(xTile != 0x20)
        {
            while(yPosT != 8)
            {
                while(xPosT != 8)
                {
                    if(xPosT != 0x7)
                    {
                        currentByteRender = tileData >> (28 - ((xPosT) * 4));
                    }
                    if(xPosT == 0x7)
                    {
                        currentByteRender = tileData;
                    }
                    if(swapPal == false)
                    {
                        tiles1Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }
                    if(swapPal == true)
                    {
                        uint8_t loopRead = 0;
                        while (loopRead != 0x10)
                        {
                            readablePalRam[0][0][loopRead] = loopRead * 0x10;
                            readablePalRam[1][0][loopRead] = loopRead * 0x10;
                            readablePalRam[2][0][loopRead] = loopRead * 0x10;
                            loopRead++;
                        }

                        tiles1Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF0) >> 4] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF0) >> 4] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF0) >> 4];
                    }

                    xPosT++;
                    xPosT2 = xPosT + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                yPosT++;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            xTile++;
            xPosT2 = xTile * 8;
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        yTile++;
        yPosT = 0;
        yPosT2 = (yTile * 8);
    }
    xPosT = 0;
    yPosT = 0;

}
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

void debugPalWindow()
{
    xPos = 0;
    yPos = 0;
    while(yPos != 0x10)
    {
        while(xPos != 0x10)
        {
            palTexture[yPos][xPos] = 0xFF << 24 | readablePalRam[0][yPos][xPos] << 16 | readablePalRam[1][yPos][xPos] << 8 | readablePalRam[2][yPos][xPos];
            palTexture[yPos][xPos + 1] = 0xFF << 24 | readablePalRam[0][yPos][xPos] << 16 | readablePalRam[1][yPos][xPos] << 8 | readablePalRam[2][yPos][xPos];
            palTexture[yPos + 1][xPos] = 0xFF << 24 | readablePalRam[0][yPos][xPos] << 16 | readablePalRam[1][yPos][xPos] << 8 | readablePalRam[2][yPos][xPos];
            palTexture[yPos + 1][xPos + 1] = 0xFF << 24 | readablePalRam[0][yPos][xPos] << 16 | readablePalRam[1][yPos][xPos] << 8 | readablePalRam[2][yPos][xPos];

            //palTexture[yPos][xPos] = 0xFFFFFFFF;
            xPos += 2;
        }
        xPos = 0;
        yPos += 2;
    }
    xPos = 0;
    yPos = 0;
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

        case 0:
            basicMode0Render();
        break;

        case 4:
            basicRenderMode4();
        break;

        default:
            printf("Unimplemented Render Mode 0x%X!\nDefaulting to Mode 4!\n");
            basicRenderMode4();
        break;
    }
    if(displayDisplayView == true)
    {
        debugPalWindow();
        glBindTexture(GL_TEXTURE_2D, palTexGL);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,0x1E,0x1E * 2,0,GL_RGBA,GL_UNSIGNED_BYTE,*palTexture);
        glGenerateMipmap(GL_TEXTURE_2D);
        //glBindTexture(GL_TEXTURE_2D, 2);
    }
    if(displayTiles1 == true)
    {
        basicTiles1Render();
        glBindTexture(GL_TEXTURE_2D, tiles1TexGL);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,0x3E0,0x3E0,0,GL_RGBA,GL_UNSIGNED_BYTE,*tiles1Texture);
        glGenerateMipmap(GL_TEXTURE_2D);
        //glBindTexture(GL_TEXTURE_2D, 1);
    }

    glBindTexture(GL_TEXTURE_2D, screenTexGL);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,240,160,0,GL_RGBA,GL_UNSIGNED_BYTE,*screenTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, 0);
    handleDebugWindow();
}
