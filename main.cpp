#include "cpu.hpp"
using namespace std;
char option;
uint32_t testPrev;
int main()
{
    //breakpoint = true;
    initFunc();
    glGenTextures(1,&screenTexGL);
    //SDL_RenderSetScale(mainRenderer,2,2);
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
    while(closenopGBA == false)
    {
        while(allowRun == false)
        {
            handleMainGUI();
            handleSDLcontrol();
        }
        testPrev = gbaREG.rNUM[13];
        gbaREG.prevCpsr = gbaREG.cpsr.to_ulong();
        doOpcodeMain();
        opcodesRan++;
        handleDMA();
        //printf("R13: 0x%X\n",gbaREG.rNUM[13]);
        //printf("R13_svc: 0x%X\n",gbaREG.R1314_svc[0]);
        //printf("R13_1rq: 0x%X\n",gbaREG.R1314_irq[0]);
        if(testPrev != gbaREG.rNUM[13])
        {
            //breakpoint = true;
            //printf("R13: 0x%X\n",gbaREG.rNUM[13]);
            //printf("R13_svc: 0x%X\n",gbaREG.R1314_svc[0]);
            //printf("R13_1rq: 0x%X\n",gbaREG.R1314_irq[0]);
            //printf("STACK CHANGE!\n");
        }
        checkModeSwitch();
        //printf("R15: 0x%X\n",gbaREG.rNUM[15]);
        //breakpoint = true;
        if(gbaREG.rNUM[15] == breakpointLocation && breakpointEnabled == true)
        {
            breakpoint = true;
        }
        if(gbaREG.rNUM[15] == 0x0814FCE0)
        {
            //breakpoint = true;
        }
        if(gbaREG.rNUM[15] == 0x0800ACA2)
        {
            breakpoint = true;
        }
        if(opcodesRan == 29131)
        {
            //breakpoint = true;
        }
        if(gbaREG.rNUM[15] == 0x0814F404)
        {
            //breakpoint = true;
        }
        if(gbaREG.R1314_svc[0] == 0x03007FC0)
        {
            //breakpoint = true;
        }
        if(opcodesRan % 13000 == 0)
        {
            printf("lcdControl: 0x%X\n",gbaREG.lcdControl);
            //basicRenderMode4();
            handleRendering();
            handleSDLcontrol();
        }
        //std::cout<<"CPSR: "<<gbaREG.cpsr<<std::endl;
        if(breakpoint == true)
        {
            printRegs();
            allowRun = false;
            while(allowRun == false)
            {
                handleMainGUI();
                handleSDLcontrol();
            }
            //printf("Opcode Thumb: 0x%X\n",currentThumbOpcode);
            //printRegs();
            //printDMARegs();
            //printf("Opcodes Ran: %i\n",opcodesRan);
            //printf("Would you like to continue?\n");
            //cin>>option;
            //memDump();
            //if(option == 'n')
            //{
            //    breakpoint = false;
            //}
        }
        //printRegs();
        //printf("PC: 0x%X\n",gbaREG.rNUM[15]);
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
            //printDMARegs();
            printf("Opcodes Ran: %i\n",opcodesRan);
            //ImGui_ImplOpenGL3_Shutdown();
            //ImGui_ImplSDL2_Shutdown();
            //ImGui::DestroyContext();

            //SDL_GL_DeleteContext(gl_context);
            //SDL_DestroyWindow(debugScreen);
            //SDL_Quit();
            //return 1;
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
