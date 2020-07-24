#include <iostream>
#include <bitset>
#include <SDL2/SDL.h>
SDL_Window* mainScreen;
SDL_Renderer* mainRenderer;
std::bitset<10> joypadReg = 0xFFF;
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
        uint64_t gbaCycles;
        uint8_t gbaScanline;
        uint16_t lcdControl;
};
bool breakpoint;
uint32_t currentOpcode;
uint16_t currentThumbOpcode;
bool opcodeError;
std::bitset<32> op32bit;
std::bitset<16> op16bit;
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
void memDump()
{
    FILE* memdump1 = fopen("memdump/biosRAM.nGBA","w+");
    fwrite(gbaRAM.biosRAM,sizeof(uint8_t),sizeof(gbaRAM.biosRAM),memdump1);
    memdump1 = fopen("memdump/onBoardWRAM.nGBA","w+");
    fwrite(gbaRAM.onBoardWRAM,sizeof(uint8_t),sizeof(gbaRAM.onBoardWRAM),memdump1);
    memdump1 = fopen("memdump/onChipWRAM.nGBA","w+");
    fwrite(gbaRAM.onChipWRAM,sizeof(uint8_t),sizeof(gbaRAM.onChipWRAM),memdump1);
    memdump1 = fopen("memdump/bg_objPalRAM.nGBA","w+");
    fwrite(gbaRAM.Bg_ObjPalRam,sizeof(uint8_t),sizeof(gbaRAM.Bg_ObjPalRam),memdump1);
    memdump1 = fopen("memdump/videoRAM.nGBA","w+");
    fwrite(gbaRAM.VideoRAM,sizeof(uint8_t),sizeof(gbaRAM.VideoRAM),memdump1);
    memdump1 = fopen("memdump/objectRAM.nGBA","w+");
    fwrite(gbaRAM.ObjectRAM,sizeof(uint8_t),sizeof(gbaRAM.ObjectRAM),memdump1);
    memdump1 = fopen("memdump/saveRAM.nGBA","w+");
    fwrite(gbaRAM.SaveRAM,sizeof(uint8_t),sizeof(gbaRAM.SaveRAM),memdump1);
    fclose(memdump1);
}
std::bitset<1> backupBit2;
std::bitset<32> rotateValNeed;
uint32_t rotateValue32(bool goRight, uint8_t rotateTimes, uint32_t valueNeed)
{
    rotateValNeed = valueNeed;
    if(goRight == true)
    {
        if(rotateTimes == 0)
        {
            return rotateValNeed.to_ulong();
        }
        while(rotateTimes != 0xFF)
        {
            backupBit2[0] = rotateValNeed[0];
            rotateValNeed = rotateValNeed >> 1;
            rotateValNeed[31] = backupBit2[0];
            rotateTimes--;
        }
    }
    if(goRight == false)
    {
        if(rotateTimes == 0)
        {
            return rotateValNeed.to_ulong();
        }
        while(rotateTimes != 0xFF)
        {
            backupBit2[0] = rotateValNeed[31];
            rotateValNeed = rotateValNeed << 1;
            rotateValNeed[0] = backupBit2[0];
            rotateTimes--;
        }
    }
    return rotateValNeed.to_ulong();
}

