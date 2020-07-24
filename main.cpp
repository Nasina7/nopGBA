#include "cpu.hpp"
using namespace std;
char option;
uint64_t opcodesRan;
int main()
{
    //breakpoint = true;
    mainScreen = SDL_CreateWindow("nopGBA",0,0,240 * 2,160 * 2,SDL_WINDOW_RESIZABLE);
    mainRenderer = SDL_CreateRenderer(mainScreen,-1,SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(mainRenderer,2,2);
    gbaREG.rNUM[13] = 0x03007F00;
    gbaREG.rNUM[14] = 0x08000000;
    gbaREG.rNUM[15] = gbaREG.rNUM[14];
    gbaREG.cpsr = 0x1F;
    gbaREG.spsr = 0x10;
    loadROM();
    while(true)
    {
        if(gbaREG.cpsr[5] == 0)
        {
            //printCurrentOpcode();
            //printRegs();
            currentOpcode = readMem(2,gbaREG.rNUM[15]);
            //currentOpcode = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 3] << 24 |
            //                gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 2] << 16 |
            //                gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 1] << 8 |
            //                gbaRAM.flashROM[gbaREG.rNUM[15] - 0x08000000];
            //currentOpcode = readMem(2,gbaREG.rNUM[15]);
            doOpcode(currentOpcode);
            //op32bit = currentOpcode;
            //printf("In Binary: ");
            //std::cout<<op32bit<<std::endl;
        }
        if(gbaREG.cpsr[5] == 1)
        {
            //printf("PC: 0x%X\n",gbaREG.rNUM[15]);
            if(gbaREG.rNUM[15] == 0x02000000)
            {
                memDump();
                printf("MEMDUMP!\n");
            }
            //currentThumbOpcode = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 1] << 8 |
            //                     gbaRAM.flashROM[gbaREG.rNUM[15] - 0x08000000];
            currentThumbOpcode = readMem(1,gbaREG.rNUM[15]);
            op16bit = currentThumbOpcode;
            //printf("Thumb Opcode: 0x%X\n",currentThumbOpcode);
            //printf("In Binary: ");
            //std::cout<<op16bit<<std::endl;
            doThumbOpcode(currentThumbOpcode);
        }
        //printf("R1: 0x%X\n",gbaREG.rNUM[1]);
        if(gbaREG.rNUM[15] == 0x08042EA0)
        {
            //breakpoint = true;
        }
        if(breakpoint == true)
        {
            printf("Opcode Thumb: 0x%X\n",currentThumbOpcode);
            printRegs();
            printf("Opcodes Ran: %i\n",opcodesRan);
            printf("Would you like to continue?\n");
            cin>>option;
            memDump();
            if(option == 'n')
            {
                breakpoint = false;
            }
        }
        //printRegs();
        //printf("PC: 0x%X\n",gbaREG.rNUM[15]);
        if(opcodeError == true)
        {
            printf("In Binary: ");
            std::cout<<op16bit<<std::endl;
            printf("lcdControl: 0x%X\n",gbaREG.lcdControl);
            memDump();
            printRegs();
            printf("Opcodes Ran: %i\n",opcodesRan);
            cin>>option;
            return 1;
        }
        opcodesRan++;
        if(opcodesRan % 100000 == 0)
        {
            basicRenderMode4();
        }
    }
    return 0;
}
