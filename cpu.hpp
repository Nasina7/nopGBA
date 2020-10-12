#include "cpuTHUMB.hpp"
/*
XXXX001XXXXXXXXXXXXXXXXXXXXXXXXX Data Processing
XXXX000000??XXXXXXXXXXXX1001XXXX Multiply
XXXX00001???XXXXXXXXXXXX1001XXXX Multiply Long
XXXX00010?00XXXXXXXX00001001XXXX Single Data Swap
XXXX000100101111111111110001XXXX Branch and Exchange
XXXX000??0??XXXXXXXX00001??1XXXX Halfword Data Transfer: Register Offset
XXXX000??1??XXXXXXXXXXXX1??1XXXX Halfword Data Transfer: Immediate Offset
XXXX01??????XXXXXXXXXXXXXXXXXXXX Single Data Transfer
XXXX011XXXXXXXXXXXXXXXXXXXX1XXXX Undefined
XXXX100?????XXXXXXXXXXXXXXXXXXXX Block Data Transfer
XXXX101?XXXXXXXXXXXXXXXXXXXXXXXX Branch
XXXX110?????XXXXXXXXXXXXXXXXXXXX Coprocessor Data Transfer
XXXX1110XXXXXXXXXXXXXXXXXXX0XXXX Coprocessor Data Operation
XXXX1110XXXXXXXXXXXXXXXXXXX1XXXX Coprocessor Register Transfer
XXXX1111XXXXXXXXXXXXXXXXXXXXXXXX Software Interrupt
*/
int loadROM()
{
    printf("Welcome to nopGBA!\nEnter Rom Name: ");
    //scanf("%s",romName);
    FILE* rom = fopen(romName,"r");
    if(rom == NULL)
    {
        printf("Error! Rom not found!\n");
        fclose(rom);
        loadROM();
        return 0;
    }
    fseek (rom , 0 , SEEK_END);
    romSize = ftell(rom);
    rewind(rom);
    if(romSize > 0x6000000)
    {
        printf("ROM SIZE ERROR!\n");
        loadROM();
    }
    fread(gbaRAM.flashROM,romSize,1,rom);
    fclose(rom);
    FILE* bios = fopen("bios/gba.rom","r");
    if(bios == NULL)
    {
        printf("WARNING!  BIOS NOT DETECTED! GAMES WILL LIKELY NOT WORK!\nBios must go into bios/gba.rom!\n");
        fclose(bios);
        return 1;
    }
    fseek (bios , 0 , SEEK_END);
    romSize = ftell(bios);
    rewind(bios);
    fread(gbaRAM.biosRAM,romSize,1,bios);
    fclose(bios);
}
uint32_t startAddrDMA;
uint32_t endAddrDMA;
uint16_t wordCountDMA;
std::bitset<16> dmaControl;
uint8_t dmaWriteMode;
uint8_t addrControl;
void doDMATransfer()
{
    dmaWriteMode = dmaControl[10] + 1;
    while(wordCountDMA != 0)
    {
        writeMem(dmaWriteMode,endAddrDMA,readMem(dmaWriteMode,startAddrDMA));
        wordCountDMA--;
        addrControl = dmaControl[6] << 1 | dmaControl[5];
        switch(addrControl)
        {
            case 0:
                endAddrDMA++;
            break;

            case 1:
                endAddrDMA--;
            break;

            case 2:
                // Don't change end address
            break;

            default:
                printf("DMA WARNING 3!\n");
            break;
        }
        addrControl = dmaControl[8] << 1 | dmaControl[7];
        switch(addrControl)
        {
            case 0:
                startAddrDMA++;
            break;

            case 1:
                startAddrDMA--;
            break;

            case 2:
                // Don't change end address
            break;

            default:
                printf("DMA WARNING 3!\n");
            break;
        }
    }
    printf("DMA FINISH!\n");
}
uint8_t currentMode;
uint8_t prevMode;
std::bitset<5> checkMode;
uint32_t swapVar;
uint8_t prevModeSwitch = 0x1F;
void checkModeSwitch()
{
    checkMode = gbaREG.cpsr.to_ulong();
    currentMode = checkMode.to_ulong();
    checkMode = gbaREG.prevCpsr.to_ulong();
    prevMode = checkMode.to_ulong();
    if(currentMode != prevMode)
    {
        printf("PrevMode: 0x%X\n",prevModeSwitch);
        printf("CurrentMode: 0x%X\n",currentMode);
        switch(currentMode)
        {

            case 0x12:
                switch(prevModeSwitch)
                {
                    case 0x13:
                        swapVar = gbaREG.R1314_svc[0];
                        gbaREG.R1314_svc[0] = gbaREG.R1314_irq[0];
                        gbaREG.R1314_irq[0] = swapVar;

                        swapVar = gbaREG.R1314_svc[1];
                        gbaREG.R1314_svc[1] = gbaREG.R1314_irq[1];
                        gbaREG.R1314_irq[1] = swapVar;

                        swapVar = gbaREG.spsr_svc.to_ulong();
                        gbaREG.spsr_svc = gbaREG.spsr_irq.to_ulong();
                        gbaREG.spsr_irq = swapVar;
                    break;

                    case 0x1F:
                        printf("MR13irq: 0x%X\n",gbaREG.R1314_irq[0]);
                        printf("MR14irq: 0x%X\n",gbaREG.R1314_irq[1]);
                        swapVar = gbaREG.rNUM[13];
                        gbaREG.rNUM[13] = gbaREG.R1314_irq[0];
                        gbaREG.R1314_irq[0] = swapVar;

                        swapVar = gbaREG.rNUM[14];
                        gbaREG.rNUM[14] = gbaREG.R1314_irq[1];
                        gbaREG.R1314_irq[1] = swapVar;

                        swapVar = gbaREG.spsr.to_ulong();
                        gbaREG.spsr = gbaREG.spsr_irq.to_ulong();
                        gbaREG.spsr_irq = swapVar;
                    break;
                }
                prevModeSwitch = 0x12;
                //breakpoint = true;
            break;

            case 0x13:
                switch(prevModeSwitch)
                {
                    case 0x12:
                        swapVar = gbaREG.R1314_svc[0];
                        gbaREG.R1314_svc[0] = gbaREG.R1314_irq[0];
                        gbaREG.R1314_irq[0] = swapVar;

                        swapVar = gbaREG.R1314_svc[1];
                        gbaREG.R1314_svc[1] = gbaREG.R1314_irq[1];
                        gbaREG.R1314_irq[1] = swapVar;

                        swapVar = gbaREG.spsr_svc.to_ulong();
                        gbaREG.spsr_svc = gbaREG.spsr_irq.to_ulong();
                        gbaREG.spsr_irq = swapVar;
                    break;

                    case 0x1F:
                        swapVar = gbaREG.rNUM[13];
                        gbaREG.rNUM[13] = gbaREG.R1314_svc[0];
                        gbaREG.R1314_svc[0] = swapVar;

                        swapVar = gbaREG.rNUM[14];
                        gbaREG.rNUM[14] = gbaREG.R1314_svc[1];
                        gbaREG.R1314_svc[1] = swapVar;

                        swapVar = gbaREG.spsr.to_ulong();
                        gbaREG.spsr = gbaREG.spsr_svc.to_ulong();
                        gbaREG.spsr_svc = swapVar;
                    break;
                }
                prevModeSwitch = 0x13;
            break;

            case 0x1F:
                switch(prevModeSwitch)
                {
                    case 0x12:
                        swapVar = gbaREG.rNUM[13];
                        gbaREG.rNUM[13] = gbaREG.R1314_irq[0];
                        gbaREG.R1314_irq[0] = swapVar;

                        swapVar = gbaREG.rNUM[14];
                        gbaREG.rNUM[14] = gbaREG.R1314_irq[1];
                        gbaREG.R1314_irq[1] = swapVar;

                        swapVar = gbaREG.spsr.to_ulong();
                        gbaREG.spsr = gbaREG.spsr_irq.to_ulong();
                        gbaREG.spsr_irq = swapVar;
                    break;

                    case 0x13:
                        swapVar = gbaREG.rNUM[13];
                        gbaREG.rNUM[13] = gbaREG.R1314_svc[0];
                        gbaREG.R1314_svc[0] = swapVar;

                        swapVar = gbaREG.rNUM[14];
                        gbaREG.rNUM[14] = gbaREG.R1314_svc[1];
                        gbaREG.R1314_svc[1] = swapVar;

                        swapVar = gbaREG.spsr.to_ulong();
                        gbaREG.spsr = gbaREG.spsr_svc.to_ulong();
                        gbaREG.spsr_svc = swapVar;
                    break;
                }
                prevModeSwitch = 0x1F;
            break;
        }
        //printf("Mode Switch to: 0x%X\n",prevModeSwitch);
    }
}
void handleDMA()
{
    if(gbaREG.dmaControl0[15] == 1)
    {
        //printf("DMA 0 WAS ENABLED!\n");
        startAddrDMA = gbaREG.dmaStartAddress[0];
        endAddrDMA = gbaREG.dmaEndAddress[0];
        wordCountDMA = gbaREG.dmaWordCount[0];
        //printf("WORD: 0x%X\n",wordCountDMA);
        dmaControl = gbaREG.dmaControl0.to_ulong();
        if(gbaREG.dmaControl0[13] == 0 && gbaREG.dmaControl0[12] == 0 && wordCountDMA != 0)
        {
            // Immediately
            doDMATransfer();
            gbaREG.dmaControl0[15] = 0;
        }
        if(gbaREG.dmaControl0[13] != 0 || gbaREG.dmaControl0[12] != 0)
        {
            printf("DMA TIMING IS NOT IMPLEMENTED!\n");
        }
    }
    if(gbaREG.dmaControl1[15] == 1)
    {
        //printf("DMA 1 WAS ENABLED!\n");
    }
    if(gbaREG.dmaControl2[15] == 1)
    {
        printf("DMA 2 WAS ENABLED!\n");
    }
    if(gbaREG.dmaControl3[15] == 1)
    {
        //printf("DMA 3 WAS ENABLED!\n");
        startAddrDMA = gbaREG.dmaStartAddress[3];
        endAddrDMA = gbaREG.dmaEndAddress[3];
        wordCountDMA = gbaREG.dmaWordCount[3];
        //printf("WORD: 0x%X\n",wordCountDMA);
        dmaControl = gbaREG.dmaControl3.to_ulong();
        if(gbaREG.dmaControl3[13] == 0 && gbaREG.dmaControl3[12] == 0 && wordCountDMA != 0)
        {
            // Immediately
            doDMATransfer();
            gbaREG.dmaControl3[15] = 0;
        }
        if(gbaREG.dmaControl3[13] != 0 || gbaREG.dmaControl3[12] != 0)
        {
            printf("DMA TIMING IS NOT IMPLEMENTED!\n");
        }
    }
}
std::bitset<32> B32ImmedGet;
std::bitset<8> immed8;
std::bitset<4> rotate_imm;
std::bitset<3> getB6tB4;
uint16_t rotateImmMult2;
uint16_t rotateIMMM;
uint32_t get32Bimmediate()
{
    immed8 = currentOpcode;
    B32ImmedGet = immed8.to_ulong();
    rotate_imm = currentOpcode >> 8;
    rotateIMMM = rotate_imm.to_ulong();
    rotateIMMM = rotateIMMM * 2;
    rotateImmMult2 = (rotateIMMM);
    printf("rotIMM: 0x%X\n",rotateImmMult2);
    //if(rotateImmMult2 == 0)
    //{
    //    return B32ImmedGet.to_ulong();
    //}
    while(rotateImmMult2 != 0x0)
    {
        immed8[0] = B32ImmedGet[0];
        B32ImmedGet = B32ImmedGet >> 1;
        B32ImmedGet[32] = immed8[0];
        rotateImmMult2--;
    }
    return B32ImmedGet.to_ulong();
}
uint8_t op001code;
std::bitset<32> rotation8mov;
std::bitset<4> rotatebitAmount;
bool rotation8back;
uint8_t rotationMovAmount;
uint8_t rMovAmBack;
uint8_t movIntoReg;
void movOP()
{
    //opcodeError = true;
    rotation8mov = currentOpcode << 24;
    rotation8mov = rotation8mov >> 24;
    rotatebitAmount = currentOpcode >> 8;
    rotationMovAmount = rotatebitAmount.to_ulong();
    rotationMovAmount = rotationMovAmount * 2;
    rMovAmBack = rotationMovAmount;
    while(rotationMovAmount != 0)
    {
        rotation8back = rotation8mov[0];
        rotation8mov = rotation8mov >> 1;
        rotation8mov[31] = rotation8back;
        rotationMovAmount--;
    }
    rotatebitAmount = currentOpcode >> 12; // Using rotate bit amount because it's val doesnt matter and I needed a 4bit bitset anyway
    movIntoReg = rotatebitAmount.to_ulong();
    gbaREG.rNUM[movIntoReg] = rotation8mov.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        //printf("Handle Flag Stub for 001-1101!\n");
    }
    if(rMovAmBack != 0)
    {
        //printf("Does Carry 001-1101 Work?\n");
        gbaREG.cpsr[29] = rotation8mov[31];
    }
    gbaREG.rNUM[15] += 4;
}
void movNOTOP()
{
    //opcodeError = true;
    rotation8mov = currentOpcode << 24;
    rotation8mov = rotation8mov >> 24;
    rotatebitAmount = currentOpcode >> 8;
    rotationMovAmount = rotatebitAmount.to_ulong();
    rotationMovAmount = rotationMovAmount * 2;
    rMovAmBack = rotationMovAmount;
    while(rotationMovAmount != 0)
    {
        rotation8back = rotation8mov[0];
        rotation8mov = rotation8mov >> 1;
        rotation8mov[31] = rotation8back;
        rotationMovAmount--;
    }
    rotatebitAmount = currentOpcode >> 12; // Using rotate bit amount because it's val doesnt matter and I needed a 4bit bitset anyway
    movIntoReg = rotatebitAmount.to_ulong();
    gbaREG.rNUM[movIntoReg] = ~rotation8mov.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        //printf("Handle Flag Stub for 001-1101!\n");
    }
    if(rMovAmBack != 0)
    {
        //printf("Does Carry 001-1101 Work?\n");
        gbaREG.cpsr[29] = rotation8mov[31];
    }
    gbaREG.rNUM[15] += 4;
}
void movOPREG()
{
    opRDET = currentOpcode;
    opRN = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN];
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        handleCarryFlag(0,0,1,0);
    }
    if(opRD != 0xF)
    {
        gbaREG.rNUM[15] += 4;
    }
    if(opRD == 0xF)
    {
        gbaREG.cpsr = gbaREG.spsr.to_ulong();
    }
}
void movNOTOPREG()
{
    opRDET = currentOpcode;
    opRN = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = ~gbaREG.rNUM[opRN];
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        handleCarryFlag(0,0,1,0);
    }
    gbaREG.rNUM[15] += 4;
}
std::bitset<5> offset52;
uint32_t shiftBitLSL;
void movOPLSLbyImmediate()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    offset52 = currentOpcode >> 7;
    offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = shiftBitLSL << offset5u;
    gbaREG.rNUM[opRD] = shiftBitLSL;
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        rotation8mov = gbaREG.rNUM[opRD];
        gbaREG.cpsr[29] = rotation8mov[31];
    }
    gbaREG.rNUM[15] += 4;
}
uint8_t low8Shift;
void movOPLSLbyRegister()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    opRDET = currentOpcode >> 8;
    opRS = opRDET.to_ulong();
    low8Shift = gbaREG.rNUM[opRS];

    ZOriginal = gbaREG.rNUM[opRD];
    //offset52 = currentOpcode >> 7;
    //offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = shiftBitLSL << low8Shift;
    gbaREG.rNUM[opRD] = shiftBitLSL;
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        rotation8mov = gbaREG.rNUM[opRD];
        gbaREG.cpsr[29] = rotation8mov[31];
    }
    gbaREG.rNUM[15] += 4;
}
void movOPASRbyRegister()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    opRDET = currentOpcode >> 8;
    opRS = opRDET.to_ulong();
    low8Shift = gbaREG.rNUM[opRS];
    ZOriginal = gbaREG.rNUM[opRD];

    /*
    offset52 = currentOpcode >> 7;
    offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = shiftBitLSL >> offset5u;
    */

    //shiftIMM2 = currentOpcode >> 7;
    //shiftIMM = shiftIMM2.to_ulong();

    gbaREG.rNUM[opRD] = ASLvaluebyNum(gbaREG.rNUM[opRM],low8Shift);
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        handleCarryFlag(0,0,1,0);
    }
    gbaREG.rNUM[15] += 4;
}
void movNOTOPLSLbyImmediate()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    offset52 = currentOpcode >> 7;
    offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = shiftBitLSL << offset5u;
    gbaREG.rNUM[opRD] = ~shiftBitLSL;
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        rotation8mov = gbaREG.rNUM[opRD];
        gbaREG.cpsr[29] = rotation8mov[31];
    }
    gbaREG.rNUM[15] += 4;
}
void movOPLSRbyImmediate()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    offset52 = currentOpcode >> 7;
    offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = shiftBitLSL >> offset5u;
    gbaREG.rNUM[opRD] = shiftBitLSL;
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        handleCarryFlag(0,0,1,0);
    }
    gbaREG.rNUM[15] += 4;
}
void movOPASRbyImmediate()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];

    /*
    offset52 = currentOpcode >> 7;
    offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = shiftBitLSL >> offset5u;
    */

    shiftIMM2 = currentOpcode >> 7;
    shiftIMM = shiftIMM2.to_ulong();

    gbaREG.rNUM[opRD] = ASLvaluebyNum(gbaREG.rNUM[opRM],shiftIMM);
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        handleCarryFlag(0,0,1,0);
    }
    gbaREG.rNUM[15] += 4;
}
void movOPRORbyImmediate()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    offset52 = currentOpcode >> 7;
    offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = rotateValue32(true,offset5u,shiftBitLSL);
    gbaREG.rNUM[opRD] = shiftBitLSL;
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        handleCarryFlag(0,0,1,0);
    }
    gbaREG.rNUM[15] += 4;
}
void movNOTOPLSRbyImmediate()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    offset52 = currentOpcode >> 7;
    offset5u = offset52.to_ulong();
    shiftBitLSL = gbaREG.rNUM[opRM];
    shiftBitLSL = shiftBitLSL >> offset5u;
    gbaREG.rNUM[opRD] = ~shiftBitLSL;
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
        handleCarryFlag(0,0,1,0);
    }
    gbaREG.rNUM[15] += 4;
}
uint8_t regToWriteCPSR;
std::bitset<32> valToWriteCPSR;
std::bitset<4> BB4MSR; // Used to get REG Number and to get Field Mask
std::bitset<1> r22;
void msrREGOP()
{
    BB4MSR = currentOpcode;
    regToWriteCPSR = BB4MSR.to_ulong();
    valToWriteCPSR = gbaREG.rNUM[regToWriteCPSR];
    //printf("Note, does SBO or SBZ do anything?");
    BB4MSR = currentOpcode >> 16;
    r22 = currentOpcode >> 22;
    if(r22[0] == 0)
    {
        if(BB4MSR[0] == 1)
        {
            gbaREG.cpsr[0] = valToWriteCPSR[0];
            gbaREG.cpsr[1] = valToWriteCPSR[1];
            gbaREG.cpsr[2] = valToWriteCPSR[2];
            gbaREG.cpsr[3] = valToWriteCPSR[3];
            gbaREG.cpsr[4] = valToWriteCPSR[4];
            gbaREG.cpsr[5] = valToWriteCPSR[5];
            gbaREG.cpsr[6] = valToWriteCPSR[6];
            gbaREG.cpsr[7] = valToWriteCPSR[7];
        }
        if(BB4MSR[1] == 1)
        {
            gbaREG.cpsr[8] = valToWriteCPSR[8];
            gbaREG.cpsr[9] = valToWriteCPSR[9];
            gbaREG.cpsr[10] = valToWriteCPSR[10];
            gbaREG.cpsr[11] = valToWriteCPSR[11];
            gbaREG.cpsr[12] = valToWriteCPSR[12];
            gbaREG.cpsr[13] = valToWriteCPSR[13];
            gbaREG.cpsr[14] = valToWriteCPSR[14];
            gbaREG.cpsr[15] = valToWriteCPSR[15];
        }
        if(BB4MSR[2] == 1)
        {
            gbaREG.cpsr[16] = valToWriteCPSR[16];
            gbaREG.cpsr[17] = valToWriteCPSR[17];
            gbaREG.cpsr[18] = valToWriteCPSR[18];
            gbaREG.cpsr[19] = valToWriteCPSR[19];
            gbaREG.cpsr[20] = valToWriteCPSR[20];
            gbaREG.cpsr[21] = valToWriteCPSR[21];
            gbaREG.cpsr[22] = valToWriteCPSR[22];
            gbaREG.cpsr[23] = valToWriteCPSR[23];
        }
        if(BB4MSR[3] == 1)
        {
            gbaREG.cpsr[24] = valToWriteCPSR[24];
            gbaREG.cpsr[25] = valToWriteCPSR[25];
            gbaREG.cpsr[26] = valToWriteCPSR[26];
            gbaREG.cpsr[27] = valToWriteCPSR[27];
            gbaREG.cpsr[28] = valToWriteCPSR[28];
            gbaREG.cpsr[29] = valToWriteCPSR[29];
            gbaREG.cpsr[30] = valToWriteCPSR[30];
            gbaREG.cpsr[31] = valToWriteCPSR[31];
        }
    }
    if(r22[0] == 1)
    {
        if(BB4MSR[0] == 1)
        {
            gbaREG.spsr[0] = valToWriteCPSR[0];
            gbaREG.spsr[1] = valToWriteCPSR[1];
            gbaREG.spsr[2] = valToWriteCPSR[2];
            gbaREG.spsr[3] = valToWriteCPSR[3];
            gbaREG.spsr[4] = valToWriteCPSR[4];
            gbaREG.spsr[5] = valToWriteCPSR[5];
            gbaREG.spsr[6] = valToWriteCPSR[6];
            gbaREG.spsr[7] = valToWriteCPSR[7];
        }
        if(BB4MSR[1] == 1)
        {
            gbaREG.spsr[8] = valToWriteCPSR[8];
            gbaREG.spsr[9] = valToWriteCPSR[9];
            gbaREG.spsr[10] = valToWriteCPSR[10];
            gbaREG.spsr[11] = valToWriteCPSR[11];
            gbaREG.spsr[12] = valToWriteCPSR[12];
            gbaREG.spsr[13] = valToWriteCPSR[13];
            gbaREG.spsr[14] = valToWriteCPSR[14];
            gbaREG.spsr[15] = valToWriteCPSR[15];
        }
        if(BB4MSR[2] == 1)
        {
            gbaREG.spsr[16] = valToWriteCPSR[16];
            gbaREG.spsr[17] = valToWriteCPSR[17];
            gbaREG.spsr[18] = valToWriteCPSR[18];
            gbaREG.spsr[19] = valToWriteCPSR[19];
            gbaREG.spsr[20] = valToWriteCPSR[20];
            gbaREG.spsr[21] = valToWriteCPSR[21];
            gbaREG.spsr[22] = valToWriteCPSR[22];
            gbaREG.spsr[23] = valToWriteCPSR[23];
        }
        if(BB4MSR[3] == 1)
        {
            gbaREG.spsr[24] = valToWriteCPSR[24];
            gbaREG.spsr[25] = valToWriteCPSR[25];
            gbaREG.spsr[26] = valToWriteCPSR[26];
            gbaREG.spsr[27] = valToWriteCPSR[27];
            gbaREG.spsr[28] = valToWriteCPSR[28];
            gbaREG.spsr[29] = valToWriteCPSR[29];
            gbaREG.spsr[30] = valToWriteCPSR[30];
            gbaREG.spsr[31] = valToWriteCPSR[31];
        }
    }
    gbaREG.rNUM[15] += 4;
}
uint32_t writeReadAddress;
uint16_t offset_12;
std::bitset<12> offset_12B;
std::bitset<8> checkFor0forB4tB11;
uint8_t checkFor0forB4tB11u;
uint8_t PWLSval;
std::bitset<4> conditionCode;
uint8_t conditionCodeS;
uint8_t shiftOP;
std::bitset<2> shiftOPB;
uint32_t shiftReg;
int loadStoreOp()
{
    op32bit = currentOpcode;
    //std::cout<<"Opcode: "<<op32bit<<std::endl;
    iBit = op32bit[25]; // 0
    pBit = op32bit[24]; // 1
    uBit = op32bit[23]; // 0
    bBit = op32bit[22]; // 0
    wBit = op32bit[21]; // 0
    lBit = op32bit[20]; // 1
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong(); // R0
    //printf("opRN: 0x%X\n",opRN);
    //std::cout<<"ibit: "<<iBit<<std::endl;
    PWLSval = pBit << 1 | wBit;
    if(iBit == 0) // Immediate
    {
        switch(PWLSval)
        {
            case 0x0: // Immediate Post Indexed
                //printf("case0\n");
                if(uBit == 1)
                {
                    offset_12B = currentOpcode;
                    offset_12 = offset_12B.to_ulong();
                    //printf("offset12: 0x%X\n",offset_12);
                    opRDET = currentOpcode >> 12;
                    opRD = opRDET.to_ulong();
                    //printf("opRD: 0x%X\n",opRD);
                    writeReadAddress = gbaREG.rNUM[opRN];
                    //printf("WRAddress: 0x%X\n",writeReadAddress);
                    if(opRN == 0xF)
                    {
                        writeReadAddress += 8;
                    }
                    if(opRN != 0xF)
                    {
                        gbaREG.rNUM[opRN] = writeReadAddress + offset_12;
                    }
                }
                if(uBit == 0)
                {
                    offset_12B = currentOpcode;
                    offset_12 = offset_12B.to_ulong();
                    //printf("offset12: 0x%X\n",offset_12);
                    opRDET = currentOpcode >> 12;
                    opRD = opRDET.to_ulong();
                    //printf("opRD: 0x%X\n",opRD);
                    writeReadAddress = gbaREG.rNUM[opRN];
                    //printf("WRAddress: 0x%X\n",writeReadAddress);
                    if(opRN == 0xF)
                    {
                        writeReadAddress += 8;
                    }
                    if(opRN != 0xF)
                    {
                        gbaREG.rNUM[opRN] = writeReadAddress - offset_12;
                    }
                }
            break;

            case 0x2: // Immediate Offset
                //printf("case2\n");
                if(uBit == 1)
                {
                    offset_12B = currentOpcode;
                    offset_12 = offset_12B.to_ulong();
                    writeReadAddress = gbaREG.rNUM[opRN] + offset_12;
                    //printf("WRAddress: 0x%X\n",writeReadAddress);
                    if(opRN == 0xF)
                    {
                        writeReadAddress += 8;
                    }
                }
                if(uBit == 0)
                {
                    offset_12B = currentOpcode;
                    offset_12 = offset_12B.to_ulong();
                    printf("offset_12B: 0x%X\n",offset_12);
                    writeReadAddress = gbaREG.rNUM[opRN] - offset_12;
                    printf("wrAddress: 0x%X\n",writeReadAddress);
                    //printf("WRAddress: 0x%X\n",writeReadAddress);
                    if(opRN == 0xF)
                    {
                        writeReadAddress += 8;
                    }
                }
            break;

            case 0x3: // Immediate Pre Indexed
                //printf("case3\n");
                if(uBit == 1)
                {
                    offset_12B = currentOpcode;
                    offset_12 = offset_12B.to_ulong();
                    //printf("offset12: 0x%X\n",offset_12);
                    opRDET = currentOpcode >> 12;
                    opRD = opRDET.to_ulong();
                    //printf("opRD: 0x%X\n",opRD);
                    writeReadAddress = gbaREG.rNUM[opRN] + offset_12;
                    //printf("WRAddress: 0x%X\n",writeReadAddress);
                    if(opRN == 0xF)
                    {
                        writeReadAddress += 8;
                    }
                    if(opRN != 0xF)
                    {
                        gbaREG.rNUM[opRN] = writeReadAddress + offset_12;
                    }
                }
                if(uBit == 0)
                {
                    offset_12B = currentOpcode;
                    offset_12 = offset_12B.to_ulong();
                    //printf("offset12: 0x%X\n",offset_12);
                    opRDET = currentOpcode >> 12;
                    opRD = opRDET.to_ulong();
                    //printf("opRD: 0x%X\n",opRD);
                    writeReadAddress = gbaREG.rNUM[opRN] - offset_12;
                    //printf("WRAddress: 0x%X\n",writeReadAddress);
                    if(opRN == 0xF)
                    {
                        writeReadAddress += 8;
                    }
                    if(opRN != 0xF)
                    {
                        gbaREG.rNUM[opRN] = writeReadAddress - offset_12;
                    }
                }
            break;

            default:
                printf("UNIMPLEMENTED PWLS MODE 0x%X!\n",PWLSval);
                opcodeError = true;
            break;
        }
    }
    if(iBit == 1) // Register
    {
        checkFor0forB4tB11 = currentOpcode >> 4;
        checkFor0forB4tB11u = checkFor0forB4tB11.to_ulong();
        if(checkFor0forB4tB11u == 0) // REGISTER
        {
            switch(PWLSval)
            {
                case 0x0: // Register Post Indexed
                    //printf("Do apply offset only after?\n");
                    opRDET = currentOpcode;
                    opRM = opRDET.to_ulong();
                    //printf("opRM: 0x%X\n",opRM);
                    //printf("opRN: 0x%X\n",opRN);
                    if(uBit == 1)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN];
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRN != 0xF)
                        {
                            gbaREG.rNUM[opRN] = writeReadAddress  + gbaREG.rNUM[opRM];
                        }
                    }
                    if(uBit == 0)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN];
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress -= 8;
                        }
                        if(opRN != 0xF)
                        {
                            gbaREG.rNUM[opRN] = writeReadAddress - gbaREG.rNUM[opRM];
                        }
                    }
                break;

                case 0x2: // Register Offset
                    opRDET = currentOpcode;
                    opRM = opRDET.to_ulong();
                    if(uBit == 1)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN] + gbaREG.rNUM[opRM];
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                    }
                    if(uBit == 0)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN] - gbaREG.rNUM[opRM];
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress -= 8;
                        }
                    }
                break;

                case 0x3: // Register Pre Indexed
                    //printf("Do apply offset only after?\n");
                    opRDET = currentOpcode;
                    opRM = opRDET.to_ulong();
                    //printf("opRM: 0x%X\n",opRM);
                    //printf("opRN: 0x%X\n",opRN);
                    if(uBit == 1)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN] + gbaREG.rNUM[opRM];
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRN != 0xF)
                        {
                            gbaREG.rNUM[opRN] = writeReadAddress + gbaREG.rNUM[opRM];
                        }
                    }
                    if(uBit == 0)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN] - gbaREG.rNUM[opRM];
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress -= 8;
                        }
                        if(opRN != 0xF)
                        {
                            gbaREG.rNUM[opRN] = writeReadAddress - gbaREG.rNUM[opRM];
                        }
                    }
                break;
            }
        }
        if(checkFor0forB4tB11u != 0) // Scaled Register
        {
            switch(PWLSval)
            {
                case 0x2: // Scaled Register Offset
                    opRDET = currentOpcode;
                    opRM = opRDET.to_ulong();
                    opRDET = currentOpcode >> 16;
                    opRN = opRDET.to_ulong();
                    shiftOPB = currentOpcode >> 5;
                    shiftOP = shiftOPB.to_ulong();
                    shiftReg = gbaREG.rNUM[opRM];
                    shiftIMM2 = currentOpcode >> 7;
                    shiftIMM = shiftIMM2.to_ulong();
                    //printf("opRM: 0x%X\n",opRM);
                    //printf("opRN: 0x%X\n",opRN);
                    switch(shiftOP)
                    {
                        case 0x0:
                            shiftReg = shiftReg << shiftIMM;
                        break;

                        case 0x3:
                            if(shiftIMM == 0)
                            {
                                printf("Unimplemented shiftOP mode RRX\n");
                                opcodeError = true;
                            }
                            if(shiftIMM != 0)
                            {
                                shiftReg = rotateValue32(true,shiftIMM,shiftReg);
                            }
                        break;

                        default:
                            printf("Unimplemented shiftOP mode 0x%X\n",shiftOP);
                            opcodeError = true;
                        break;
                    }
                    if(uBit == 1)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN] + shiftReg;
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                    }
                    if(uBit == 0)
                    {
                        writeReadAddress = gbaREG.rNUM[opRN] - shiftReg;
                        if(opRN == 0xF)
                        {
                            writeReadAddress += 8;
                        }
                        if(opRM == 0xF)
                        {
                            writeReadAddress -= 8;
                        }
                    }
                    //printf("W/R Address: 0x%X\n",writeReadAddress);
                    //opcodeError = true;
                    //return 0;
                break;

                default:
                    printf("Unimplemented Scaled Register PWLS Mode 0x%X\n",PWLSval);
                    opcodeError = true;
                break;
            }
        }
    }
    /*
    *   Immediate Offset:            XXXX0101??0?
    *   Immediate Pre-Indexed:       XXXX0101??1?
    *   Immediate Post-Indexed:      XXXX0100??0?
    *   Register Offset:             XXXX0111??0?XXXXXXXX00000000XXXX
    *   Register Pre-Indexed:        XXXX0111??1?XXXXXXXX00000000XXXX
    *   Register Post-Indexed:       XXXX0110??0?XXXXXXXX00000000XXXX
    *   Scaled Register Offset:      XXXX0111??0?XXXXXXXXXXXXXXX0XXXX
        Scaled Register Pre-Indexed: XXXX0111??1?XXXXXXXXXXXXXXX0XXXX
        Scaled Register Post-Indexed:XXXX0110??0?XXXXXXXXXXXXXXX0XXXX
    */
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    if(bBit == 0 && lBit == 0)
    {
        writeMem(2,writeReadAddress,gbaREG.rNUM[opRD]);
    }
    if(bBit == 1 && lBit == 0)
    {
        writeMem(0,writeReadAddress,gbaREG.rNUM[opRD]);
    }
    if(bBit == 0 && lBit == 1)
    {
        gbaREG.rNUM[opRD] = readMem(2,writeReadAddress);
    }
    if(bBit == 1 && lBit == 1)
    {
        gbaREG.rNUM[opRD] = readMem(0,writeReadAddress);
    }
    gbaREG.rNUM[15] += 4;
    return 0;
}
bool linkBranch;
int32_t branchOffset;
std::bitset<32> signExtend;
std::bitset<24> branchOffsetB;
void branchOp() // 0x5
{
    branchOffsetB = currentOpcode;
    signExtend = branchOffsetB.to_ulong();
    signExtend = signExtend << 2;
    if(signExtend[25] == 1)
    {
        signExtend[31] = 1;
        signExtend[30] = 1;
        signExtend[29] = 1;
        signExtend[28] = 1;
        signExtend[27] = 1;
        signExtend[26] = 1;
    }
    branchOffset = signExtend.to_ulong();
    op32bit = currentOpcode;
    linkBranch = op32bit[24];
    if(linkBranch == true)
    {
        gbaREG.rNUM[14] = gbaREG.rNUM[15] + 4;
    }
    //printf("branchOffset: %X\n",branchOffset);
    gbaREG.rNUM[15] += branchOffset + 8;
    //printf("PC: 0x%02X\n",gbaREG.rNUM[15]);
}
std::bitset<32> bAndEJump;
void branchAndExchangeOp()
{
    opRDET = currentOpcode;
    opRN = opRDET.to_ulong();
    bAndEJump = gbaREG.rNUM[opRN];
    gbaREG.cpsr[5] = bAndEJump[0];
    bAndEJump[0] = 0;
    gbaREG.rNUM[15] = bAndEJump.to_ulong();
}
uint32_t immed8VAL;
std::bitset<32> B32Bit;
std::bitset<8> immed8valget;
std::bitset<4> immed8rotate;
std::bitset<5> getB7tB11;
void addOP()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    //printf("OPRN: 0x%X\n",opRN);
    opRDET = currentOpcode >> 20;
    doFlagUpdate = opRDET[0];
    opRDET = currentOpcode >> 25;
    B25ShifterDet = opRDET[0];
    switch(B25ShifterDet)
    {
        case 0:
            getB6tB4 = currentOpcode >> 4;
            switch(getB6tB4.to_ulong())
            {
                case 0:
                    getB7tB11 = currentOpcode >> 7;
                    switch(getB7tB11.to_ulong())
                    {
                        case 0:
                            opRDET = currentOpcode;
                            opRM = opRDET.to_ulong();
                            shifterResult = gbaREG.rNUM[opRM];
                        break;

                        //case 6:
                        //    shifterResult = getRRbyIMM();
                        //break;

                        default: // If B7 to B11 != 0
                            shifterResult = getLSLbyIMM();
                            //printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X B7tB11: 0x%X\n",getB6tB4.to_ulong(),getB7tB11.to_ulong());
                            //opcodeError = true;
                        break;
                    }
                break;

                case 2:
                    shifterResult = getLSRbyIMM();
                break;

                default:
                    printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                    opcodeError = true;
                break;
            }
        break;

        case 1:
            rotate_imm = currentOpcode >> 8;
            rotateIMMM = rotate_imm.to_ulong();
            rotateIMMM = 2 * rotateIMMM;
            immed8valget = currentOpcode;
            immed8VAL = immed8valget.to_ulong();
            shifterResult = rotateValue32(true,rotateIMMM,immed8VAL);
            //printf("Shift Result: 0x%X\n",shifterResult);
        break;
    }
    ZOriginal = gbaREG.rNUM[opRD];
    //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] + shifterResult;
    //printf("OPRN: 0x%X\n",opRN);
    //printf("OPRD: 0x%X\n",opRD);
    if(doFlagUpdate == true)
    {
        handleCarryFlag(gbaREG.rNUM[opRN],gbaREG.rNUM[opRN] + shifterResult,2,shifterResult);
        handleOverflowFlag(gbaREG.rNUM[opRN],gbaREG.rNUM[opRN] + shifterResult,2,shifterResult);
    }
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] + shifterResult;
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }
    if(doFlagUpdate == true)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    }
    //std::cout<<"B25: "<<opRDET[0]<<std::endl;
    gbaREG.rNUM[15] += 4;
}
void opAND()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        if(opRN != 0xF)
        {
            gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] & immed8VAL;
        }
        if(opRN == 0xF)
        {
            gbaREG.rNUM[opRD] = (gbaREG.rNUM[opRN] + 8) & immed8VAL;
        }
    }
    if(op32bit[25] == 0)
    {
        opRDET = currentOpcode;
        opRM = opRDET.to_ulong();
        immed8VAL = gbaREG.rNUM[opRM];
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        if(opRN != 0xF)
        {
            gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] & immed8VAL;
        }
        if(opRN == 0xF)
        {
            gbaREG.rNUM[opRD] = (gbaREG.rNUM[opRN] + 8) & immed8VAL;
        }
    }
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    rotation8mov = gbaREG.rNUM[opRD];
    gbaREG.cpsr[29] = rotation8mov[31];
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opBIC()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] & ~immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        opRDET = currentOpcode;
        opRM = opRDET.to_ulong();
        immed8VAL = gbaREG.rNUM[opRM];
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] & ~immed8VAL;
    }
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    rotation8mov = gbaREG.rNUM[opRD];
    gbaREG.cpsr[29] = rotation8mov[31];
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opEOR()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] ^ immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] ^ immed8VAL;
                    break;

                    //case 6:
                    //    immed8VAL = getRRbyIMM();
                    //    B32Bit = immed8VAL;
                    //    gbaREG.cpsr[29] = B32Bit[31];
                    //    ZOriginal = gbaREG.rNUM[opRD];
                    //    printf("IMMED8VAL: 0x%X\n",immed8VAL);
                    //    printf("RS: 0x%X\n",opRN);
                    //    printf("RD: 0x%X\n",opRD);
                    //    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
                    //break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] ^ immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                B32Bit = immed8VAL;
                gbaREG.cpsr[29] = B32Bit[31];
                ZOriginal = gbaREG.rNUM[opRD];
                //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                //printf("RS: 0x%X\n",opRN);
                //printf("RD: 0x%X\n",opRD);
                gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] ^ immed8VAL;
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opOR()
{
    //printf("OR!\n");
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        immed8rotate = currentOpcode >> 8;
        //printf("IMMED8ROT: 0x%X\n",immed8rotate.to_ulong() * 2);
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
                    break;

                    //case 6:
                    //    immed8VAL = getRRbyIMM();
                    //    B32Bit = immed8VAL;
                    //    gbaREG.cpsr[29] = B32Bit[31];
                    //    ZOriginal = gbaREG.rNUM[opRD];
                    //    printf("IMMED8VAL: 0x%X\n",immed8VAL);
                    //    printf("RS: 0x%X\n",opRN);
                    //    printf("RD: 0x%X\n",opRD);
                    //    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
                    //break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                B32Bit = immed8VAL;
                gbaREG.cpsr[29] = B32Bit[31];
                ZOriginal = gbaREG.rNUM[opRD];
                gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
uint32_t cmpValueA;
void opCMP()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        cmpValueA = gbaREG.rNUM[opRN] - immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] - immed8VAL;
                    break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] - immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                B32Bit = immed8VAL;
                gbaREG.cpsr[29] = B32Bit[31];
                ZOriginal = gbaREG.rNUM[opRD];
                cmpValueA = gbaREG.rNUM[opRN] - immed8VAL;
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleSignFlag(cmpValueA);
    handleZeroFlag(ZOriginal,cmpValueA);
    handleCarryFlag(gbaREG.rNUM[opRN],cmpValueA,3,immed8VAL);
    handleOverflowFlag(gbaREG.rNUM[opRN],cmpValueA,3,immed8VAL);
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opCMN()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        cmpValueA = gbaREG.rNUM[opRN] + immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] + immed8VAL;
                    break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] + immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                B32Bit = immed8VAL;
                gbaREG.cpsr[29] = B32Bit[31];
                ZOriginal = gbaREG.rNUM[opRD];
                cmpValueA = gbaREG.rNUM[opRN] + immed8VAL;
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleSignFlag(cmpValueA);
    handleZeroFlag(ZOriginal,cmpValueA);
    handleCarryFlag(gbaREG.rNUM[opRN],cmpValueA,2,immed8VAL);
    handleOverflowFlag(gbaREG.rNUM[opRN],cmpValueA,2,immed8VAL);
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opSUB()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        //gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] - immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        //gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] - immed8VAL;
                    break;

                    //case 6:
                    //    immed8VAL = getRRbyIMM();
                    //    B32Bit = immed8VAL;
                    //    gbaREG.cpsr[29] = B32Bit[31];
                    //    ZOriginal = gbaREG.rNUM[opRD];
                    //    printf("IMMED8VAL: 0x%X\n",immed8VAL);
                    //    printf("RS: 0x%X\n",opRN);
                    //    printf("RD: 0x%X\n",opRD);
                    //    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
                    //break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        //gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] - immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                ZOriginal = gbaREG.rNUM[opRD];
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleCarryFlag(gbaREG.rNUM[opRN],gbaREG.rNUM[opRN] - immed8VAL,3,immed8VAL);
    handleOverflowFlag(gbaREG.rNUM[opRN],gbaREG.rNUM[opRN] - immed8VAL,3,immed8VAL);
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] - immed8VAL;
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opRSB()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        //gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] - immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        //gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] - immed8VAL;
                    break;

                    //case 6:
                    //    immed8VAL = getRRbyIMM();
                    //    B32Bit = immed8VAL;
                    //    gbaREG.cpsr[29] = B32Bit[31];
                    //    ZOriginal = gbaREG.rNUM[opRD];
                    //    printf("IMMED8VAL: 0x%X\n",immed8VAL);
                    //    printf("RS: 0x%X\n",opRN);
                    //    printf("RD: 0x%X\n",opRD);
                    //    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] | immed8VAL;
                    //break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        //gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] - immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                ZOriginal = gbaREG.rNUM[opRD];
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleCarryFlag(immed8VAL,immed8VAL - gbaREG.rNUM[opRN],3,gbaREG.rNUM[opRN]);
    handleOverflowFlag(immed8VAL,immed8VAL - gbaREG.rNUM[opRN],3,gbaREG.rNUM[opRN]);
    gbaREG.rNUM[opRD] = immed8VAL - gbaREG.rNUM[opRN];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opADC()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        //gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] + immed8VAL + gbaREG.cpsr[29];
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        //gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);

                        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] + immed8VAL + gbaREG.cpsr[29];
                    break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        //gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        //gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] + immed8VAL + gbaREG.cpsr[29];
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                ZOriginal = gbaREG.rNUM[opRD];
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleCarryFlag(gbaREG.rNUM[opRN],gbaREG.rNUM[opRN] + immed8VAL + gbaREG.cpsr[29],2,immed8VAL + gbaREG.cpsr[29]);
    handleOverflowFlag(gbaREG.rNUM[opRN],gbaREG.rNUM[opRN] + immed8VAL + gbaREG.cpsr[29],2,immed8VAL + gbaREG.cpsr[29]);

    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] + immed8VAL + gbaREG.cpsr[29];
    if(opRN == 0xF)
    {
        gbaREG.rNUM[opRD] += 8;
    }

    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
