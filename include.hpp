#include <iostream>
#include <bitset>
#include <SDL2/SDL.h>
#include <bits/stdc++.h>

#include <GL/glew.h>
#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "imgui_memory_editor.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
SDL_Window* debugScreen;
SDL_Renderer* debugRenderer;
SDL_Event SDL_EVENT_HANDLING;
std::bitset<10> controlBuffer = 0x3FF;
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
uint8_t readablePalRam[3][0x20][0x10];  // RGB COLOR, ROW, COLLUM
uint8_t readablePalRam2[3][0x10];  // RGB COLOR, ROW, COLLUM
bool updateReadPal;
class nGBACPU
{
    public:
        uint32_t rNUM[16];
        std::bitset<32> cpsr;
        std::bitset<32> prevCpsr;
        std::bitset<32> spsr;
        std::bitset<32> spsr_svc;
        uint32_t R1314_svc[2];
        std::bitset<32> spsr_irq;
        uint32_t R1314_irq[2];
        uint64_t gbaCycles;
        uint8_t gbaScanline;
        uint16_t lcdControl;
        uint32_t dmaStartAddress[4];
        uint32_t dmaEndAddress[4];
        uint16_t dmaWordCount[4];
        std::bitset<16> dmaControl0;
        std::bitset<16> dmaControl1;
        std::bitset<16> dmaControl2;
        std::bitset<16> dmaControl3;
        uint16_t LY;
        bool IME;
        bool HALTCNT;
        bool currentlyHalted;
        uint16_t IE;
        uint32_t bgCNT[4];
        uint32_t soundBias;
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
void printDMARegs()
{
    printf("DMAstart0: 0x%X\n",gbaREG.dmaStartAddress[0]);
    printf("DMAend  0: 0x%X\n",  gbaREG.dmaEndAddress[0]);
    printf("DMAwordC0: 0x%X\n",   gbaREG.dmaWordCount[0]);
    std::cout<<"DMAcontr0: "<<gbaREG.dmaControl0<<std::endl;
    printf("DMAstart1: 0x%X\n",gbaREG.dmaStartAddress[1]);
    printf("DMAend  1: 0x%X\n",  gbaREG.dmaEndAddress[1]);
    printf("DMAwordC1: 0x%X\n",   gbaREG.dmaWordCount[1]);
    std::cout<<"DMAcontr1: "<<gbaREG.dmaControl1<<std::endl;
    printf("DMAstart2: 0x%X\n",gbaREG.dmaStartAddress[2]);
    printf("DMAend  2: 0x%X\n",  gbaREG.dmaEndAddress[2]);
    printf("DMAwordC2: 0x%X\n",   gbaREG.dmaWordCount[2]);
    std::cout<<"DMAcontr2: "<<gbaREG.dmaControl2<<std::endl;
    printf("DMAstart3: 0x%X\n",gbaREG.dmaStartAddress[3]);
    printf("DMAend  3: 0x%X\n",  gbaREG.dmaEndAddress[3]);
    printf("DMAwordC3: 0x%X\n",   gbaREG.dmaWordCount[3]);
    std::cout<<"DMAcontr3: "<<gbaREG.dmaControl3<<std::endl;
}
void memDump()
{
    printf("MEMDUMP!\n");
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
uint32_t onesCompilment(uint32_t value)
{
    rotateValNeed = value;
    uint32_t oneFixC = 31;
    while(oneFixC != 0xFFFFFFFF)
    {
        if(rotateValNeed[oneFixC] == 0)
        {

        }
    }
}
uint32_t rotateValue32(bool goRight, uint8_t rotateTimes, uint32_t valueNeed)
{
    rotateValNeed = valueNeed;
    if(goRight == true)
    {
        //if(rotateTimes == 0)
        //{
        //    return rotateValNeed.to_ulong();
        //}
        while(rotateTimes != 0x0)
        {
            backupBit2[0] = rotateValNeed[0];
            rotateValNeed = rotateValNeed >> 1;
            rotateValNeed[31] = backupBit2[0];
            rotateTimes--;
        }
    }
    if(goRight == false)
    {
        //if(rotateTimes == 0)
        //{
        //    return rotateValNeed.to_ulong();
        //}
        while(rotateTimes != 0x0)
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
uint8_t opRO;
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
const char* breakpointSettings[] = {"Equal To", "Less Than", "Less Than or Equal To", "Greater Than", "Greater Than or Equal To"};
static const char* breakpointCurrent = NULL;
bool breakpointEnabled;
uint32_t breakpointLocation;
std::bitset<32> ASLShift;
bool signValue;
uint32_t ASLvaluebyNum(uint32_t value2, uint8_t shiftAmount)
{
    ASLShift = value2;
    while(shiftAmount != 0)
    {
        signValue = ASLShift[31];
        ASLShift = ASLShift >> 1;
        ASLShift[31] = signValue;
        shiftAmount--;
    }
    return ASLShift.to_ulong();
}
uint32_t endRotate;
uint8_t shiftIMM;
std::bitset<5> shiftIMM2;
bool shiftOut;
uint32_t getLSLbyIMM()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    shiftIMM2 = currentOpcode >> 7;
    shiftIMM = shiftIMM2.to_ulong();
    if(shiftIMM == 0x0)
    {
        shiftIMM = 0x20;
    }
    endRotate = gbaREG.rNUM[opRM] << shiftIMM;
    shiftOut = ((gbaREG.rNUM[opRM] << (shiftIMM - 1)) & 0x80000000);
    return endRotate;
}
uint32_t getLSRbyIMM()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    shiftIMM2 = currentOpcode >> 7;
    shiftIMM = shiftIMM2.to_ulong();
    if(shiftIMM == 0x0)
    {
        shiftIMM = 0x20;
    }
    endRotate = gbaREG.rNUM[opRM] >> shiftIMM;
    shiftOut = ((gbaREG.rNUM[opRM] >> (shiftIMM - 1)) & 0x1);
    return endRotate;
}
uint32_t getRRbyIMM()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    shiftIMM2 = currentOpcode >> 7;
    shiftIMM = shiftIMM2.to_ulong();
    if(shiftIMM == 0x0)
    {
        shiftIMM = 0x20;
    }
    endRotate = rotateValue32(true,shiftIMM,gbaREG.rNUM[opRM]);
    shiftOut = (rotateValue32(true,shiftIMM - 1,gbaREG.rNUM[opRM]) & 0x1);
    return endRotate;
}
uint32_t getASRbyIMM()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    shiftIMM2 = currentOpcode >> 7;
    shiftIMM = shiftIMM2.to_ulong();
    if(shiftIMM == 0x0)
    {
        shiftIMM = 0x20;
    }
    endRotate = ASLvaluebyNum(gbaREG.rNUM[opRM],shiftIMM);
    shiftOut = (ASLvaluebyNum(gbaREG.rNUM[opRM],shiftIMM - 1) & 0x1);
    return endRotate;
}
uint32_t LSL(uint32_t value, uint8_t shiftAmount)
{
    while(shiftAmount != 0)
    {
        value = value << 1;
        shiftAmount--;
    }
    return value;
}
uint16_t return16;
uint32_t return32;
uint8_t fix1;
uint8_t fix2;
uint8_t fix3;
uint8_t fix4;
uint8_t randoRet0404;
std::bitset<16> IntReg;
std::bitset<16> IntVal;
uint8_t IntCountW;
uint16_t location16;
int location2;
uint32_t readMem(int readMode, int location)
{
    location2 = location;
    switch(readMode)
    {
        case 1:
            location = location & 0xFFFFFFFE;
        break;

        case 2:
            location = location & 0xFFFFFFFC;
        break;
    }
    switch(location)
    {
    case 0x0000 ... 0x4000:
        if(readMode == 0)
        {
            fix1 = gbaRAM.biosRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.biosRAM[location] << 8 |
                       gbaRAM.biosRAM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    case 0x02000000 ... 0x0203FFFF:
        location -= 0x02000000;
        if(readMode == 0)
        {
            fix1 = gbaRAM.onBoardWRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.onBoardWRAM[location] << 8 |
                       gbaRAM.onBoardWRAM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    case 0x03000000 ... 0x03007FFF:
        location -= 0x03000000;
        if(readMode == 0)
        {
            fix1 = gbaRAM.onChipWRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.onChipWRAM[location] << 8 |
                       gbaRAM.onChipWRAM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    case 0x03008000 ... 0x03FFFFFF:
        location = location & 0x7FFF;
        if(readMode == 0)
        {
            fix1 = gbaRAM.onChipWRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.onChipWRAM[location] << 8 |
                       gbaRAM.onChipWRAM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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

            case 0x04000006:
                gbaREG.LY++;
                if(gbaREG.LY % 256 == 0)
                {
                    gbaREG.LY = 0;
                }
                return gbaREG.LY;
            break;

            case 0x04000088:
                return gbaREG.soundBias;
            break;

            case 0x040000BA:
                return gbaREG.dmaControl0.to_ulong();
            break;

            case 0x040000DE:
                return gbaREG.dmaControl3.to_ulong();
            break;

            case 0x04000104:
                printf("TIMER1STUB!\n");
                return 0;
            break;

            case 0x04000130:
                return controlBuffer.to_ulong();
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
            fix1 = gbaRAM.Bg_ObjPalRam[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.Bg_ObjPalRam[location] << 8 |
                       gbaRAM.Bg_ObjPalRam[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    case 0x06000000 ... 0x06017FFF:
        location -= 0x06000000;
        if(readMode == 0)
        {
            fix1 = gbaRAM.VideoRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.VideoRAM[location] << 8 |
                       gbaRAM.VideoRAM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    case 0x07000000 ... 0x070003FF:
        location -= 0x06000000;
        if(readMode == 0)
        {
            fix1 = gbaRAM.ObjectRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.ObjectRAM[location] << 8 |
                       gbaRAM.ObjectRAM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    case 0x08000000 ... 0x0DFFFFFF:
        location -= 0x08000000;
        if(readMode == 0)
        {
            fix1 = gbaRAM.flashROM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.flashROM[location] << 8 |
                       gbaRAM.flashROM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    case 0x0E000000 ... 0x0E00FFFF:
        location -= 0x0E000000;
        if(readMode == 0)
        {
            fix1 = gbaRAM.SaveRAM[location];
        }
        if(readMode == 1)
        {
            return16 = gbaRAM.SaveRAM[location] << 8 |
                       gbaRAM.SaveRAM[location + 1];
            fix1 = return16 >> 8;
            fix2 = return16;
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
        }
    break;

    default:
        printf("Unknown Read Address 0x%X\n",location);
        opcodeError = true;
        return 0;
    break;
    }
    location = location2;
    switch(readMode)
    {
        case 0:
            fix4 = 0;
            fix3 = 0;
            fix2 = 0;
            return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
            return return32;
        break;

        case 1:
            fix4 = 0;
            fix3 = 0;
            switch(location & 0x1)
            {
                case 0:
                    return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
                    return return32;
                break;

                case 1:
                    return32 = fix4 << 24 | fix3 << 16 | fix1 << 8 | fix2;
                    return return32;
                break;
            }
        break;

        case 2:
            switch(location & 0x3)
            {
                case 0:
                    return32 = fix4 << 24 | fix3 << 16 | fix2 << 8 | fix1;
                    return return32;
                break;

                case 1:
                    return32 = fix1 << 24 | fix4 << 16 | fix3 << 8 | fix2;
                    return return32;
                break;

                case 2:
                    return32 = fix2 << 24 | fix1 << 16 | fix4 << 8 | fix3;
                    return return32;
                break;

                case 3:
                    return32 = fix3 << 24 | fix2 << 16 | fix1 << 8 | fix4;
                    return return32;
                break;
            }
        break;
    }
}
std::bitset<24> getLowerBits;
uint32_t trueLocation;
int writeMem(uint8_t writeMode, uint32_t location, uint32_t value)
{
    if(location < 0x4000)
    {
        printf("noWriteBIOS!\n");
        return 0;
    }
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

        case 0x03008000 ... 0x03FFFFFF:
            //printf("ASSUMING RAM MIRRORING!\n");
            getLowerBits = location;
            trueLocation = getLowerBits.to_ulong() % 0x8000;
            //printf("TRUE: 0x%X\n",trueLocation);
            if(writeMode == 0)
            {
                gbaRAM.onChipWRAM[trueLocation] = value;
            }
            if(writeMode == 1)
            {
                fix1 = value >> 8;
                fix2 = value;
                gbaRAM.onChipWRAM[trueLocation + 1] = fix1;
                gbaRAM.onChipWRAM[trueLocation] = fix2;
            }
            if(writeMode == 2)
            {
                fix1 = value >> 24;
                fix2 = value >> 16;
                fix3 = value >> 8;
                fix4 = value;
                gbaRAM.onChipWRAM[trueLocation + 3] = fix1;
                gbaRAM.onChipWRAM[trueLocation + 2] = fix2;
                gbaRAM.onChipWRAM[trueLocation + 1] = fix3;
                gbaRAM.onChipWRAM[trueLocation] = fix4;
            }
        break;

        case 0x04000000 ... 0x040003FE:
            switch(location)
            {
                case 0x04000000:
                    gbaREG.lcdControl = value;
                break;

                case 0x04000008:
                    breakpoint = true;
                    printf("VAL: 0x%X\n",value);
                    gbaREG.bgCNT[0] = value;
                break;

                case 0x0400000A:
                    gbaREG.bgCNT[1] = value;
                break;

                case 0x0400000C:
                    gbaREG.bgCNT[2] = value;
                break;

                case 0x0400000E:
                    gbaREG.bgCNT[3] = value;
                break;

                case 0x04000088:
                    gbaREG.soundBias = value;
                break;

                case 0x040000B0:
                    gbaREG.dmaStartAddress[0] = value;
                break;

                case 0x040000B4:
                    gbaREG.dmaEndAddress[0] = value;
                break;

                case 0x040000B8:
                    gbaREG.dmaWordCount[0] = value;
                    gbaREG.dmaControl0 = value >> 16;
                break;

                case 0x040000BC:
                    gbaREG.dmaStartAddress[1] = value;
                break;

                case 0x040000C0:
                    gbaREG.dmaEndAddress[1] = value;
                break;

                case 0x040000C4:
                    gbaREG.dmaWordCount[1] = value;
                    gbaREG.dmaControl1 = value >> 16;
                break;

                case 0x040000C8:
                    gbaREG.dmaStartAddress[2] = value;
                break;

                case 0x040000CC:
                    gbaREG.dmaEndAddress[2] = value;
                break;

                case 0x040000D0:
                    gbaREG.dmaWordCount[2] = value;
                    gbaREG.dmaControl2 = value >> 16;
                break;

                case 0x040000D4:
                    gbaREG.dmaStartAddress[3] = value;
                break;

                case 0x040000D8:
                    gbaREG.dmaEndAddress[3] = value;
                break;

                case 0x040000DC:
                    gbaREG.dmaWordCount[3] = value;
                    gbaREG.dmaControl3 = value >> 16;
                break;

                case 0x04000200:
                    printf("Software is currently attempting to enable Interrupts!\n");
                    IntReg = gbaREG.IE;
                    IntVal = value;
                    IntCountW = 0;
                    for(IntCountW = 0; IntCountW < 0x10; IntCountW++)
                    {
                        if(IntVal[IntCountW] == 1)
                        {
                            IntReg.flip(IntCountW);
                        }
                    }
                    gbaREG.IE = IntReg.to_ulong();
                break;

                case 0x04000202:
                    printf("IF WRITTEN TO!\n");
                    breakpoint = true;
                break;

                case 0x04000208:
                    gbaREG.IME = value;
                break;

                case 0x04000301:
                    gbaREG.HALTCNT = value >> 7;
                    gbaREG.currentlyHalted = true;
                break;

                default:
                    printf("Unimplemented IO reg 0x%X Written to!  Value was 0x%X!\n",location,value);
                break;
            }
        break;

        case 0x04000410:
            printf("I/O REG WITH NO PURPOSE 04000410!\n");
        break;

        case 0x05000000 ... 0x050003FF:
            updateReadPal = true;
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
                //printf("VWRITE!\n");
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
            //opcodeError = true;
        break;

        case 0x0E000000 ... 0x0E00FFFF:
            location -= 0x0E000000;
            if(writeMode == 0)
            {
                gbaRAM.SaveRAM[location] = value;
            }
            if(writeMode == 1)
            {
                printf("INVALID WRITE AMOUNT FOR SRAM!\n");
                fix1 = value >> 8;
                fix2 = value;
                gbaRAM.SaveRAM[location + 1] = fix1;
                gbaRAM.SaveRAM[location] = fix2;
            }
            if(writeMode == 2)
            {
                printf("INVALID WRITE AMOUNT FOR SRAM!\n");
                fix1 = value >> 24;
                fix2 = value >> 16;
                fix3 = value >> 8;
                fix4 = value;
                gbaRAM.SaveRAM[location + 3] = fix1;
                gbaRAM.SaveRAM[location + 2] = fix2;
                gbaRAM.SaveRAM[location + 1] = fix3;
                gbaRAM.SaveRAM[location] = fix4;
            }
        break;

        default:
            opcodeError = true;
            printf("Unimplemented Write ADDR 0x%02X\n",location);
        break;
    }
    if(location >= 0x040000B0 && location <= 0x040000E0)
    {
        //breakpoint = true;
        //printf("DMA REGISTERS WERE WRITTEN TO!\n");
        //printDMARegs();
    }
}
bool allowRun = false;
bool dontDisplayError = true;
uint64_t opcodesRan;
void resetEMU()
{
            opcodesRan = 0;
            gbaREG.rNUM[0] = 0;
            gbaREG.rNUM[1] = 0;
            gbaREG.rNUM[2] = 0;
            gbaREG.rNUM[3] = 0;
            gbaREG.rNUM[4] = 0;
            gbaREG.rNUM[5] = 0;
            gbaREG.rNUM[6] = 0;
            gbaREG.rNUM[7] = 0;
            gbaREG.rNUM[8] = 0;
            gbaREG.rNUM[9] = 0;
            gbaREG.rNUM[10] = 0;
            gbaREG.rNUM[11] = 0;
            gbaREG.rNUM[12] = 0;
            int counter22 = 0;
            while(counter22 != 0x40000)
            {
                gbaRAM.onBoardWRAM[counter22] = 0;
                counter22++;
            }
            counter22 = 0;
            while(counter22 != 0x8000)
            {
                gbaRAM.onChipWRAM[counter22] = 0;
                counter22++;
            }
            counter22 = 0;
            while(counter22 != 0x400)
            {
                gbaRAM.Bg_ObjPalRam[counter22] = 0;
                counter22++;
            }
            counter22 = 0;
            while(counter22 != 0x18000)
            {
                gbaRAM.VideoRAM[counter22] = 0;
                counter22++;
            }
            counter22 = 0;
            while(counter22 != 0x400)
            {
                gbaRAM.VideoRAM[counter22] = 0;
                counter22++;
            }
            counter22 = 0;
            while(counter22 != 0x10000)
            {
                gbaRAM.VideoRAM[counter22] = 0;
                counter22++;
            }
            counter22 = 0;
            gbaREG.rNUM[13] = 0x03007F00;
            gbaREG.rNUM[14] = 0x08000000;
            gbaREG.R1314_svc[0] = 0x03007FE0;
            gbaREG.R1314_svc[1] = 0x08000000;
            gbaREG.R1314_irq[0] = 0x03007F00;
            gbaREG.R1314_irq[1] = 0x08000000;
            gbaREG.rNUM[15] = gbaREG.rNUM[14];
            gbaREG.cpsr = 0x1F;
            gbaREG.spsr = 0x10;
            allowRun = true;
            opcodeError = false;
            dontDisplayError = true;
}

bool breakpointOpcodeA;
uint32_t opcodeRanBreak;
int handleBreakpoint()
{
    if(opcodeRanBreak == opcodesRan && breakpointOpcodeA == true)
    {
        breakpoint = true;
    }
    if(breakpointEnabled != true)
    {
        return 0;
    }
    if(gbaREG.rNUM[15] == breakpointLocation && breakpointCurrent == "Equal To")
    {
        breakpoint = true;
    }
    if(gbaREG.rNUM[15] < breakpointLocation && breakpointCurrent == "Less Than")
    {
        breakpoint = true;
    }
    if(gbaREG.rNUM[15] <= breakpointLocation && breakpointCurrent == "Less Than or Equal To")
    {
        breakpoint = true;
    }
    if(gbaREG.rNUM[15] > breakpointLocation && breakpointCurrent == "Greater Than")
    {
        breakpoint = true;
    }
    if(gbaREG.rNUM[15] >= breakpointLocation && breakpointCurrent == "Greater Than or Equal To")
    {
        breakpoint = true;
    }
}
// Some Dear ImGUI Stuff
const char* glsl_version = "#version 130";
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
SDL_GLContext gl_context;
int initFunc()
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    debugScreen = SDL_CreateWindow("nopGBA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, window_flags);
    gl_context = SDL_GL_CreateContext(debugScreen);
    SDL_GL_MakeCurrent(debugScreen, gl_context);
    SDL_GL_SetSwapInterval(1);

    glewInit();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplSDL2_InitForOpenGL(debugScreen,gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

GLuint screenTexGL;
GLuint palTexGL;
GLuint tiles1TexGL;
GLuint BG0TexGL;
GLuint BG1TexGL;
GLuint BG2TexGL;
GLuint BG3TexGL;
std::string fileNamePath;
const char* romName;
int romSize;
static MemoryEditor mem_edit_bios;

bool displayRAMView;
bool displayREGView;
bool displayCPSRView;
bool displayExtraView;
bool displayBreakpointView;
bool displayDisplayView;
bool displayDisplayView2;
bool displayIntRegs;
bool displayBIOSRam;
bool displayOBRam;
bool displayOCRam;
bool displayPALRAM;
bool displayVRAM;
bool displayOBJRAM;
bool displayROM;
bool displaysaveRAM;
bool displayTiles1;
bool swapPal;
bool displayBG0;
bool displayBG1;
bool displayBG2;
uint32_t opcodesPerFrame = 30000;


bool sFlag;
bool zFlag;
bool cFlag;
bool vFlag;
bool irqFlag;
bool fiqFlag;
bool thumbMode;
bool stateBit;
uint8_t curMode;
std::bitset<5> curModeBit;
uint32_t screenBG0Texture[248][248];
uint32_t screenBG1Texture[248][248];
uint32_t screenBG2Texture[248][248];
uint32_t screenBG3Texture[248][248];
void handleDebugWindow()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(debugScreen);
    ImGui::NewFrame();

    ImGui::Begin("Screen");
    ImGui::Image((void*)(intptr_t)screenTexGL,ImVec2(240,160));
    ImGui::End();

    if(displayBG0 == true)
    {
        glBindTexture(GL_TEXTURE_2D, BG0TexGL);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,248,248,0,GL_RGBA,GL_UNSIGNED_BYTE,*screenBG0Texture);
        glGenerateMipmap(GL_TEXTURE_2D);
        ImGui::Begin("BG0");
        ImGui::Image((void*)(intptr_t)BG0TexGL,ImVec2(248,248));
        ImGui::End();

    }

    if(displayBG1 == true)
    {
        glBindTexture(GL_TEXTURE_2D, BG1TexGL);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,248,248,0,GL_RGBA,GL_UNSIGNED_BYTE,*screenBG1Texture);
        glGenerateMipmap(GL_TEXTURE_2D);
        ImGui::Begin("BG1");
        ImGui::Image((void*)(intptr_t)BG1TexGL,ImVec2(248,248));
        ImGui::End();
    }

    if(displayBG2 == true)
    {
        glBindTexture(GL_TEXTURE_2D, BG2TexGL);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,248,248,0,GL_RGBA,GL_UNSIGNED_BYTE,*screenBG2Texture);
        glGenerateMipmap(GL_TEXTURE_2D);
        ImGui::Begin("BG2");
        ImGui::Image((void*)(intptr_t)BG2TexGL,ImVec2(248,248));
        ImGui::End();
    }


    ImGui::Begin("Options");
    if(ImGui::Button("Open ROM File"))
    {
        igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey","Choose File", ".gba,.bin",".");
    }
    if(igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
    {
        if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
        {
            fileNamePath = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
            std::cout<<"fileNamePath: "<<fileNamePath<<std::endl;
            romName = fileNamePath.c_str();
            std::cout<<"ResultROM: "<<romName<<std::endl;
            FILE* rom = fopen(romName,"r");
            fseek (rom , 0 , SEEK_END);
            romSize = ftell(rom);
            rewind(rom);
            fread(gbaRAM.flashROM,romSize,1,rom);
            fclose(rom);
            FILE* bios = fopen("bios/gba.rom","r");
            if(bios == NULL)
            {
                printf("WARNING!  BIOS NOT DETECTED! GAMES WILL LIKELY NOT WORK!\nBios must go into bios/gba.rom!\n");
                fclose(bios);
            }
            if(bios != NULL)
            {
                fseek (bios , 0 , SEEK_END);
                romSize = ftell(bios);
                rewind(bios);
                fread(gbaRAM.biosRAM,romSize,1,bios);
                fclose(bios);
            }
            resetEMU();

        }
        igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
    }
    if(ImGui::Button("Reset"))
    {
        resetEMU();
    }
    if(ImGui::Button("Boot Bios"))
    {
        FILE* bios = fopen("bios/gba.rom","r");
            if(bios == NULL)
            {
                printf("WARNING!  BIOS NOT DETECTED! GAMES WILL LIKELY NOT WORK!\nBios must go into bios/gba.rom!\n");
                fclose(bios);
            }
            if(bios != NULL)
            {
                fseek (bios , 0 , SEEK_END);
                romSize = ftell(bios);
                rewind(bios);
                fread(gbaRAM.biosRAM,romSize,1,bios);
                fclose(bios);
            }
        resetEMU();
        gbaREG.rNUM[15] = 0;
        gbaREG.rNUM[14] = 0;
    }
    ImGui::Checkbox("Display RAM Viewer",&displayRAMView);
    ImGui::Checkbox("Display Register Viewer",&displayREGView);
    ImGui::Checkbox("Display Debug Screen Menu",&displayDisplayView2);
    ImGui::Checkbox("Display CPSR",&displayCPSRView);
    ImGui::Checkbox("Display Interrupt Register Viewer",&displayIntRegs);
    ImGui::Checkbox("Display Extra Values",&displayExtraView);
    ImGui::Checkbox("Display Breakpoint Options",&displayBreakpointView);
    ImGui::End();
    if(displayDisplayView2 == true)
    {
        ImGui::Begin("Display Options");
        ImGui::Checkbox("Display Pallete Viewer",&displayDisplayView);
        ImGui::Checkbox("Display Tiles 1 Viewer",&displayTiles1);
        ImGui::Checkbox("Display Background 0 Viewer",&displayBG0);
        ImGui::Checkbox("Display Background 1 Viewer",&displayBG1);
        ImGui::Checkbox("Display Background 2 Viewer",&displayBG2);
        ImGui::InputScalar("Opcodes Per Frame:", ImGuiDataType_U32, &opcodesPerFrame, NULL, NULL, "%i", ImGuiInputTextFlags_CharsDecimal);
        ImGui::InputScalar("BG0:", ImGuiDataType_U32, &gbaREG.bgCNT[0], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::End();
        if(displayDisplayView == true)
        {
            ImGui::Begin("Pal");
            ImGui::Image((void*)(intptr_t)palTexGL,ImVec2(0x1E,0x1E * 2));
            ImGui::End();
        }
        if(displayTiles1 == true)
        {
            ImGui::Begin("Tiles1");
            ImGui::Checkbox("Swap to Default Pallete",&swapPal);
            ImGui::Image((void*)(intptr_t)tiles1TexGL,ImVec2(0x3E0,0x3E0));
            ImGui::End();
        }
    }
    if(displayREGView == true)
    {
        ImGui::Begin("Registers");
        ImGui::InputScalar("R0:", ImGuiDataType_U32, &gbaREG.rNUM[0], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R1:", ImGuiDataType_U32, &gbaREG.rNUM[1], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R2:", ImGuiDataType_U32, &gbaREG.rNUM[2], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R3:", ImGuiDataType_U32, &gbaREG.rNUM[3], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R4:", ImGuiDataType_U32, &gbaREG.rNUM[4], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R5:", ImGuiDataType_U32, &gbaREG.rNUM[5], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R6:", ImGuiDataType_U32, &gbaREG.rNUM[6], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R7:", ImGuiDataType_U32, &gbaREG.rNUM[7], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R8:", ImGuiDataType_U32, &gbaREG.rNUM[8], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R9:", ImGuiDataType_U32, &gbaREG.rNUM[9], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R10:", ImGuiDataType_U32, &gbaREG.rNUM[10], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R11:", ImGuiDataType_U32, &gbaREG.rNUM[11], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R12:", ImGuiDataType_U32, &gbaREG.rNUM[12], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R13:", ImGuiDataType_U32, &gbaREG.rNUM[13], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R14:", ImGuiDataType_U32, &gbaREG.rNUM[14], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("R15:", ImGuiDataType_U32, &gbaREG.rNUM[15], NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::End();
    }
    if(displayCPSRView == true)
    {
        ImGui::Begin("CPSR Viewer");
        ImGui::Text("THESE VALUES CANNOT BE EDITED!");
        sFlag = gbaREG.cpsr[31];
        zFlag = gbaREG.cpsr[30];
        cFlag = gbaREG.cpsr[29];
        vFlag = gbaREG.cpsr[28];
        irqFlag = gbaREG.cpsr[7];
        fiqFlag = gbaREG.cpsr[6];
        thumbMode = gbaREG.cpsr[5];
        curModeBit = gbaREG.cpsr.to_ulong();
        curMode = curModeBit.to_ulong();
        ImGui::Checkbox("S",&sFlag);
        ImGui::Checkbox("Z",&zFlag);
        ImGui::Checkbox("C",&cFlag);
        ImGui::Checkbox("V",&vFlag);
        ImGui::Checkbox("IRQ Flag",&irqFlag);
        ImGui::Checkbox("FIQ FLAG",&fiqFlag);
        ImGui::Checkbox("Thumb Mode",&thumbMode);
        ImGui::InputScalar("Current Mode", ImGuiDataType_U8, &curMode, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::End();
    }
    if(displayIntRegs == true)
    {
        ImGui::Begin("Interrupt Registers");
        ImGui::Checkbox("IME",&gbaREG.IME);
        ImGui::InputScalar("IE", ImGuiDataType_U16, &gbaREG.IE, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::End();
    }
    if(displayRAMView == true)
    {
        ImGui::Begin("RAM View Options");
        ImGui::Checkbox("Display BIOS RAM Viewer",&displayBIOSRam);
        ImGui::Checkbox("Display Onboard RAM Viewer",&displayOBRam);
        ImGui::Checkbox("Display Onchip RAM Viewer",&displayOCRam);
        ImGui::Checkbox("Display Palette RAM Viewer",&displayPALRAM);
        ImGui::Checkbox("Display Video RAM Viewer",&displayVRAM);
        ImGui::Checkbox("Display Object RAM Viewer",&displayOBJRAM);
        ImGui::Checkbox("Display ROM Viewer",&displayROM);
        ImGui::Checkbox("Display Save RAM Viewer",&displaysaveRAM);
        ImGui::End();
        if(displayBIOSRam == true)
        {
            ImGui::Begin("BIOS RAM");
            mem_edit_bios.DrawContents(gbaRAM.biosRAM,sizeof(gbaRAM.biosRAM), 0);
            ImGui::End();
        }
        if(displayOBRam == true)
        {
            ImGui::Begin("Onboard RAM");
            mem_edit_bios.DrawContents(gbaRAM.onBoardWRAM,sizeof(gbaRAM.onBoardWRAM), 0x02000000);
            ImGui::End();
        }
        if(displayOCRam == true)
        {
            ImGui::Begin("Onchip RAM");
            mem_edit_bios.DrawContents(gbaRAM.onChipWRAM,sizeof(gbaRAM.onChipWRAM), 0x03000000);
            ImGui::End();
        }
        if(displayPALRAM == true)
        {
            ImGui::Begin("Palette RAM");
            mem_edit_bios.DrawContents(gbaRAM.Bg_ObjPalRam,sizeof(gbaRAM.Bg_ObjPalRam), 0x05000000);
            ImGui::End();
        }
        if(displayVRAM == true)
        {
            ImGui::Begin("Palette RAM");
            mem_edit_bios.DrawContents(gbaRAM.VideoRAM,sizeof(gbaRAM.VideoRAM), 0x06000000);
            ImGui::End();
        }
        if(displayOBJRAM == true)
        {
            ImGui::Begin("Palette RAM");
            mem_edit_bios.DrawContents(gbaRAM.ObjectRAM,sizeof(gbaRAM.ObjectRAM), 0x07000000);
            ImGui::End();
        }
        if(displayROM == true)
        {
            ImGui::Begin("ROM");
            mem_edit_bios.DrawContents(gbaRAM.flashROM,sizeof(gbaRAM.flashROM), 0x08000000);
            ImGui::End();
        }
        if(displaysaveRAM == true)
        {
            ImGui::Begin("Palette RAM");
            mem_edit_bios.DrawContents(gbaRAM.SaveRAM,sizeof(gbaRAM.SaveRAM), 0x0E000000);
            ImGui::End();
        }
    }
    if(displayExtraView == true)
    {
        ImGui::Begin("Extra Info");
        ImGui::InputScalar("Opcodes Ran", ImGuiDataType_U64, &opcodesRan, NULL, NULL, "%i", ImGuiInputTextFlags_CharsDecimal);
        ImGui::InputScalar("Last Arm Opcode", ImGuiDataType_U32, &currentOpcode, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("Last Thumb Opcode", ImGuiDataType_U32, &currentThumbOpcode, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("Cycles", ImGuiDataType_U64, &gbaREG.gbaCycles, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("Scanline", ImGuiDataType_U8, &gbaREG.gbaScanline, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("LCD Control", ImGuiDataType_U8, &gbaREG.lcdControl, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::End();
    }
    if(displayBreakpointView == true)
    {
        ImGui::Begin("Breakpoint Settings");
        ImGui::Checkbox("Enabled",&breakpointEnabled);
        if(ImGui::BeginCombo("Combo", breakpointCurrent))
        {
            for (int n = 0; n < IM_ARRAYSIZE(breakpointSettings); n++)
            {
                bool breakpointIsSelect = (breakpointCurrent == breakpointSettings[n]);
                if(ImGui::Selectable(breakpointSettings[n], breakpointIsSelect))
                {
                    breakpointCurrent = breakpointSettings[n];
                }
                if(breakpointIsSelect)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::InputScalar("PC Location:", ImGuiDataType_U32, &breakpointLocation, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::Checkbox("2nd Enabled",&breakpointOpcodeA);
        ImGui::InputScalar("Opcodes Ran:", ImGuiDataType_U32, &opcodeRanBreak, NULL, NULL, "%i", ImGuiInputTextFlags_CharsDecimal);
        ImGui::End();
    }
    if(opcodeError == true && dontDisplayError == false)
    {
        ImGui::Begin("ERROR!");
        ImGui::Text("An Opcode Error has occured!");
        ImGui::Text("See the console for more details.");
        dontDisplayError = ImGui::Button("Ok");
        ImGui::End();
    }
    if(breakpoint == true)
    {
        ImGui::Begin("BREAKPOINT HIT");
        ImGui::Text("What would you like to do?");
        if(ImGui::Button("Step to next opcode"))
        {
            allowRun = true;
        }
        if(ImGui::Button("Continue Running"))
        {
            allowRun = true;
            breakpoint = false;
        }
        if(ImGui::Button("Memdump"))
        {
            memDump();
        }
        ImGui::End();
    }


    ImGui::Render();
    glViewport(0, 0, 800, 600);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(debugScreen);
}
// Readable Pal Ram Update Func
void updateReadPalF()
{
    updateReadPal = false;
    uint8_t pRow, pCol;
    uint32_t palReadLocate;
    pRow = 0;
    pCol = 0;
    while (pRow != 0x10)
    {
        while(pCol != 0x10)
        {
            palReadLocate = (pRow * 0x20) + (pCol * 0x2);
            readablePalRam[0][pRow][pCol] = ((readMem(1, 0x05000000 + palReadLocate ) & 0x1F) << 3);
            readablePalRam[1][pRow][pCol] = (((readMem(1, 0x05000000 + palReadLocate ) >> 5) & 0x1F) << 3);
            readablePalRam[2][pRow][pCol] = (((readMem(1, 0x05000000 + palReadLocate ) >> 10) & 0x1F) << 3);
            pCol++;
        }
        pRow++;
    }
}
int handleMainGUI()
{
    handleDebugWindow();
    return 0;
}
