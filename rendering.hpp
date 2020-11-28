#include "input.hpp"
uint32_t screenTexture[180][240];
uint32_t palTexture[0x1E * 2][0xF * 2];
uint32_t tiles1Texture[0x3E0][0x3E0];

uint16_t colorWrite;
//std::bitset<16> colorWriteInvert;
//std::bitset<32> finalColor = 0xFF000000;
//std::bitset<3> currentDispMode2;
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

uint32_t spriteLocate;

uint8_t currentSpriteY;
bool currentSpriteRotate;
bool spriteBit9;
uint8_t currentSpriteMode;
bool currentSpriteMos;
bool currentSpriteColors;
uint8_t currentSpriteShape;

uint16_t currentSpriteX;
uint8_t rotateScaleParam;
bool currSpriteHFlip;
bool currSpriteVFlip;
uint8_t currSpriteSize;

uint16_t currSpriteTileNum;
uint8_t currSpritePriority;
uint8_t currSpritePalNum;

uint16_t getCurrSpriteData;
void getSpriteInfo(uint8_t spriteNumber)
{
    spriteLocate = 0x07000000 + (spriteNumber * 8);
    getCurrSpriteData = readMem(1, spriteLocate); // Gets OBJ att 0

    currentSpriteY = getCurrSpriteData & 0xFF;
    currentSpriteRotate = (getCurrSpriteData >> 8) & 0x1;
    spriteBit9 = (getCurrSpriteData >> 9) & 0x1;
    currentSpriteMode = (getCurrSpriteData >> 10) & 0x3;
    currentSpriteMos = (getCurrSpriteData >> 12) & 0x1;
    currentSpriteColors = (getCurrSpriteData >> 13) & 0x1;
    currentSpriteShape = (getCurrSpriteData >> 14) & 0x3;

    getCurrSpriteData = readMem(1, spriteLocate + 2); // Gets OBJ att 1

    currentSpriteX = getCurrSpriteData & 0x1FF;
    if(currentSpriteRotate == 1)
    {
        rotateScaleParam = (getCurrSpriteData >> 9) & 0x1F;
    }
    if(currentSpriteRotate == 0)
    {
        currSpriteHFlip = (getCurrSpriteData >> 12) & 0x1;
        currSpriteVFlip = (getCurrSpriteData >> 13) & 0x1;
    }
    currSpriteSize = (getCurrSpriteData >> 14) & 0x3;

    getCurrSpriteData = readMem(1, spriteLocate + 4); // Gets OBJ att 1

    currSpriteTileNum = getCurrSpriteData & 0x3FF;
    currSpritePriority = (getCurrSpriteData >> 10) & 0x3;
    currSpritePalNum = (getCurrSpriteData >> 12) & 0xF;
}
uint8_t X88;
uint8_t X882;
uint8_t Y88;
int render8by8Sprite()
{
    //printf("SpriteX: 0x%X\n", currentSpriteX);
    //printf("SpriteY: 0x%X\n", currentSpriteY);
    //breakpoint = true;

    uint32_t tileData;
    uint8_t currentByteRender;
    if((0x06010000 + (currSpriteTileNum * 0x20)) > 0x06017FFF)
    {
        //printf("SPRITE DRAWING ROUTINE ATTEMPTED TO ACCESS OUT OF BOUNDS AREA OF OBJ RAM!\n");
        return 0;
    }
    tileData = readMem(2, 0x06010000 + (currSpriteTileNum * 0x20));
    //printf("DataLocate: 0x%X\n", 0x06010000 + (currSpriteTileNum * 0x20));
    //charIndex = charData & 0x1FF;
    //charPal = (charData >> 12) & 0xF;
    //tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    //tileData = readMem(2,tileLocate);

    X88 = 0;
    Y88 = 0;
    while(Y88 != 8)
    {
        while(X88 != 8)
        {
            X882 = flipFix[X88];
            if(X88 != 0x7)
            {
                currentByteRender = tileData >> (28 - ((X88) * 4));
            }
            if(X88 == 0x7)
            {
                currentByteRender = tileData;
            }

            //screenTexture[currentSpriteY + Y88][currentSpriteX + X88] = 0xFFFF0000;
            if((currentByteRender & 0xF) != 0 && (currentSpriteY + Y88) < 180 && (currentSpriteX + X882) < 240)
            {
                screenTexture[currentSpriteY + Y88][currentSpriteX + X882] = 0xFF << 24 |
                                                readablePalRam[2][0x10 + currSpritePalNum][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0x10 + currSpritePalNum][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0x10 + currSpritePalNum][(currentByteRender & 0xF)];
            }
            X88++;
        }
        X88 = 0;
        Y88++;
        if((0x06010000 + (currSpriteTileNum * 0x20)) + (Y88 * 4) > 0x06017FFF)
        {
            //printf("SPRITE DRAWING ROUTINE ATTEMPTED TO ACCESS OUT OF BOUNDS AREA OF OBJ RAM!\n");
            return 0;
        }
        tileData = readMem(2, (0x06010000 + (currSpriteTileNum * 0x20)) + (Y88 * 4) );
    }
    return 0;
}
int render8by8SpriteHFLIP()
{
    //printf("SpriteX: 0x%X\n", currentSpriteX);
    //printf("SpriteY: 0x%X\n", currentSpriteY);
    //breakpoint = true;

    uint32_t tileData;
    uint8_t currentByteRender;
    if((0x06010000 + (currSpriteTileNum * 0x20)) > 0x06017FFF)
    {
        //printf("SPRITE DRAWING ROUTINE ATTEMPTED TO ACCESS OUT OF BOUNDS AREA OF OBJ RAM!\n");
        return 0;
    }
    tileData = readMem(2, 0x06010000 + (currSpriteTileNum * 0x20));
    //printf("DataLocate: 0x%X\n", 0x06010000 + (currSpriteTileNum * 0x20));
    //charIndex = charData & 0x1FF;
    //charPal = (charData >> 12) & 0xF;
    //tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    //tileData = readMem(2,tileLocate);

    X88 = 0;
    Y88 = 0;
    while(Y88 != 8)
    {
        while(X88 != 8)
        {
            X882 = X88;
            if(X88 != 0x7)
            {
                currentByteRender = tileData >> (28 - ((X88) * 4));
            }
            if(X88 == 0x7)
            {
                currentByteRender = tileData;
            }

            //screenTexture[currentSpriteY + Y88][currentSpriteX + X88] = 0xFFFF0000;
            if((currentByteRender & 0xF) != 0 && (currentSpriteY + Y88) < 180 && (currentSpriteX + X882) < 240)
            {
                screenTexture[currentSpriteY + Y88][currentSpriteX + X882] = 0xFF << 24 |
                                                readablePalRam[2][0x10 + currSpritePalNum][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0x10 + currSpritePalNum][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0x10 + currSpritePalNum][(currentByteRender & 0xF)];
            }
            X88++;
        }
        X88 = 0;
        Y88++;
        if((0x06010000 + (currSpriteTileNum * 0x20)) + (Y88 * 4) > 0x06017FFF)
        {
            //printf("SPRITE DRAWING ROUTINE ATTEMPTED TO ACCESS OUT OF BOUNDS AREA OF OBJ RAM!\n");
            return 0;
        }
        tileData = readMem(2, (0x06010000 + (currSpriteTileNum * 0x20)) + (Y88 * 4) );
    }
    return 0;
}
uint8_t renderSizeX;
uint8_t renderSizeY;
uint8_t currentSprite;
void renderSpriteNormal()
{
    switch(currSpriteSize)
    {
        case 0:
            switch(currentSpriteShape)
            {
                case 0: // 8x8
                    render8by8Sprite();
                break;

                case 1: // 16x8
                    render8by8Sprite();
                    currSpriteTileNum++;
                    currentSpriteX += 8;
                    render8by8Sprite();
                break;

                case 2: // 8x16
                    render8by8Sprite();
                    currSpriteTileNum += 0x20;
                    currentSpriteY += 8;
                    render8by8Sprite();
                break;
            }
        break;

        case 1:
            switch(currentSpriteShape)
            {
                case 0: // 16x16
                    renderSizeX = 0;
                    renderSizeY = 0;
                    while(renderSizeY != 2)
                    {
                        while(renderSizeX != 2)
                        {
                            render8by8Sprite();

                            renderSizeX++;
                            currSpriteTileNum++;
                            currentSpriteX += 8;
                        }
                        renderSizeX = 0;
                        currSpriteTileNum -= 2;
                        currentSpriteX -= 16;

                        renderSizeY++;
                        currSpriteTileNum += 0x20;
                        currentSpriteY += 8;
                    }
                    //render8by8Sprite();
                break;

                case 1: // 32x8
                    renderSizeX = 0;
                    while(renderSizeX != 4)
                    {
                        render8by8Sprite();

                        renderSizeX++;
                        currSpriteTileNum++;
                        currentSpriteX += 8;
                    }
                break;

                case 2: // 8x32
                    renderSizeY = 0;
                    while(renderSizeY != 4)
                    {
                        render8by8Sprite();

                        renderSizeY++;
                        currSpriteTileNum += 0x20;
                        currentSpriteY += 8;
                    }
                break;
            }
        break;

        case 2:
            switch(currentSpriteShape)
            {
                case 0: // 32x32
                    renderSizeX = 0;
                    renderSizeY = 0;
                    while(renderSizeY != 4)
                    {
                        while(renderSizeX != 4)
                        {
                            render8by8Sprite();

                            renderSizeX++;
                            currSpriteTileNum++;
                            currentSpriteX += 8;
                        }
                        renderSizeX = 0;
                        currSpriteTileNum -= 4;
                        currentSpriteX -= 32;

                        renderSizeY++;
                        currSpriteTileNum += 0x20;
                        currentSpriteY += 8;
                    }
                    //render8by8Sprite();
                break;

                default:
                    printf("spriteNOtIMp\n");
                break;
            }
        break;

        default:

        break;
    }
}