uint16_t immed8rotate2;
void opTST()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8rotate2 = immed8rotate.to_ulong();
        immed8rotate2 = immed8rotate2 * 2;
        immed8VAL = rotateValue32(true,immed8rotate2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        cmpValueA = gbaREG.rNUM[opRN] & immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] & immed8VAL;
                    break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] & immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                B32Bit = immed8VAL;
                gbaREG.cpsr[29] = B32Bit[31];
                ZOriginal = gbaREG.rNUM[opRD];
                cmpValueA = gbaREG.rNUM[opRN] & immed8VAL;
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleSignFlag(cmpValueA);
    handleZeroFlag(ZOriginal,cmpValueA);
    rotation8mov = cmpValueA;
    gbaREG.cpsr[29] = rotation8mov[31];
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
void opTEQ()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[25] == 1)
    {
        immed8valget = currentOpcode;
        immed8VAL = immed8valget.to_ulong();
        immed8rotate = currentOpcode >> 8;
        immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);
        B32Bit = immed8VAL;
        gbaREG.cpsr[29] = B32Bit[31];
        ZOriginal = gbaREG.rNUM[opRD];
        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
        //printf("RS: 0x%X\n",opRN);
        //printf("RD: 0x%X\n",opRD);
        cmpValueA = gbaREG.rNUM[opRN] ^ immed8VAL;
    }
    if(op32bit[25] == 0)
    {
        getB6tB4 = currentOpcode >> 4;
        switch(getB6tB4.to_ulong())
        {
            case 0:
                getB7tB11 = currentOpcode >> 7;
                switch(getB7tB11.to_ulong())
                {
                    case 0:
                        opRDET = currentOpcode;
                        opRM = opRDET.to_ulong();
                        immed8VAL = gbaREG.rNUM[opRM];
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] ^ immed8VAL;
                    break;

                    default:
                        immed8VAL = getLSLbyIMM();
                        B32Bit = immed8VAL;
                        gbaREG.cpsr[29] = B32Bit[31];
                        ZOriginal = gbaREG.rNUM[opRD];
                        //printf("IMMED8VAL: 0x%X\n",immed8VAL);
                        //printf("RS: 0x%X\n",opRN);
                        //printf("RD: 0x%X\n",opRD);
                        cmpValueA = gbaREG.rNUM[opRN] ^ immed8VAL;
                    break;
                }
            break;

            case 2:
                immed8VAL = getLSRbyIMM();
                B32Bit = immed8VAL;
                gbaREG.cpsr[29] = B32Bit[31];
                ZOriginal = gbaREG.rNUM[opRD];
                cmpValueA = gbaREG.rNUM[opRN] ^ immed8VAL;
            break;

            default:
                printf("Unimplemented Shifter opernand B25: 0 and B6tB4: 0x%X\n",getB6tB4.to_ulong());
                opcodeError = true;
            break;
        }
        //immed8valget = currentOpcode;
        //immed8VAL = immed8valget.to_ulong();
        //immed8rotate = currentOpcode >> 8;
        //immed8VAL = rotateValue32(true,immed8rotate.to_ulong() * 2,immed8VAL);/
        /*
        */
    }
    handleSignFlag(cmpValueA);
    handleZeroFlag(ZOriginal,cmpValueA);
    rotation8mov = cmpValueA;
    gbaREG.cpsr[29] = rotation8mov[31];
    gbaREG.rNUM[15] += 4;
    //std::cout<<"B25: "<<op32bit[25]<<std::endl;
}
std::bitset<32> bitOpcode;
std::bitset<1> currentOpcodeBit;
std::bitset<4> opcode001;
std::bitset<4> check11to8;
uint8_t opcode0018B;
std::bitset<5> B7tB11;
void dataProcessingORpsrTransfer(uint32_t opcode)
{
    opcode001 = opcode >> 21;
    opcode0018B = opcode001.to_ulong();
    //printf("opcode001: 0x%X\n",opcode0018B);
    currentOpcodeBit = opcode >> 25;
    switch(opcode0018B) // XXXX001????
    {
        case 0x0:
            opAND();
        break;

        case 0x1:
            opEOR();
        break;

        case 0x2:
            opSUB();
        break;

        case 0x3:
            opRSB();
        break;

        case 0x4:
            addOP();
        break;

        case 0x5:
            opADC();
        break;

        case 0x8:
            opTST();
        break;

        case 0x9:
            opTEQ();
        break;

        case 0xA:
            opCMP();
        break;

        case 0xB:
            opCMN();
        break;

        case 0xC:
            opOR();
        break;

        case 0xD:
            currentOpcodeBit = opcode >> 25;
            if(currentOpcodeBit[0] == 1)
            {
                movOP();
            }
            if(currentOpcodeBit[0] == 0)
            {
                std::bitset<3> bit27t25;
                bit27t25 = currentOpcode >> 4;
                uint8_t checkModeMov = bit27t25.to_ulong();
                switch(checkModeMov)
                {
                    case 0:
                        B7tB11 = currentOpcode >> 7;
                        if(B7tB11.to_ulong() == 0)
                        {
                            movOPREG();
                        }
                        if(B7tB11.to_ulong() != 0)
                        {
                            bit27t25 = currentOpcode >> 4;
                            switch(bit27t25.to_ulong())
                            {
                                case 0x0:
                                    movOPLSLbyImmediate();
                                break;

                                default:
                                    printf("UNIMPLEMENTED TYPE OF MOVE: ");
                                    std::cout<<bit27t25<<std::endl;
                                    opcodeError = true;
                                break;
                            }
                        }
                    break;

                    case 1:
                        movOPLSLbyRegister();
                    break;

                    case 2:
                        movOPLSRbyImmediate();
                    break;

                    case 4:
                        movOPASRbyImmediate();
                    break;

                    case 5:
                        movOPASRbyRegister();
                    break;

                    case 6:
                        movOPRORbyImmediate();
                    break;

                    default:
                        printf("UNIMPLEMENTED TYPE OF MOVE2 0x%X!\n",checkModeMov);
                        opcodeError = true;
                    break;
                }
            }
        break;

        case 0xE:
            opBIC();
        break;

        case 0xF:
            currentOpcodeBit = opcode >> 25;
            if(currentOpcodeBit[0] == 1)
            {
                movNOTOP();
            }
            if(currentOpcodeBit[0] == 0)
            {
                std::bitset<3> bit27t25;
                bit27t25 = currentOpcode >> 4;
                uint8_t checkModeMov = bit27t25.to_ulong();
                switch(checkModeMov)
                {
                    case 0:
                        B7tB11 = currentOpcode >> 7;
                        if(B7tB11.to_ulong() == 0)
                        {
                            movNOTOPREG();
                        }
                        if(B7tB11.to_ulong() != 0)
                        {
                            bit27t25 = currentOpcode >> 4;
                            switch(bit27t25.to_ulong())
                            {
                                case 0x0:
                                    movNOTOPLSLbyImmediate();
                                break;

                                default:
                                    printf("UNIMPLEMENTED TYPE OF NOTMOVE: ");
                                    std::cout<<bit27t25<<std::endl;
                                    opcodeError = true;
                                break;
                            }
                        }
                    break;

                    case 2:
                        movNOTOPLSRbyImmediate();
                    break;

                    default:
                        printf("UNIMPLEMENTED TYPE OF NOTMOVE2 0x%X!\n",checkModeMov);
                        opcodeError = true;
                    break;
                }
            }
        break;

        default:
            printf("UNIMPLEMENTED OP XXXX00");
            currentOpcodeBit = opcode >> 25;
            std::cout<<currentOpcodeBit;
            printf(" then 0x%X!\n",opcode0018B);
            std::cout<<"Opcode: "<<bitOpcode<<std::endl;
            opcodeError = true;
        break;
    }
}
uint8_t registerIndexDetermine;
std::bitset<4> offset8ImmedB1;
std::bitset<4> offset8ImmedB2;
void opLoadStoreHalfwordWithOffset()
{
    /*
        Bit 5  (H) Is at 1 when this function is called
        Bit 6  (S) Is at 0 when this function is called
        Bit 20 (L) Specifies a load or store
        Bit 22 (I) Determines Immediate or Register
        Bit 23 (U) Specifies if the offset is added or subtracted from a register
        Bit 24 (P) & Bit 21 (W) Specifies the type of load or store to do.
    */
    op32bit = currentOpcode;
    pBit = op32bit[24];
    uBit = op32bit[23];
    iBit = op32bit[22];
    wBit = op32bit[21];
    lBit = op32bit[20];
    registerIndexDetermine = pBit << 1 | wBit;
    if(iBit == false) // Register Index
    {
        switch(registerIndexDetermine)
        {
            case 0x0:
                opRDET = currentOpcode >> 16;
                opRN = opRDET.to_ulong();
                offset8ImmedB1 = currentOpcode;
                offset8ImmedB2 = currentOpcode >> 8;
                offset8Immed = offset8ImmedB2.to_ulong() << 4 | offset8ImmedB1.to_ulong();
                if(uBit == true)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] + gbaREG.rNUM[opRM];
                    gbaREG.rNUM[opRN] = writeReadAddress;
                }
                if(uBit == false)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] - gbaREG.rNUM[opRM];
                    gbaREG.rNUM[opRN] = writeReadAddress;
                }
            break;

            case 0x2:
                opRDET = currentOpcode;
                opRM = opRDET.to_ulong();
                opRDET = currentOpcode >> 16;
                opRN = opRDET.to_ulong();
                if(uBit == true)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] + gbaREG.rNUM[opRM];
                }
                if(uBit == false)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] - gbaREG.rNUM[opRM];
                }
            break;

            default:
                printf("Unimplemented Register Index 0x%X!\n",registerIndexDetermine);
                opcodeError = true;
            break;
        }
    }
    if(iBit == true) // Immediate Index
    {
        switch(registerIndexDetermine)
        {
            case 0x0:
                opRDET = currentOpcode >> 16;
                opRN = opRDET.to_ulong();
                offset8ImmedB1 = currentOpcode;
                offset8ImmedB2 = currentOpcode >> 8;
                offset8Immed = offset8ImmedB2.to_ulong() << 4 | offset8ImmedB1.to_ulong();
                if(uBit == true)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] + offset8Immed;
                    gbaREG.rNUM[opRN] = writeReadAddress;
                }
                if(uBit == false)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] - offset8Immed;
                    gbaREG.rNUM[opRN] = writeReadAddress;
                }
            break;

            case 0x2:
                opRDET = currentOpcode >> 16;
                opRN = opRDET.to_ulong();
                offset8ImmedB1 = currentOpcode;
                offset8ImmedB2 = currentOpcode >> 8;
                offset8Immed = offset8ImmedB2.to_ulong() << 4 | offset8ImmedB1.to_ulong();
                if(uBit == true)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] + offset8Immed;
                }
                if(uBit == false)
                {
                    writeReadAddress = gbaREG.rNUM[opRN] - offset8Immed;
                }
            break;

            default:
                printf("Unimplemented Immediate Index 0x%X!\n",registerIndexDetermine);
                opcodeError = true;
            break;
        }
    }
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    if(lBit == 0)
    {
        writeMem(1,writeReadAddress,gbaREG.rNUM[opRD]);
    }
    if(lBit == 1)
    {
        gbaREG.rNUM[opRD] = readMem(1,writeReadAddress);
    }
    gbaREG.rNUM[15] += 4;
}
std::bitset<16> regListToPushPull;
uint8_t loopRegList;
uint32_t regUseOther;
uint8_t PULSDet;
void opLoadStoreMultiple()
{
    op32bit = currentOpcode;
    pBit = op32bit[24];
    uBit = op32bit[23];
    sBit = op32bit[22];
    wBit = op32bit[21];
    lBit = op32bit[20];
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    regListToPushPull = currentOpcode;
    PULSDet = pBit << 1 | uBit;
    if(sBit == 1)
    {
        printf("SBIT IS ENABLED!  COULD CAUSE BUGS!\n");
    }
    if(uBit == 1) // R15 to R0
    {
        regUseOther = gbaREG.rNUM[opRN];
        loopRegList = 0xF;
        while(loopRegList != 0xFF)
        {
            if(regListToPushPull[loopRegList] == 1)
            {
                if(pBit == 1)
                {
                    regUseOther += 4;
                }
                if(lBit == 1)
                {
                    gbaREG.rNUM[loopRegList] = readMem(2,regUseOther);
                }
                if(lBit == 0)
                {
                    writeMem(2,regUseOther,gbaREG.rNUM[loopRegList]);
                }
                if(pBit == 0)
                {
                    regUseOther += 4;
                }
            }
            loopRegList -= 1;
            if(loopRegList > 0xF && loopRegList != 0xFF)
            {
                printf("loopREG: 0x%X\n",loopRegList);
            }

        }
        if(wBit == 1)
        {
            gbaREG.rNUM[opRN] = regUseOther;
        }
    }
    if(uBit == 0) // R0 to R15
    {
        regUseOther = gbaREG.rNUM[opRN];
        loopRegList = 0x0;
        while(loopRegList != 0x10)
        {
            if(regListToPushPull[loopRegList] == 1)
            {
                if(pBit == 1)
                {
                    regUseOther -= 4;
                }
                if(lBit == 1)
                {
                    gbaREG.rNUM[loopRegList] = readMem(2,regUseOther);
                }
                if(lBit == 0)
                {
                    writeMem(2,regUseOther,gbaREG.rNUM[loopRegList]);
                }
                if(pBit == 0)
                {
                    regUseOther -= 4;
                }
            }
            loopRegList++;
            if(loopRegList > 0xF && loopRegList != 0xFF)
            {
                printf("loopREG: 0x%X\n",loopRegList);
            }
        }
        if(wBit == 1)
        {
            gbaREG.rNUM[opRN] = regUseOther;
        }
    }
    //breakpoint = true;
    gbaREG.rNUM[15] += 4;
    //printf("TEST2\n");
}
void opMultiplyMUL()
{
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 8;
    opRS = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRD = opRDET.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRM] * gbaREG.rNUM[opRS];
    op32bit = currentOpcode;
    if(op32bit[20] == 1)
    {
        handleSignFlag(gbaREG.rNUM[opRD]);
        handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    }
    gbaREG.rNUM[15] += 4;
}
std::bitset<4> findMultiply;
void opMultiply()
{
    findMultiply = currentOpcode >> 21;
    switch(findMultiply.to_ulong())
    {
        case 0:
            opMultiplyMUL();
        break;

        default:
            printf("Unimplemented Multiply: 0x%X\n",findMultiply.to_ulong());
            opcodeError = true;
        break;
    }
}
void mrsREGOP()
{
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    op32bit = currentOpcode;
    if(op32bit[22] == 1)
    {
        gbaREG.rNUM[opRD] = gbaREG.spsr.to_ulong();
    }
    if(op32bit[22] == 0)
    {
        gbaREG.rNUM[opRD] = gbaREG.cpsr.to_ulong();
    }
    gbaREG.rNUM[15] += 4;
}
std::bitset<4> getB7tB4;
void handleMiscInstructions()
{
    getB7tB4 = currentOpcode >> 4;
    op32bit = currentOpcode;
    switch(getB7tB4.to_ulong())
    {
        case 0x0:
            switch(op32bit[21])
            {
                case 0:
                    mrsREGOP();
                break;

                case 1:
                    msrREGOP();
                break;
            }
        break;

        case 0x1:
            switch(op32bit[22])
            {
                case 0:
                    branchAndExchangeOp();
                break;

                case 1:
                    printf("COUNT LEADING ZEROS NOT IMPLEMENTED!\n");
                    opcodeError = true;
                break;
            }
        break;

        default:
            printf("UNIMPLEMENTED MISC INSTRUCTION!\n");
            opcodeError = true;
        break;
    }
}
uint32_t loadAddress;
int32_t signExtend2;
void armLoadStoreSignByteWRegisterOffset() // These function names are getting kinda long.
{
    op32bit = currentOpcode;
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    if(op32bit[23] == 1)
    {
        loadAddress = gbaREG.rNUM[opRN] + gbaREG.rNUM[opRM];
    }
    if(op32bit[23] == 0)
    {
        loadAddress = gbaREG.rNUM[opRN] - gbaREG.rNUM[opRM];
    }
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    printf("PC OFFSET NOT DONE YET!\n");
    switch(op32bit[20])
    {
        case 0: // Store
            signExtend2 = gbaREG.rNUM[opRD];
            signExtend2 = (signExtend2 << 24) >> 24;
            writeMem(0,loadAddress,signExtend2);
        break;

        case 1: // Load
            signExtend2 = readMem(0,loadAddress);
            signExtend2 = (signExtend2 << 24) >> 24;
            gbaREG.rNUM[opRD] = signExtend2;
        break;
    }
    gbaREG.rNUM[15] += 4;
}
void armLoadStoreSignHalfWordWRegisterOffset()
{
    op32bit = currentOpcode;
    opRDET = currentOpcode;
    opRM = opRDET.to_ulong();
    opRDET = currentOpcode >> 16;
    opRN = opRDET.to_ulong();
    if(op32bit[23] == 1)
    {
        loadAddress = gbaREG.rNUM[opRN] + gbaREG.rNUM[opRM];
    }
    if(op32bit[23] == 0)
    {
        loadAddress = gbaREG.rNUM[opRN] - gbaREG.rNUM[opRM];
    }
    opRDET = currentOpcode >> 12;
    opRD = opRDET.to_ulong();
    printf("PC OFFSET NOT DONE YET!\n");
    switch(op32bit[20])
    {
        case 0: // Store
            signExtend2 = gbaREG.rNUM[opRD];
            signExtend2 = (signExtend2 << 16) >> 16;
            writeMem(1,loadAddress,signExtend2);
        break;

        case 1: // Load
            signExtend2 = readMem(1,loadAddress);
            signExtend2 = (signExtend2 << 16) >> 16;
            gbaREG.rNUM[opRD] = signExtend2;
        break;
    }
    gbaREG.rNUM[15] += 4;
}
void handleMultiplyandExtra()
{
    getB7tB4 = currentOpcode >> 4;
    op32bit = currentOpcode;
    switch(getB7tB4.to_ulong())
    {
        case 0x9:
            switch(op32bit[24])
            {
                case 0:
                    opMultiply();
                break;

                default:
                    printf("SWAP NOT IMPLEMENTED!\n");
                    opcodeError = true;
                break;
            }
        break;

        case 0xB:
            opLoadStoreHalfwordWithOffset();
        break;

        case 0xD:
            switch(op32bit[20])
            {
                case 0:
                    printf("LOAD STORE TWO WORDS NOT IMPLEMENTED!\n");
                    opcodeError = true;
                break;

                case 1:
                    switch(op32bit[22])
                    {
                        case 0:
                            armLoadStoreSignByteWRegisterOffset();
                        break;

                        case 1:
                            armLoadStoreSignHalfWordWRegisterOffset();
                        break;
                    }
                break;
            }
        break;

        default:
            printf("UNIMPLEMENTED MULTIPLY AND EXTRA OPCODE 0x%X!\n",getB7tB4.to_ulong());
            opcodeError = true;
        break;
    }
}
std::bitset<24> offsetBB24;
void doSoftwareInterruptARM()
{
    offsetBB24 = currentOpcode;
    breakpoint = true;
    printf("SOFTWARE IN ARM IS ATTEMPTING BIOS CALL 0x%X!\n",offsetBB24.to_ulong());
    gbaREG.R1314_svc[1] = gbaREG.rNUM[15] + 4;
    //gbaREG.R1314_svc[0] = gbaREG.rNUM[13];
    gbaREG.spsr_svc = gbaREG.cpsr.to_ulong();
    gbaREG.rNUM[15] = 0x8;
    gbaREG.cpsr[5] = 0;
    gbaREG.cpsr[7] = 1;
    gbaREG.cpsr[4] = 1;
    gbaREG.cpsr[3] = 0;
    gbaREG.cpsr[2] = 0;
    gbaREG.cpsr[1] = 1;
    gbaREG.cpsr[0] = 1;
    //breakpoint = true;
}
int findAndDoOpcode(uint32_t opcode)
{
    // This will likely become a mess, but in the interest of just getting the CPU working, i'm going to use it until it works.
    // This code looks awful, but it's a lot better than the last implementation.
    bitOpcode = opcode;
    currentOpcodeBit = opcode >> 27;
    //std::cout<<currentOpcodeBit<<std::endl;
    switch(currentOpcodeBit[0])
    {
        case 0: // 0
            currentOpcodeBit = opcode >> 26;
            switch(currentOpcodeBit[0])
            {
                case 0: // 00
                    currentOpcodeBit = opcode >> 25;
                    switch(currentOpcodeBit[0])
                    {
                        case 0: // 000
                            //currentOpcodeBit = currentOpcode >> 24;
                            op32bit = currentOpcode;
                            if(op32bit[24] == 1 && op32bit[23] == 0 && op32bit[20] == 0 && op32bit[4] == 0)
                            {// Miscellaneous
                                handleMiscInstructions();
                                return 0;
                            }
                            if(op32bit[24] == 1 && op32bit[23] == 0 && op32bit[20] == 0 && op32bit[4] == 1 && op32bit[7] == 0)
                            {// Miscellaneous
                                handleMiscInstructions();
                                return 0;
                            }
                            if(op32bit[7] == 1 && op32bit[4] == 1)
                            {// Multiplies Extra Load Stores
                                handleMultiplyandExtra();
                                return 0;
                            }
                            if(op32bit[24] == 1 && op32bit[23] == 0 && op32bit[20] == 1)
                            {
                                //printf("DATA!\n");
                                dataProcessingORpsrTransfer(currentOpcode);
                                return 0;
                            }
                            if(op32bit[24] != 1 || op32bit[23] != 0)
                            {
                                //printf("DATA!\n");
                                dataProcessingORpsrTransfer(currentOpcode);
                                return 0;
                            }
                            printf("THIS SHOULD NOT HAPPEN22!\n");
                            opcodeError = true;
                        break;

                        case 1: // XXXX001
                            dataProcessingORpsrTransfer(opcode);
                        break;
                    }
                break;

                case 1: // XXXX01
                    loadStoreOp();
                break;
            }
        break;

        case 1:
            currentOpcodeBit = opcode >> 26;
            switch(currentOpcodeBit[0])
            {
                case 0:
                    currentOpcodeBit = opcode >> 25;
                    switch(currentOpcodeBit[0])
                    {
                        case 0:
                            opLoadStoreMultiple();
                        break;

                        case 1:
                            branchOp(); // XXX101END
                        break;
                    }
                break;

                case 1:
                    currentOpcodeBit = opcode >> 25;
                    switch(currentOpcodeBit[0])
                    {
                        case 0:
                            printf("UNIMPLEMENTED OP XXXX110END!\n");
                            opcodeError = true;
                        break;

                        case 1:
                            currentOpcodeBit = opcode >> 24;
                            switch(currentOpcodeBit[0])
                            {
                                case 0:
                                    printf("UNIMPLEMENTED OP XXXX1110END!\n");
                                    opcodeError = true;
                                break;

                                case 1:
                                    doSoftwareInterruptARM();
                                break;
                            }
                        break;
                    }
                    //printf("UNIMPLEMENTED OP XXXX11END!\n");
                    //std::cout<<"Opcode: "<<bitOpcode<<std::endl;
                break;
            }
        break;
    }
}
int doOpcode(uint32_t opcode)
{
    op32bit = opcode;
    op32bit = op32bit >> 28;
    opCondition = op32bit.to_ulong();
    op32bit = opcode;
    //std::cout<<op32bit;
    op32bit = op32bit >> 25;
    //op32bit[7] = 0;
    op32bit[6] = 0;
    op32bit[5] = 0;
    op32bit[4] = 0;
    op32bit[3] = 0;
    //printf("\ntest:0x%X\n",op32bit.to_ulong());
    //std::cout<<op32bit;
    opDetermine = op32bit.to_ulong();
    switch(opCondition)
    {
        case 0x0:
            if(gbaREG.cpsr[30] == 0)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[30] == 1)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x1:
            if(gbaREG.cpsr[30] == 1)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[30] == 0)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x2:
            if(gbaREG.cpsr[29] == 0)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[29] == 1)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x3:
            if(gbaREG.cpsr[29] == 1)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[29] == 0)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x4:
            if(gbaREG.cpsr[31] == 0)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[31] == 1)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x5:
            if(gbaREG.cpsr[31] == 1)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[31] == 0)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x6:
            printf("OVERFLOW NOT IMPLEMENTED!\n");
            if(gbaREG.cpsr[28] == 0)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[28] == 1)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x7:
            printf("OVERFLOW NOT IMPLEMENTED!\n");
            if(gbaREG.cpsr[28] == 1)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[28] == 0)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0x8:
            if(gbaREG.cpsr[29] == 1 && gbaREG.cpsr[30] == 0)
            {
                findAndDoOpcode(opcode);
            }
            if(gbaREG.cpsr[29] != 1 || gbaREG.cpsr[30] != 0)
            {
                gbaREG.rNUM[15] += 4;
            }
        break;

        case 0xA:
            //printf("WARN OP COND USES OVERFLOW AND CARRY!\n");
            if(gbaREG.cpsr[31] != gbaREG.cpsr[28])
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[31] == gbaREG.cpsr[28])
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0xB:
            if(gbaREG.cpsr[31] != gbaREG.cpsr[28])
            {
                findAndDoOpcode(opcode);
                return 0;
            }
            gbaREG.rNUM[15] += 4;
        break;

        case 0xC:
            //printf("WARN OP COND USES OVERFLOW AND CARRY!\n");
            if(gbaREG.cpsr[31] != gbaREG.cpsr[28] || gbaREG.cpsr[30] == 1)
            {
                gbaREG.rNUM[15] += 4;
            }
            if(gbaREG.cpsr[31] == gbaREG.cpsr[28] && gbaREG.cpsr[30] == 0)
            {
                findAndDoOpcode(opcode);
            }
        break;

        case 0xD:
            if(gbaREG.cpsr[30] == 1 || gbaREG.cpsr[31] != gbaREG.cpsr[28])
            {
                findAndDoOpcode(opcode);
                return 0;
            }
            gbaREG.rNUM[15] += 4;
        break;

        case 0xE: // ALWAYS CONDITION
            //opTypes[opDetermine]();
            findAndDoOpcode(opcode);
        break;

        default:
            printf("Error!  Unimplemented Condition 0x%X!\n",opCondition);
            opcodeError = true;
        break;
    }
}

