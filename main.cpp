#include "cpu.hpp"
using namespace std;
char option;
uint32_t testPrev;
uint32_t testPrev2;
uint32_t test15;
int main()
{
    //breakpoint = true;
    initFunc();
    glGenTextures(8,&screenTexGL);
    //SDL_RenderSetScale(mainRenderer,2,2);
    //printf("test\n");
    gbaREG.rNUM[13] = 0x03007F00;
    gbaREG.rNUM[14] = 0x08000000;
    gbaREG.R1314_svc[0] = 0x03007FD0;
    gbaREG.R1314_svc[1] = 0x08000000;
    gbaREG.R1314_irq[0] = 0x03007F00;
    gbaREG.R1314_irq[1] = 0x08000000;
    gbaREG.rNUM[15] = gbaREG.rNUM[14];
    gbaREG.cpsr = 0x1F;
    gbaREG.spsr = 0x10;
    //loadROM();
    //printf("test\n");
    //internalFPSBenchmark();
    doInternalFramerate();
    while(closenopGBA == false)
    {
        while(allowRun == false)
        {
            handleMainGUI();
            handleSDLcontrol();
        }
        if(gbaREG.currentlyHalted == false)
        {
            testPrev = gbaREG.rNUM[13];
            gbaREG.prevCpsr = gbaREG.cpsr.to_ulong();
            doOpcodeMain();
            test15 = gbaREG.rNUM[15];
            if(gbaREG.cpsr[5] == 0)
            {
                gbaREG.rNUM[15] = gbaREG.rNUM[15] & 0xFFFFFFFC;
            }
            if(gbaREG.cpsr[5] == 1)
            {
                gbaREG.rNUM[15] = gbaREG.rNUM[15] & 0xFFFFFFFE;
            }
            if(test15 != gbaREG.rNUM[15])
            {
                printf("BROKEN JUMP PREVENTED!\n");
                breakpoint = true;
            }
        }
        if(doInterrupts == true)
        {
            handleInterrupts();
        }
        //gbaRAM.onChipWRAM[0x1EC4] = 0x00;
        opcodesRan++;
        if(gbaREG.currentlyHalted == false)
        {
            handleDMA();
            checkModeSwitch();
            handleBreakpoint();
            if(breakpoint == true)
            {
                printRegs();
                allowRun = false;
                while(allowRun == false)
                {
                    handleMainGUI();
                    handleSDLcontrol();
                }
            }
            if(opcodeError == true)
            {

                allowRun = false;
                dontDisplayError = false;

                printf("LAST THUMB In Binary: ");
                std::cout<<op16bit<<std::endl;
                printf("LAST ARM In Binary: ");
                op32bit = currentOpcode;
                std::cout<<op32bit<<std::endl;
                printf("lcdControl: 0x%X\n",gbaREG.lcdControl);
                memDump();
                printRegs();
                printf("Opcodes Ran: %i\n",opcodesRan);
            }
        }
        //printf("R15: 0x%X\n",gbaREG.rNUM[15]);
        //breakpoint = true;
        if(opcodesRan % opcodesPerFrame == 0)
        {
            //printf("lcdControl: 0x%X\n",gbaREG.lcdControl);
            //basicRenderMode4();
            updateReadPalF();
            handleRendering();
            handleSDLcontrol();
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(debugScreen);
    SDL_Quit();
    return 0;
}
