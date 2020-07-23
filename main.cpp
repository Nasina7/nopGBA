#include "cpu.hpp"
using namespace std;
char option;
uint64_t opcodesRan;
int main()
{
    //breakpoint = true;
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
            printCurrentOpcode();
            currentOpcode = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 3] << 24 |
                            gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 2] << 16 |
                            gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 1] << 8 |
                            gbaRAM.flashROM[gbaREG.rNUM[15] - 0x08000000];
            //currentOpcode = readMem(2,gbaREG.rNUM[15]);
            doOpcode(currentOpcode);
            op32bit = currentOpcode;
            printf("In Binary: ");
            std::cout<<op32bit<<std::endl;
        }
        if(gbaREG.cpsr[5] == 1)
        {
            currentThumbOpcode = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 1] << 8 |
                                 gbaRAM.flashROM[gbaREG.rNUM[15] - 0x08000000];
            op16bit = currentThumbOpcode;
            //printf("Thumb Opcode: 0x%X\n",currentThumbOpcode);
            //printf("In Binary: ");
            //std::cout<<op16bit<<std::endl;
            doThumbOpcode(currentThumbOpcode);
        }
        //printf("R1: 0x%X\n",gbaREG.rNUM[1]);
        if(gbaREG.rNUM[15] == 0x080041D2)
        {
            //breakpoint = true;
        }
        if(breakpoint == true)
        {
            printf("Opcode Thumb: 0x%X\n",currentThumbOpcode);
            printRegs();
            printf("Would you like to continue?\n");
            cin>>option;
            if(option == 'n')
            {
                breakpoint = false;
            }
        }
        //printRegs();
        if(opcodeError == true)
        {
            printRegs();
            printf("Opcodes Ran: %i\n",opcodesRan);
            return 1;
        }
        opcodesRan++;
    }
    return 0;
}
