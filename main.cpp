#include "cpu.hpp"

using namespace std;
int main()
{
    gbaREG.rNUM[13] = 0x03007F00;
    gbaREG.rNUM[14] = 0x08000000;
    gbaREG.rNUM[15] = gbaREG.rNUM[14];
    gbaREG.cpsr = 0x1F;
    gbaREG.spsr = 0x10;
    loadROM();
    while(true)
    {
        printCurrentOpcode();
        currentOpcode = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 3] << 24 |
                        gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 2] << 16 |
                        gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 1] << 8 |
                        gbaRAM.flashROM[gbaREG.rNUM[15] - 0x08000000];
        //currentOpcode = readMem(2,gbaREG.rNUM[15]);
        doOpcode(currentOpcode);
        //printRegs();
        if(opcodeError == true)
        {
            return 1;
        }
    }
    return 0;
}