// From CPU.hpp
uint8_t opRD;
uint8_t opRN;
uint8_t opRS;
uint8_t opRB;
uint8_t opRM;
bool doFlagUpdate;
bool B25ShifterDet;
std::bitset<4> opRDET;
uint32_t shifterResult;
bool pBit;
bool uBit;
bool wBit;
bool lBit;
bool iBit;
bool bBit;
bool sBit;
// END
uint32_t endRotate;
uint8_t shiftIMM;
std::bitset<5> shiftIMM2;
uint32_t getLSLbyIMM()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    shiftIMM2 = currentOpcode >> 7;
    shiftIMM = shiftIMM2.to_ulong();
    endRotate = gbaREG.rNUM[opRM] << shiftIMM;
    return endRotate;
}
uint32_t getRRbyIMM()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    shiftIMM2 = currentOpcode >> 7;
    shiftIMM = shiftIMM2.to_ulong();
    endRotate = rotateValue32(true,shiftIMM,gbaREG.rNUM[opRM]);
    return endRotate;
}
uint16_t return16;
uint32_t return32;
uint8_t fix1;
uint8_t fix2;
uint8_t fix3;
uint8_t fix4;
uint8_t randoRet0404;
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.biosRAM[location] << 24 |
                       gbaRAM.biosRAM[location + 1] << 16 |
                       gbaRAM.biosRAM[location + 2] << 8 |
                       gbaRAM.biosRAM[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.onBoardWRAM[location] << 24 |
                       gbaRAM.onBoardWRAM[location + 1] << 16 |
                       gbaRAM.onBoardWRAM[location + 2] << 8 |
                       gbaRAM.onBoardWRAM[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.onChipWRAM[location] << 24 |
                       gbaRAM.onChipWRAM[location + 1] << 16 |
                       gbaRAM.onChipWRAM[location + 2] << 8 |
                       gbaRAM.onChipWRAM[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
            return return32;
        }
    break;

    case 0x04000000 ... 0x040003FE:
        switch(location)
        {
            case 0x04000004:
                randoRet0404++;
                randoRet0404 = randoRet0404 % 2;
                return randoRet0404;
            break;

            case 0x04000130:
                joypadReg.flip(7);
                return joypadReg.to_ulong();
            break;

            default:
                printf("IO REG NOT IMPLEMENTED YET! READ 0x%X\n!",location);
                return 0;
            break;
        }
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.Bg_ObjPalRam[location] << 24 |
                       gbaRAM.Bg_ObjPalRam[location + 1] << 16 |
                       gbaRAM.Bg_ObjPalRam[location + 2] << 8 |
                       gbaRAM.Bg_ObjPalRam[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.VideoRAM[location] << 24 |
                       gbaRAM.VideoRAM[location + 1] << 16 |
                       gbaRAM.VideoRAM[location + 2] << 8 |
                       gbaRAM.VideoRAM[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.ObjectRAM[location] << 24 |
                       gbaRAM.ObjectRAM[location + 1] << 16 |
                       gbaRAM.ObjectRAM[location + 2] << 8 |
                       gbaRAM.ObjectRAM[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.flashROM[location] << 24 |
                       gbaRAM.flashROM[location + 1] << 16 |
                       gbaRAM.flashROM[location + 2] << 8 |
                       gbaRAM.flashROM[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
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
            fix1 = return16 >> 8;
            fix2 = return16;
            return16 = fix2 << 8 | fix1;
            return return16;
        }
        if(readMode == 2)
        {
            return32 = gbaRAM.SaveRAM[location] << 24 |
                       gbaRAM.SaveRAM[location + 1] << 16 |
                       gbaRAM.SaveRAM[location + 2] << 8 |
                       gbaRAM.SaveRAM[location + 3];
            fix1 = return32 >> 24;
            fix2 = return32 >> 16;
            fix3 = return32 >> 8;
            fix4 = return32;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
            return return32;
        }
    break;
    }
}

void writeMem(uint8_t writeMode, uint32_t location, uint32_t value)
{
    switch(location)
    {
        case 0x02000000 ... 0x0203FFFF:
            location -= 0x02000000;
            if(writeMode == 0)
            {
                gbaRAM.onBoardWRAM[location] = value;
            }
            if(writeMode == 1)
            {
                fix1 = value >> 8;
                fix2 = value;
                gbaRAM.onBoardWRAM[location + 1] = fix1;
                gbaRAM.onBoardWRAM[location] = fix2;
            }
            if(writeMode == 2)
            {
                fix1 = value >> 24;
                fix2 = value >> 16;
                fix3 = value >> 8;
                fix4 = value;
                gbaRAM.onBoardWRAM[location + 3] = fix1;
                gbaRAM.onBoardWRAM[location + 2] = fix2;
                gbaRAM.onBoardWRAM[location + 1] = fix3;
                gbaRAM.onBoardWRAM[location] = fix4;
            }
        break;

        case 0x03000000 ... 0x03007FFF:
            location -= 0x03000000;
            if(writeMode == 0)
            {
                gbaRAM.onChipWRAM[location] = value;
            }
            if(writeMode == 1)
            {
                fix1 = value >> 8;
                fix2 = value;
                gbaRAM.onChipWRAM[location + 1] = fix1;
                gbaRAM.onChipWRAM[location] = fix2;
            }
            if(writeMode == 2)
            {
                fix1 = value >> 24;
                fix2 = value >> 16;
                fix3 = value >> 8;
                fix4 = value;
                gbaRAM.onChipWRAM[location + 3] = fix1;
                gbaRAM.onChipWRAM[location + 2] = fix2;
                gbaRAM.onChipWRAM[location + 1] = fix3;
                gbaRAM.onChipWRAM[location] = fix4;
            }
        break;

        case 0x04000000 ... 0x040003FE:
            switch(location)
            {
                case 0x04000000:
                    gbaREG.lcdControl = value;
                break;

                default:
                    printf("THIS IS IO REG STUB WRITE!\n");
                break;
            }
        break;

        case 0x05000000 ... 0x050003FF:
            location -= 0x05000000;
            if(writeMode == 0)
            {
                gbaRAM.Bg_ObjPalRam[location] = value;
            }
            if(writeMode == 1)
            {
                fix1 = value >> 8;
                fix2 = value;
                gbaRAM.Bg_ObjPalRam[location + 1] = fix1;
                gbaRAM.Bg_ObjPalRam[location] = fix2;
            }
            if(writeMode == 2)
            {
                fix1 = value >> 24;
                fix2 = value >> 16;
                fix3 = value >> 8;
                fix4 = value;
                gbaRAM.Bg_ObjPalRam[location + 3] = fix1;
                gbaRAM.Bg_ObjPalRam[location + 2] = fix2;
                gbaRAM.Bg_ObjPalRam[location + 1] = fix3;
                gbaRAM.Bg_ObjPalRam[location] = fix4;
            }
        break;

        case 0x06000000 ... 0x06017FFF:
            if(value != 0)
            {
                printf("VWRITE!\n");
            }
            location -= 0x06000000;
            if(writeMode == 0)
            {
                gbaRAM.VideoRAM[location] = value;
            }
            if(writeMode == 1)
            {
                fix1 = value >> 8;
                fix2 = value;
                gbaRAM.VideoRAM[location + 1] = fix1;
                gbaRAM.VideoRAM[location] = fix2;
            }
            if(writeMode == 2)
            {
                fix1 = value >> 24;
                fix2 = value >> 16;
                fix3 = value >> 8;
                fix4 = value;
                gbaRAM.VideoRAM[location + 3] = fix1;
                gbaRAM.VideoRAM[location + 2] = fix2;
                gbaRAM.VideoRAM[location + 1] = fix3;
                gbaRAM.VideoRAM[location] = fix4;
            }
        break;

        case 0x07000000 ... 0x070003FF:
            location -= 0x07000000;
            if(writeMode == 0)
            {
                gbaRAM.ObjectRAM[location] = value;
            }
            if(writeMode == 1)
            {
                fix1 = value >> 8;
                fix2 = value;
                gbaRAM.ObjectRAM[location + 1] = fix1;
                gbaRAM.ObjectRAM[location] = fix2;
            }
            if(writeMode == 2)
            {
                fix1 = value >> 24;
                fix2 = value >> 16;
                fix3 = value >> 8;
                fix4 = value;
                gbaRAM.ObjectRAM[location + 3] = fix1;
                gbaRAM.ObjectRAM[location + 2] = fix2;
                gbaRAM.ObjectRAM[location + 1] = fix3;
                gbaRAM.ObjectRAM[location] = fix4;
            }
        break;

        case 0x08000000 ... 0x0DFFFFFF:
            printf("WRITE TO ROM!  THIS SHOULDN'T HAPPEN!\n");
            breakpoint = true;
        break;

        default:
            opcodeError = true;
            printf("Unimplemented Write ADDR 0x%02X\n",location);
        break;
    }
}
