#include <iostream>
#include <bitset>
class nGBARAM
{
    public:
        uint8_t biosRAM[0x4000]; // 0x00000000 - 0x00003FFF
        // 0x00004000 - 0x01FFFFFF UNUSED
        uint8_t onBoardWRAM[0x40000]; // 0x02000000 - 0x0203FFFF
        // 0x02040000 - 0x02FFFFFF UNUSED
        uint8_t onChipWRAM[0x8000]; // 0x03000000 - 0x03007FFF
        // 0x03008000 - 0x03FFFFFF UNUSED
        // 0x04000000 - 0x040003FE IO REGS (Will redirect writes)
        // 0x04000400 - 0x04FFFFFF UNUSED
        uint8_t Bg_ObjPalRam[0x400]; // 05000000-050003FF
        // 05000400-05FFFFFF UNUSED
        uint8_t VideoRAM[0x18000]; // 06000000-06017FFF
        // 06018000-06FFFFFF UNUSED
        uint8_t ObjectRAM[0x400]; // 07000000-070003FF
        // 07000400-07FFFFFF UNUSED
        uint8_t flashROM[0x6000000];
        uint8_t SaveRAM[0x10000];
        // 0E010000 - FFFFFFFF UNUSED
};
class nGBACPU
{
    public:
        uint32_t rNUM[16];
        std::bitset<32> cpsr;
        std::bitset<32> spsr;
};
uint32_t currentOpcode;
bool opcodeError;
std::bitset<32> op32bit;
uint8_t opCondition;
uint8_t opDetermine;
nGBACPU gbaREG;
nGBARAM gbaRAM;
void printCurrentOpcode()
{
    printf("Current Opcode: 0x%02X%02X%02X%02X\n",
    gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 3],
    gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 2],
    gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 1],
    gbaRAM.flashROM[gbaREG.rNUM[15] - 0x08000000]);
}
void printRegs()
{
    for(int i = 0; i != 16; i++)
    {
        printf("R%i: 0x%02X  ",i,gbaREG.rNUM[i]);
    }
    std::cout<<"CPSR: "<<gbaREG.cpsr<<std::endl;
}
uint16_t return16;
uint32_t return32;
uint32_t readMem(int readMode, int location)
{
    switch(location)
    {
    case 0x0000 ... 0x4000:
        if(readMode == 0)
        {
            return gbaRAM.biosRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.biosRAM[location] << 8 |
                       gbaRAM.biosRAM[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.biosRAM[location] << 24 |
                       gbaRAM.biosRAM[location + 1] << 16 |
                       gbaRAM.biosRAM[location + 2] << 8 |
                       gbaRAM.biosRAM[location + 3];
            return return32;
        }
    break;

    case 0x02000000 ... 0x0203FFFF:
        location -= 0x02000000;
        if(readMode == 0)
        {
            return gbaRAM.onBoardWRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.onBoardWRAM[location] << 8 |
                       gbaRAM.onBoardWRAM[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.onBoardWRAM[location] << 24 |
                       gbaRAM.onBoardWRAM[location + 1] << 16 |
                       gbaRAM.onBoardWRAM[location + 2] << 8 |
                       gbaRAM.onBoardWRAM[location + 3];
            return return32;
        }
    break;

    case 0x03000000 ... 0x03007FFF:
        location -= 0x03000000;
        if(readMode == 0)
        {
            return gbaRAM.onChipWRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.onChipWRAM[location] << 8 |
                       gbaRAM.onChipWRAM[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.onChipWRAM[location] << 24 |
                       gbaRAM.onChipWRAM[location + 1] << 16 |
                       gbaRAM.onChipWRAM[location + 2] << 8 |
                       gbaRAM.onChipWRAM[location + 3];
            return return32;
        }
    break;

    case 0x04000000 ... 0x040003FE:
        printf("IO REG NOT IMPLEMENTED YET! READ!");
        return 0;
    break;

    case 0x05000000 ... 0x050003FF:
        location -= 0x05000000;
        if(readMode == 0)
        {
            return gbaRAM.Bg_ObjPalRam[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.Bg_ObjPalRam[location] << 8 |
                       gbaRAM.Bg_ObjPalRam[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.Bg_ObjPalRam[location] << 24 |
                       gbaRAM.Bg_ObjPalRam[location + 1] << 16 |
                       gbaRAM.Bg_ObjPalRam[location + 2] << 8 |
                       gbaRAM.Bg_ObjPalRam[location + 3];
            return return32;
        }
    break;

    case 0x06000000 ... 0x06017FFF:
        location -= 0x06000000;
        if(readMode == 0)
        {
            return gbaRAM.VideoRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.VideoRAM[location] << 8 |
                       gbaRAM.VideoRAM[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.VideoRAM[location] << 24 |
                       gbaRAM.VideoRAM[location + 1] << 16 |
                       gbaRAM.VideoRAM[location + 2] << 8 |
                       gbaRAM.VideoRAM[location + 3];
            return return32;
        }
    break;

    case 0x07000000 ... 0x070003FF:
        location -= 0x06000000;
        if(readMode == 0)
        {
            return gbaRAM.ObjectRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.ObjectRAM[location] << 8 |
                       gbaRAM.ObjectRAM[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.ObjectRAM[location] << 24 |
                       gbaRAM.ObjectRAM[location + 1] << 16 |
                       gbaRAM.ObjectRAM[location + 2] << 8 |
                       gbaRAM.ObjectRAM[location + 3];
            return return32;
        }
    break;

    case 0x08000000 ... 0x0DFFFFFF:
        location -= 0x08000000;
        if(readMode == 0)
        {
            return gbaRAM.flashROM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.flashROM[location] << 8 |
                       gbaRAM.flashROM[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.flashROM[location] << 24 |
                       gbaRAM.flashROM[location + 1] << 16 |
                       gbaRAM.flashROM[location + 2] << 8 |
                       gbaRAM.flashROM[location + 3];
            return return32;
        }
    break;

    case 0x0E000000 ... 0x0E00FFFF:
        location -= 0x08000000;
        if(readMode == 0)
        {
            return gbaRAM.SaveRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.SaveRAM[location] << 8 |
                       gbaRAM.SaveRAM[location + 1];
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.SaveRAM[location] << 24 |
                       gbaRAM.SaveRAM[location + 1] << 16 |
                       gbaRAM.SaveRAM[location + 2] << 8 |
                       gbaRAM.SaveRAM[location + 3];
            return return32;
        }
    break;
    }
}

uint8_t *memPointer;
void writeMem(uint8_t writeMode, uint32_t location, uint32_t value)
{
    switch(location)
    {
        case 0x04000000 ... 0x040003FE:
            printf("THIS IS IO REG STUB WRITE!\n");
        break;

        default:
            opcodeError = true;
            printf("Unimplemented Write ADDR 0x%02X\n",location);
        break;
    }
}
