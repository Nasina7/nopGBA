#include "include.hpp"
void basicRenderMode4()
{
    int xPos = 0;
    int yPos = 0;
    uint8_t currentByteRender;
    while(yPos != 160)
    {
        while(xPos != 240)
        {
            currentByteRender = gbaRAM.VideoRAM[(yPos * 240) + xPos];
            if(currentByteRender == 0)
            {
                SDL_SetRenderDrawColor(mainRenderer,0x00,0x00,0x00,0x00);
            }
            if(currentByteRender != 0)
            {
                SDL_SetRenderDrawColor(mainRenderer,0xFF,0xFF,0xFF,0x00);
            }
            SDL_RenderDrawPoint(mainRenderer,xPos,yPos);
            xPos++;
        }
        xPos = 0;
        yPos++;
    }
    xPos = 0;
    yPos = 0;
    SDL_RenderPresent(mainRenderer);
}
