#include "cpu.hpp"
using namespace std;
char option;
uint64_t opcodesRan;
uint32_t testPrev;
int main()
{
    //breakpoint = true;
    mainScreen = SDL_CreateWindow("nopGBA",0,0,240 * 2,160 * 2,SDL_WINDOW_RESIZABLE);
    mainRenderer = SDL_CreateRenderer(mainScreen,-1,SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(mainRenderer,2,2);
    gbaREG.rNUM[13] = 0x03007F00;
    gbaREG.rNUM[14] = 0x08000000;
    gbaREG.R1314_svc[0] = 0x03007FE0;
    gbaREG.R1314_svc[1] = 0x08000000;
    gbaREG.R1314_irq[0] = 0x03007F00;
    gbaREG.R1314_irq[1] = 0x08000000;
    gbaREG.rNUM[15] = gbaREG.rNUM[14];
    gbaREG.cpsr = 0x1F;
    gbaREG.spsr = 0x10;
    loadROM();
    while(true)
    {
        testPrev = gbaREG.rNUM[13];
        gbaREG.prevCpsr = gbaREG.cpsr.to_ulong();
        doOpcodeMain();
        handleDMA();
        if(testPrev != gbaREG.rNUM[13])
        {
            //breakpoint = true;
            printf("STACK CHANGE!\n");
        }
        checkModeSwitch();
        //printf("R1: 0x%X\n",gbaREG.rNUM[1]);
        if(gbaREG.rNUM[15] == 0x0800033C || gbaREG.rNUM[15] == 0x080004EC)
        {
            //breakpoint = true;
        }
        //std::cout<<"CPSR: "<<gbaREG.cpsr<<std::endl;
        if(breakpoint == true)
        {
            printf("Opcode Thumb: 0x%X\n",currentThumbOpcode);
            printRegs();
            //printDMARegs();
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
            printf("LAST THUMB In Binary: ");
            std::cout<<op16bit<<std::endl;
            printf("LAST ARM In Binary: ");
            op32bit = currentOpcode;
            std::cout<<op32bit<<std::endl;
            printf("lcdControl: 0x%X\n",gbaREG.lcdControl);
            memDump();
            printRegs();
            //printDMARegs();
            printf("Opcodes Ran: %i\n",opcodesRan);
            cin>>option;
            return 1;
        }
        opcodesRan++;
        if(opcodesRan % 6500 == 0)
        {
            printf("lcdControl: 0x%X\n",gbaREG.lcdControl);
            basicRenderMode4();
            handleSDLcontrol();
        }
    }
    return 0;
}