int doOpcodeMain()
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
        return 0;
    }
    if(gbaREG.cpsr[5] == 1)
    {
        //printf("PC: 0x%X\n",gbaREG.rNUM[15]);
        if(gbaREG.rNUM[15] == 0x02000000)
        {
            memDump();
            //printf("MEMDUMP!\n");
        }
            //currentThumbOpcode = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 1] << 8 |
            //                     gbaRAM.flashROM[gbaREG.rNUM[15] - 0x08000000];
        currentThumbOpcode = readMem(1,gbaREG.rNUM[15]);
        op16bit = currentThumbOpcode;
            //printf("Thumb Opcode: 0x%X\n",currentThumbOpcode);
            //printf("In Binary: ");
            //std::cout<<op16bit<<std::endl;
        doThumbOpcode(currentThumbOpcode);
        return 0;
    }

}
std::bitset<16> checkIE;
uint8_t IEcount;
bool endWhile;
void handleInterrupts()
{
    if(gbaREG.IME == true && gbaREG.currentlyHalted == true)
    {
        checkIE = gbaREG.IE;
        IEcount = 0;
        endWhile = false;
        while(IEcount != 16 || endWhile == false)
        {
            if(checkIE[IEcount] == 1)
            {
                endWhile = true;
            }
            IEcount++;
        }
        if(endWhile == true)
        {
            switch(IEcount)
            {
                default: // This is heavily stubbed right now, but whatever.  I just want games to run right now.  I'll fix it later.
                    printf("Doing Hardware Interrupt from bit 0x%X!\n",IEcount);
                    gbaREG.R1314_irq[1] = gbaREG.rNUM[15] + 4;
                    printf("R13irq: 0x%X\n",gbaREG.R1314_irq[0]);
                    printf("R14irq: 0x%X\n",gbaREG.R1314_irq[1]);
                    gbaREG.spsr_irq = gbaREG.cpsr.to_ulong();
                    gbaREG.rNUM[15] = 0x18;
                    gbaREG.cpsr[5] = 0;
                    gbaREG.cpsr[4] = 1;
                    gbaREG.cpsr[3] = 0;
                    gbaREG.cpsr[2] = 0;
                    gbaREG.cpsr[1] = 1;
                    gbaREG.cpsr[0] = 0;
                    //breakpoint = true;
                    printf("Are we supposed to swap to SVC?\n");
                    gbaREG.currentlyHalted = false;
                break;
            }
        }
    }
}