void renderSpriteHFlip()
{
    switch(currSpriteSize)
    {
        case 0:
            switch(currentSpriteShape)
            {
                case 0: // 8x8
                    render8by8SpriteHFLIP();
                break;

                case 1: // 16x8
                    currSpriteTileNum++;
                    //currentSpriteX += 8;
                    render8by8SpriteHFLIP();
                    currSpriteTileNum--;
                    currentSpriteX += 8;
                    render8by8SpriteHFLIP();
                break;

                case 2: // 8x16
                    render8by8SpriteHFLIP();
                    currSpriteTileNum += 0x20;
                    currentSpriteY += 8;
                    render8by8SpriteHFLIP();
                break;
            }
        break;

        case 1:
            switch(currentSpriteShape)
            {
                case 0: // 16x16
                    renderSizeX = 0;
                    renderSizeY = 0;

                    currSpriteTileNum++;
                    //currentSpriteX += 8;

                    while(renderSizeY != 2)
                    {
                        while(renderSizeX != 2)
                        {
                            render8by8SpriteHFLIP();

                            renderSizeX++;
                            currSpriteTileNum--;
                            currentSpriteX += 8;
                        }
                        renderSizeX = 0;
                        currSpriteTileNum += 2;
                        currentSpriteX -= 16;

                        renderSizeY++;
                        currSpriteTileNum += 0x20;
                        currentSpriteY += 8;
                    }
                break;

                case 1: // 32x8
                    printf("spriteNOtIMp\n");
                break;

                case 2: // 8x32
                    printf("spriteNOtIMp\n");
                break;
            }
        break;

        case 2:
            switch(currentSpriteShape)
            {
                case 0: // 32x32
                    printf("spriteNOtIMp\n");
                    //render8by8Sprite();
                break;

                default:
                    printf("spriteNOtIMp\n");
                break;
            }
        break;

        default:

        break;
    }
}
void renderSprites()
{
    currentSprite = 127;
    while(currentSprite != 0xFF)
    {
        getSpriteInfo(currentSprite);
        switch(currSpriteHFlip << 1 | currSpriteVFlip)
        {
            case 0x0: // No Flipping
                renderSpriteNormal();
            break;

            case 0x2: // Horizontal Flipping
                renderSpriteHFlip();
            break;
        }

    currentSprite--;
    }

}
int basicMode0Render()
{
    bg0Screen = (gbaREG.bgCNT[0] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[0] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg0S: 0x%X\n",bg0Screen);
    //printf("bg0C: 0x%X\n",bg0Char);
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
    while(yTile != 0x1F)
    {
        while(xTile != 0x1F)
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
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
                    }

                    ++xPosT;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                ++yPosT;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            ++xTile;
            xPosT2 = xTile * 8;
            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
            charIndex = charData & 0x1FF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            //printf("tileLocate: 0x%X\n",tileLocate);
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        ++yTile;
        yPosT = 0;
        yPosT2 = (yTile * 8);
    }
    xPosT = 0;
    yPosT = 0;

    bg0Screen = (gbaREG.bgCNT[1] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[1] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg1S: 0x%X\n",bg0Screen);
    //printf("bg1C: 0x%X\n",bg0Char);
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
    while(yTile != 0x1F)
    {
        while(xTile != 0x1F)
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
                        currentByteRender = tileData & 0xF;
                    }
                    if(swapPal == false)
                    {
                        screenBG1Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
                    }

                    xPosT++;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                yPosT++;
                yPosT2 = (yTile * 8) + yPosT;
                //printf("tileLocate: 0x%X\n",tileLocate);
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
    //printf("bg2S: 0x%X\n",bg0Screen);
    //printf("bg2C: 0x%X\n",bg0Char);
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
    while(yTile != 0x1F)
    {
        while(xTile != 0x1F)
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
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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



    bg0Screen = (gbaREG.bgCNT[3] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[3] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg3S: 0x%X\n",bg0Screen);
    //printf("bg3C: 0x%X\n",bg0Char);
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
    while(yTile != 0x1F)
    {
        while(xTile != 0x1F)
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
                        screenBG3Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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

                        screenBG3Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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

    return 0;
}

uint8_t bgSize;
int renderBGtoTexMode0(uint8_t background)
{

    bg0Screen = (gbaREG.bgCNT[background] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[background] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg0S: 0x%X\n",bg0Screen);
    //printf("bg0C: 0x%X\n",bg0Char);
    uint32_t tileLocate;
    uint32_t tileData;
    uint16_t charIndex;
    uint16_t charData;
    uint8_t charPal;
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    charPal = (charData >> 12) & 0xF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    uint8_t currentByteRender;
    tileData = readMem(2,tileLocate);
    while(yTile != 0x20) // 0x14
    {
        while(xTile != 0x20) // 0x1F
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

                        bgTex[background][yPosT2][xPosT2] = 0xFF << 24 | readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 | readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 | readablePalRam[0][charPal][(currentByteRender & 0xF)];
                        if((currentByteRender & 0xF) == 0 && background != 0x3)
                        {
                            bgTex[background][yPosT2][xPosT2] = 0x00123456;
                        }
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

                        bgTex[background][yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                        if((currentByteRender & 0xF) == 0 && background != 0x3)
                        {
                            bgTex[background][yPosT2][xPosT2] = 0x00123456;
                        }
                    }

                    ++xPosT;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                ++yPosT;
                yPosT2 = (yTile * 8) + yPosT;
                //printf("tileLocate: 0x%X\n",tileLocate);
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            ++xTile;
            xPosT2 = xTile * 8;

            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));

            charIndex = charData & 0x1FF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            tileData = readMem(2,tileLocate);
            //printf("tileLocate: 0x%X\n",0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        ++yTile;
        yPosT = 0;
        yPosT2 = (yTile * 8);

        charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
        charIndex = charData & 0x1FF;
        tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
    }

    xPosT = 0;
    yPosT = 0;

    bgSize = (gbaREG.bgCNT[background] & 0xC000) >> 14;
    printf("bgSize: 0x%X\n", bgSize);
    return 0;
    switch(bgSize)
    {
        case 0:

        break;

        case 1:
            bg0Screen = (gbaREG.bgCNT[background] >> 8) & 0x1F;
            bg0Char = (gbaREG.bgCNT[background] >> 2) & 0x3;
            //breakpoint = true;
            //printf("bg0S: 0x%X\n",bg0Screen);
            //printf("bg0C: 0x%X\n",bg0Char);
            charData = readMem(1, (0x06000000 + 0x800) + bg0Screen * 0x800);
            charIndex = charData & 0x1FF;
            charPal = (charData >> 12) & 0xF;
            tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
            xPosT = 0;
            yPosT = 0;
            xPosT2 = 0;
            yPosT2 = 0;
            xTile = 0;
            yTile = 0;
            tileData = readMem(2,tileLocate);
            if((gbaREG.lcdControl & 0x800) != 0x00)
            {
            while(yTile != 0x20) // 0x1F
            {
                while(xTile != 0x20) // 0x1F
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

                                bgTex[background][yPosT2][xPosT2 + 256] = 0xFF << 24 |
                                                        readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                        readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                        readablePalRam[0][charPal][(currentByteRender & 0xF)];
                                if((currentByteRender & 0xF) == 0 && background != 0x3)
                                {
                                    bgTex[background][yPosT2][xPosT2 + 256] = 0x00123456;
                                }
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

                                bgTex[background][yPosT2][xPosT2 + 256] = 0xFF << 24 |
                                                        readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                        readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                        readablePalRam[0][charPal][(currentByteRender & 0xF)];
                                if((currentByteRender & 0xF) == 0 && background != 0x3)
                                {
                                    bgTex[background][yPosT2][xPosT2 + 256] = 0x00123456;
                                }
                            }

                            ++xPosT;
                            xPosT2 = flipFix[xPosT] + (xTile * 8);
                        }
                        //printf("xP: 0x%X\n",xPosT);
                        xPosT = 0;
                        xPosT2 = xTile * 8;
                        ++yPosT;
                        yPosT2 = (yTile * 8) + yPosT;
                        //printf("tileLocate: 0x%X\n",tileLocate);
                        tileLocate += 4;
                        tileData = readMem(2,tileLocate);
                    }
                    yPosT = 0;
                    yPosT2 = (yTile * 8);
                    ++xTile;
                    xPosT2 = xTile * 8;

                    charData = readMem(1, (0x06000000 + 0x800) + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));

                    charIndex = charData & 0x1FF;
                    tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
                    tileData = readMem(2,tileLocate);
                    //printf("tileLocate: 0x%X\n",0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
                }
                xPosT = 0;
                xTile = 0;
                xPosT2 = xTile * 8;
                ++yTile;
                yPosT = 0;
                yPosT2 = (yTile * 8);

                charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
                charIndex = charData & 0x1FF;
                tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            }

            }
            xPosT = 0;
            yPosT = 0;
        break;

        default:
            printf("UNIMPLEMENTED BACKGROUND MODE!\n");
        break;
    }
}
int renderBGmode0(uint8_t background)
{
    if((gbaREG.lcdControl & (0x100 << background) ) == 0x0)
    {
        return 0;
    }

    uint16_t xPosCopy2 = 0;
    uint16_t yPosCopy2 = 0;
    while(yPosCopy2 < 160)
    {
        while(xPosCopy2 < 240)
        {
            if(bgTex[background][(yPosCopy2 + gbaREG.bgYScroll[background]) & 0x1FF][(xPosCopy2 + gbaREG.bgXScroll[background]) & 0x1FF] != 0x00123456)
            {
                screenTexture[yPosCopy2][xPosCopy2] = bgTex[background]
                                                         [(yPosCopy2 + gbaREG.bgYScroll[background]) & 0x1FF]
                                                         [(xPosCopy2 + gbaREG.bgXScroll[background]) & 0x1FF];
            }
                xPosCopy2++;
        }
        xPosCopy2 = 0;
        yPosCopy2++;
        if(yPosCopy2 == 160)
        {
            return 0;
        }
    }
    return 0;
}
int basicMode0RenderMSC()
{

    uint32_t clearX = 0;
    uint32_t clearY = 0;
    while(clearY != 180)
    {
        while(clearX != 240)
        {
            screenTexture[clearY][clearX] = 0x0;
            clearX++;
        }
        clearX = 0;
        clearY++;
    }

    renderBGtoTexMode0(0);
    renderBGtoTexMode0(1);
    renderBGtoTexMode0(2);
    renderBGtoTexMode0(3);

    renderBGmode0(3);
    renderBGmode0(2);
    renderBGmode0(1);
    renderBGmode0(0);

    renderSprites();
    return 0;

    bg0Screen = (gbaREG.bgCNT[3] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[3] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg0S: 0x%X\n",bg0Screen);
    //printf("bg0C: 0x%X\n",bg0Char);
    uint32_t tileLocate;
    uint32_t tileData;
    uint16_t charIndex;
    uint16_t charData;
    uint8_t charPal;
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    charPal = (charData >> 12) & 0xF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    uint8_t currentByteRender;
    tileData = readMem(2,tileLocate);
    if((gbaREG.lcdControl & 0x800) != 0x00)
    {
    while(yTile != 0x14) // 0x14
    {
        while(xTile != 0x1F) // 0x1F
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

                        screenTexture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
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

                        screenTexture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                    }

                    ++xPosT;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                ++yPosT;
                yPosT2 = (yTile * 8) + yPosT;
                //printf("tileLocate: 0x%X\n",tileLocate);
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            ++xTile;
            xPosT2 = xTile * 8;

            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));

            charIndex = charData & 0x1FF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            tileData = readMem(2,tileLocate);
            //printf("tileLocate: 0x%X\n",0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        ++yTile;
        yPosT = 0;
        yPosT2 = (yTile * 8);

        charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
        charIndex = charData & 0x1FF;
        tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
    }

    }
    xPosT = 0;
    yPosT = 0;


    bg0Screen = (gbaREG.bgCNT[2] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[2] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg0S: 0x%X\n",bg0Screen);
    //printf("bg0C: 0x%X\n",bg0Char);
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    charPal = (charData >> 12) & 0xF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    tileData = readMem(2,tileLocate);
    if((gbaREG.lcdControl & 0x400) != 0x00)
    {
    while(yTile != 0x14)
    {
        while(xTile != 0x1F)
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
                    if(swapPal == false && (currentByteRender & 0xF) != 0 && xPosT2 - gbaREG.bgXScroll[2] < 240)
                    {

                        screenTexture[yPosT2][xPosT2 - gbaREG.bgXScroll[2]] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                    }
                    if(swapPal == true && (currentByteRender & 0xF) != 0)
                    {
                        uint8_t loopRead = 0;
                        while (loopRead != 0x10)
                        {
                            readablePalRam[0][0][loopRead] = loopRead * 0x10;
                            readablePalRam[1][0][loopRead] = loopRead * 0x10;
                            readablePalRam[2][0][loopRead] = loopRead * 0x10;
                            loopRead++;
                        }

                        screenTexture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                    }

                    ++xPosT;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                ++yPosT;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            ++xTile;
            xPosT2 = xTile * 8;
            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
            charIndex = charData & 0x1FF;
            charPal = (charData >> 12) & 0xF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            tileData = readMem(2,tileLocate);
            //printf("tileLocate: 0x%X\n",tileLocate);
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        ++yTile;
        yPosT = 0;
        yPosT2 = (yTile * 8);

        charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
        charIndex = charData & 0x1FF;
        tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
    }

    }
    xPosT = 0;
    yPosT = 0;

    bg0Screen = (gbaREG.bgCNT[1] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[1] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg0S: 0x%X\n",bg0Screen);
    //printf("bg0C: 0x%X\n",bg0Char);
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    charPal = (charData >> 12) & 0xF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    tileData = readMem(2,tileLocate);

    if((gbaREG.lcdControl & 0x200) != 0x00)
    {
    while(yTile != 0x14)
    {
        while(xTile != 0x1F)
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
                    if(swapPal == false && (currentByteRender & 0xF) != 0)
                    {

                        screenTexture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                    }
                    if(swapPal == true && (currentByteRender & 0xF) != 0)
                    {
                        uint8_t loopRead = 0;
                        while (loopRead != 0x10)
                        {
                            readablePalRam[0][0][loopRead] = loopRead * 0x10;
                            readablePalRam[1][0][loopRead] = loopRead * 0x10;
                            readablePalRam[2][0][loopRead] = loopRead * 0x10;
                            loopRead++;
                        }

                        screenTexture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                    }

                    ++xPosT;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                ++yPosT;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            ++xTile;
            xPosT2 = xTile * 8;
            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
            charIndex = charData & 0x1FF;
            charPal = (charData >> 12) & 0xF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            tileData = readMem(2,tileLocate);
            //printf("tileLocate: 0x%X\n",tileLocate);
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        ++yTile;
        yPosT = 0;
        yPosT2 = (yTile * 8);

        charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
        charIndex = charData & 0x1FF;
        tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
    }

    }
    xPosT = 0;
    yPosT = 0;

    bg0Screen = (gbaREG.bgCNT[0] >> 8) & 0x1F;
    bg0Char = (gbaREG.bgCNT[0] >> 2) & 0x3;
    //breakpoint = true;
    //printf("bg0S: 0x%X\n",bg0Screen);
    //printf("bg0C: 0x%X\n",bg0Char);
    charData = readMem(1, 0x06000000 + bg0Screen * 0x800);
    charIndex = charData & 0x1FF;
    charPal = (charData >> 12) & 0xF;
    tileLocate = 0x06000000 + charIndex + (bg0Char * 0x4000);
    xPosT = 0;
    yPosT = 0;
    xPosT2 = 0;
    yPosT2 = 0;
    xTile = 0;
    yTile = 0;
    tileData = readMem(2,tileLocate);
    if((gbaREG.lcdControl & 0x100) != 0x00)
    {
    while(yTile != 0x14)
    {
        while(xTile != 0x1F)
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
                    if(swapPal == false && (currentByteRender & 0xF) != 0)
                    {

                        screenTexture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                    }
                    if(swapPal == true && (currentByteRender & 0xF) != 0)
                    {
                        uint8_t loopRead = 0;
                        while (loopRead != 0x10)
                        {
                            readablePalRam[0][0][loopRead] = loopRead * 0x10;
                            readablePalRam[1][0][loopRead] = loopRead * 0x10;
                            readablePalRam[2][0][loopRead] = loopRead * 0x10;
                            loopRead++;
                        }

                        screenTexture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][charPal][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][charPal][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][charPal][(currentByteRender & 0xF)];
                    }

                    ++xPosT;
                    xPosT2 = flipFix[xPosT] + (xTile * 8);
                }
                //printf("xP: 0x%X\n",xPosT);
                xPosT = 0;
                xPosT2 = xTile * 8;
                ++yPosT;
                yPosT2 = (yTile * 8) + yPosT;
                tileLocate += 4;
                tileData = readMem(2,tileLocate);
            }
            yPosT = 0;
            yPosT2 = (yTile * 8);
            ++xTile;
            xPosT2 = xTile * 8;
            charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
            charIndex = charData & 0x1FF;
            charPal = (charData >> 12) & 0xF;
            tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
            tileData = readMem(2,tileLocate);
            //printf("tileLocate: 0x%X\n",tileLocate);
        }
        xPosT = 0;
        xTile = 0;
        xPosT2 = xTile * 8;
        ++yTile;
        yPosT = 0;
        yPosT2 = (yTile * 8);

        charData = readMem(1, 0x06000000 + (bg0Screen * 0x800) + (xTile * 2) + (yTile * 0x40));
        charIndex = charData & 0x1FF;
        tileLocate = 0x06000000 + (charIndex * 0x20) + (bg0Char * 0x4000);
    }

    }
    xPosT = 0;
    yPosT = 0;
    renderSprites();
    return 0;
}

void basicTiles1Render()
{
    uint32_t tileLocate = 0x06000000;
    if(toggleTileDisplay == true)
    {
        tileLocate = 0x06008000;
    }
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
                    currentByteRender = tileData >> (28 - ((xPosT) * 4));
                    if(swapPal == false)
                    {
                        tiles1Texture[yPosT2][xPosT2] = 0xFF << 24 |
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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
                                                readablePalRam[2][0][(currentByteRender & 0xF)] << 16 |
                                                readablePalRam[1][0][(currentByteRender & 0xF)] << 8 |
                                                readablePalRam[0][0][(currentByteRender & 0xF)];
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
                    //screenTexture[yPos + tileCordY][xPos + tileCordX] = 0xFF << 24 | currentByteRender2 << 8 | 0x00;
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
    //currentDispMode2 = gbaREG.lcdControl;
    currentDispMode = gbaREG.lcdControl & 0x7;
    switch(currentDispMode)
    {
        /*
        case 1:
            basicRenderMode1();
            //printf("MODE 1\n\n\n\n\n\n\n\n\n\n\n");
        break;
        */

        case 0:
            basicMode0RenderMSC();
        break;

        case 4:
            basicRenderMode4();
        break;

        default:
            printf("Unimplemented Render Mode 0x%X!\nDefaulting to Default!\n",currentDispMode);
            if(currentDispMode < 3)
            {
                basicMode0Render();
            }
            if(currentDispMode >= 3)
            {
                basicRenderMode4();
            }
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

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,240,180,0,GL_RGBA,GL_UNSIGNED_BYTE,*screenTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, 0);
    handleDebugWindow();
}
