#include "include.hpp"
void handleZeroFlag(uint32_t original, uint32_t result)
{
    gbaREG.cpsr[30] = 0;
    if(result == 0)
    {
        gbaREG.cpsr[30] = 1;
    }
}
std::bitset<1> signOfResult;
void handleSignFlag(uint32_t result)
{
    signOfResult = result >> 31;
    gbaREG.cpsr[31] = signOfResult[0];
}
uint64_t result64;
void handleCarryFlag(uint32_t original, uint32_t result, uint8_t operation, uint32_t valueUsed)
{
    /*
        Operation Values
        0 = Logical Shift Left
        1 = Auto Set Zero
        2 = Add
        3 = Subtract
    */
    switch(operation)
    {
        case 0:
            gbaREG.cpsr[29] = 0;
            if(valueUsed > 0)
            {
                gbaREG.cpsr[29] = 1;
            }
        break;

        case 1:
            gbaREG.cpsr[29] = 0;
        break;

        case 2:
            gbaREG.cpsr[29] = 0;
            result64 = original + valueUsed;
            if(result64 > 0xFFFFFFFF)
            {
                gbaREG.cpsr[29] = 1;
            }
        break;

        case 3:
            gbaREG.cpsr[29] = 0;
            result64 = original - valueUsed;
            if(result64 > 0xFFFFFFFF)
            {
                gbaREG.cpsr[29] = 1;
            }
        break;

        default:
            printf("Unimplemented Carry Flag Operation!\n");
            opcodeError = true;
        break;
    }
}
// From CPU.hpp
uint8_t opRD;
uint8_t opRN;
uint8_t opRS;
uint8_t opRB;
bool doFlagUpdate;
bool B25ShifterDet;
std::bitset<4> opRDET;
uint32_t shifterResult;
// From this file
std::bitset<8> word8IMM;
std::bitset<16> word8IMM2;
std::bitset<32> fixPCB1;
std::bitset<3> opRDET3B;
uint16_t word8IMMu;
uint32_t fixPCB1u;
void pcRelativeLoad()
{
    opRDET = currentThumbOpcode >> 8;
    opRDET[3] = 0;
    opRD = opRDET.to_ulong();
    word8IMM = currentThumbOpcode;
    word8IMMu = word8IMM.to_ulong();
    word8IMMu = word8IMMu * 4;
    word8IMM2 = word8IMMu;
    word8IMM2[1] = 0;
    word8IMM2[0] = 0;
    word8IMMu = word8IMM2.to_ulong();
    fixPCB1 = gbaREG.rNUM[15] + 4;
    fixPCB1[1] = 0;
    fixPCB1u = fixPCB1.to_ulong();
    gbaREG.rNUM[opRD] = readMem(2, fixPCB1u + word8IMMu);
    gbaREG.rNUM[15] += 2;
}
uint8_t thumboffset5;
std::bitset<5> thumbOffset5B;
uint32_t ZOriginal;
void thumbMovLSL()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRN = opRDET3B.to_ulong();
    thumbOffset5B = currentThumbOpcode >> 6;
    thumboffset5 = thumbOffset5B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] << thumboffset5;

    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(gbaREG.rNUM[opRN], gbaREG.rNUM[opRD],0,thumboffset5);

    gbaREG.rNUM[15] += 2;
}
uint8_t conditionForBranch;
int8_t offsetToBranch2;
int16_t offsetToBranch;
std::bitset<8> offsetBranchBit;
void doConditionBranch()
{
    offsetBranchBit = currentThumbOpcode;
    offsetToBranch2 = offsetBranchBit.to_ulong();
    gbaREG.rNUM[15] += offsetToBranch2;
    gbaREG.rNUM[15] += offsetToBranch2;
    gbaREG.rNUM[15] += 4;
}
void thumbConditionalBranch()
{
    opRDET = currentThumbOpcode >> 8;
    conditionForBranch = opRDET.to_ulong();
    switch(conditionForBranch)
    {
        case 0x0:
            if(gbaREG.cpsr[30] == 1)
            {
                doConditionBranch();
            }
            if(gbaREG.cpsr[30] == 0)
            {
                gbaREG.rNUM[15] += 2;
            }
        break;

        case 0x1:
            if(gbaREG.cpsr[30] == 0)
            {
                doConditionBranch();
            }
            if(gbaREG.cpsr[30] == 1)
            {
                gbaREG.rNUM[15] += 2;
            }
        break;

        case 0x2:
            if(gbaREG.cpsr[29] == 1)
            {
                doConditionBranch();
            }
            if(gbaREG.cpsr[29] == 0)
            {
                gbaREG.rNUM[15] += 2;
            }
        break;

        default:
            printf("UNIMPLEMENTED CONDITION FOR BRANCH 0x%X!\n",conditionForBranch);
            opcodeError = true;
        break;
    }
}
std::bitset<8> immed82;
uint8_t offset8Immed;
void thumbMov8Immediate()
{
    immed82 = currentThumbOpcode;
    offset8Immed = immed82.to_ulong();
    opRDET3B = currentThumbOpcode >> 8;
    opRD = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = offset8Immed;

    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0); // Using Operation Auto Set Zero here, so there is no need to put anything where the zeros are.
    gbaREG.rNUM[15] += 2;
    //breakpoint = true;
}
std::bitset<1> offsetBit;
std::bitset<11> offset11;
std::bitset<32> setB0LR;
uint32_t backupForPCLongBranch;
uint16_t afterThisOP;
uint32_t offset11u;
std::bitset<23> B23VAR;
std::bitset<32> B32VAR;
std::bitset<1> saveTopBit;
int32_t offset11uN;
uint8_t loopLongB;
void thumbLongBranchWithLink()
{
    offsetBit = currentThumbOpcode >> 11;
    if(offsetBit[0] == 0)
    {
        offset11 = currentThumbOpcode;
        B23VAR = offset11.to_ulong();
        B23VAR = B23VAR << 12;
        //offset11u = test23.to_ulong();
        saveTopBit[0] = B23VAR[22];
        B23VAR[22] = 0;
        B32VAR = B23VAR.to_ulong();
        if(saveTopBit[0] == 1)
        {
            loopLongB = 31;
            while(loopLongB != 21)
            {
                B32VAR[loopLongB] = 1;
                loopLongB--;
            }
        }
        offset11uN = B32VAR.to_ulong();
        printf("0x%i\n",offset11uN);
        gbaREG.rNUM[14] = (gbaREG.rNUM[15] + offset11uN);
        printf("CUrrent R14: 0x%X\n",gbaREG.rNUM[14]);


        afterThisOP = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 3] << 8 |
                      gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 2];
        offset11 = afterThisOP;
        gbaREG.rNUM[14] += (offset11.to_ulong()) << 1;
        backupForPCLongBranch = gbaREG.rNUM[15];
        gbaREG.rNUM[15] = gbaREG.rNUM[14] + 4;
        gbaREG.rNUM[14] = backupForPCLongBranch + 4;
        setB0LR = gbaREG.rNUM[14];
        setB0LR[0] = 1;
        gbaREG.rNUM[14] = setB0LR.to_ulong();
    }
    if(offsetBit[0] == 1)
    {
        opcodeError = true;
        printf("Test2\n");
    }
}
void thumbOPADDorSUB()
{
    op16bit = currentThumbOpcode;
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 6;
    opRN = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    if(op16bit[10] == 0 && op16bit[9] == 0)
    {
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRS] + gbaREG.rNUM[opRN];
        handleCarryFlag(gbaREG.rNUM[opRS], gbaREG.rNUM[opRD],2,gbaREG.rNUM[opRN]);
    }
    if(op16bit[10] == 1 && op16bit[9] == 0)
    {
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRS] + opRN;
        handleCarryFlag(gbaREG.rNUM[opRS], gbaREG.rNUM[opRD],2,opRN);
    }
    if(op16bit[10] == 0 && op16bit[9] == 1)
    {
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRS] - gbaREG.rNUM[opRN];
        handleCarryFlag(gbaREG.rNUM[opRS], gbaREG.rNUM[opRD],3,gbaREG.rNUM[opRN]);
    }
    if(op16bit[10] == 1 && op16bit[9] == 1)
    {
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRS] - opRN;
        handleCarryFlag(gbaREG.rNUM[opRS], gbaREG.rNUM[opRD],3,opRN);
    }
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    gbaREG.rNUM[15] += 2;
}
void thumbOPBIC()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] & ~gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
std::bitset<8> regWriteList;
uint8_t MIAWriteLoopReg;
void thumbOPSTMIAorLDMIA()
{
    MIAWriteLoopReg = 0;
    opRDET3B = currentThumbOpcode >> 8;
    opRB = opRDET3B.to_ulong();
    regWriteList = currentThumbOpcode;
    op16bit = currentThumbOpcode;
    if(op16bit[11] == 0)
    {
        while(MIAWriteLoopReg != 8)
        {
            if(regWriteList[MIAWriteLoopReg] == 1)
            {
                writeMem(2,gbaREG.rNUM[opRB],gbaREG.rNUM[MIAWriteLoopReg]);
                gbaREG.rNUM[opRB] += 4;
            }
            MIAWriteLoopReg++;
        }
    }
    if(op16bit[11] == 1)
    {
        while(MIAWriteLoopReg != 8)
        {
            if(regWriteList[MIAWriteLoopReg] == 1)
            {
                gbaREG.rNUM[MIAWriteLoopReg] = readMem(2,gbaREG.rNUM[opRB]);
                gbaREG.rNUM[opRB] += 4;
            }
            MIAWriteLoopReg++;
        }
    }
    gbaREG.rNUM[15] += 2;
}
void thumbOP001SUB()
{
    immed82 = currentThumbOpcode;
    offset8Immed = immed82.to_ulong();
    opRDET3B = currentThumbOpcode >> 8;
    opRD = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];

    gbaREG.rNUM[opRD] -= offset8Immed;

    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(ZOriginal,gbaREG.rNUM[opRD],3,offset8Immed);
    gbaREG.rNUM[15] += 2;
    //breakpoint = true;
}
void thumbOP001ADD()
{
    immed82 = currentThumbOpcode;
    offset8Immed = immed82.to_ulong();
    opRDET3B = currentThumbOpcode >> 8;
    opRD = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];

    gbaREG.rNUM[opRD] += offset8Immed;

    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(ZOriginal,gbaREG.rNUM[opRD],2,offset8Immed);
    gbaREG.rNUM[15] += 2;
    //breakpoint = true;
}
uint32_t comparethumb001CMP;
void thumbOP001CMP()
{
    immed82 = currentThumbOpcode;
    offset8Immed = immed82.to_ulong();
    opRDET3B = currentThumbOpcode >> 8;
    opRD = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    comparethumb001CMP = gbaREG.rNUM[opRD];
    comparethumb001CMP -= offset8Immed;

    handleSignFlag(comparethumb001CMP);
    handleZeroFlag(ZOriginal, comparethumb001CMP);
    handleCarryFlag(ZOriginal,comparethumb001CMP,3,offset8Immed);
    gbaREG.rNUM[15] += 2;
    //breakpoint = true;
}
std::bitset<32> checkB0forSwitch;
void thumbOPBranchOrExchange()
{
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    op16bit = currentThumbOpcode;
    if(op16bit[6] == 1)
    {
        opRS += 8;
    }
    checkB0forSwitch = gbaREG.rNUM[opRS];
    gbaREG.cpsr[5] = checkB0forSwitch[0];
    checkB0forSwitch[0] = 0;
    gbaREG.rNUM[15] = checkB0forSwitch.to_ulong();
}
std::bitset<5> offset5;
uint8_t offset5u;
uint32_t addValforSTRorLDR;
void thumbOP001strORldr()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRB = opRDET3B.to_ulong();
    offset5 = currentThumbOpcode >> 6;
    op16bit = currentThumbOpcode;
    if(op16bit[12] == 0)
    {
        printf("NOTE!  THIS IS POTENTIALLY INCORRECT IN 001LDRSTRTHUMB\n");
        offset5[1] = 0;
        offset5[0] = 0;
    }
    offset5u = offset5.to_ulong();
    addValforSTRorLDR = gbaREG.rNUM[opRB] + offset5u;
    if(op16bit[12] == 0 && op16bit[11] == 0)
    {
        writeMem(2,addValforSTRorLDR,gbaREG.rNUM[opRD]);
    }
    if(op16bit[12] == 0 && op16bit[11] == 1)
    {
        gbaREG.rNUM[opRD] = readMem(2,addValforSTRorLDR);
    }
    if(op16bit[12] == 1 && op16bit[11] == 0)
    {
        writeMem(0,addValforSTRorLDR,gbaREG.rNUM[opRD]);
    }
    if(op16bit[12] == 1 && op16bit[11] == 1)
    {
        gbaREG.rNUM[opRD] = readMem(0,addValforSTRorLDR);
    }
    gbaREG.rNUM[15] += 2;
}

void thumbOPPushPopReg()
{
    printf("PUSH POP\n");
    MIAWriteLoopReg = 0;
    regWriteList = currentThumbOpcode;
    op16bit = currentThumbOpcode;
    if(op16bit[8] == 1 && op16bit[11] == 0)
    {
        gbaREG.rNUM[13] -= 4;
        writeMem(2,gbaREG.rNUM[13],gbaREG.rNUM[14]);
    }
    if(op16bit[11] == 0)
    {
        while(MIAWriteLoopReg != 8)
        {
            if(regWriteList[MIAWriteLoopReg] == 1)
            {
                gbaREG.rNUM[13] -= 4;
                writeMem(2,gbaREG.rNUM[13],gbaREG.rNUM[MIAWriteLoopReg]);
            }
            MIAWriteLoopReg++;
        }
    }
    if(op16bit[11] == 1)
    {
        MIAWriteLoopReg = 7;
        while(MIAWriteLoopReg != 0xFF)
        {
            if(regWriteList[MIAWriteLoopReg] == 1)
            {
                gbaREG.rNUM[MIAWriteLoopReg] = readMem(2,gbaREG.rNUM[13]);
                gbaREG.rNUM[13] += 4;
            }
            MIAWriteLoopReg--;
        }
        MIAWriteLoopReg = 0;
    }
    if(op16bit[8] == 1 && op16bit[11] == 1)
    {
        gbaREG.rNUM[14] = readMem(2,gbaREG.rNUM[13]);
        gbaREG.rNUM[13] += 4;
    }
    gbaREG.rNUM[15] += 2;
}
void thumbMovLSR()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRN = opRDET3B.to_ulong();
    thumbOffset5B = currentThumbOpcode >> 6;
    thumboffset5 = thumbOffset5B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRN] >> thumboffset5;

    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);

    gbaREG.rNUM[15] += 2;
}
int32_t ASRtemp;
void thumbMovASR()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRN = opRDET3B.to_ulong();
    thumbOffset5B = currentThumbOpcode >> 6;
    thumboffset5 = thumbOffset5B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    ASRtemp = ZOriginal;
    ASRtemp = ASRtemp >> thumboffset5;
    gbaREG.rNUM[opRD] = ASRtemp;

    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);

    gbaREG.rNUM[15] += 2;
    //breakpoint = true;
}
void thumbOPMoveHI()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    op16bit = currentThumbOpcode;
    if(op16bit[6] == 1)
    {
        opRS += 8;
    }
    if(op16bit[7] == 1)
    {
        opRD += 8;
    }
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRS];
    gbaREG.rNUM[15] += 2;
}
std::bitset<2> op000Thumb;
std::bitset<4> op01000Thumb;
uint8_t op000Thumbu;
uint8_t op01000Thumbu;
void doThumbOpcode(uint16_t opcode)
{
    op16bit = opcode;
    switch(op16bit[15])
    {
        case 0: // 0
            switch(op16bit[14])
            {
                case 0: // 00
                    switch(op16bit[13])
                    {
                        case 0:
                            op000Thumb = currentThumbOpcode >> 11;
                            op000Thumbu = op000Thumb.to_ulong();
                            switch(op000Thumbu)
                            {
                                case 0x0: // LSL
                                    thumbMovLSL();
                                break;

                                case 0x1: // LSR
                                    thumbMovLSR();
                                break;

                                case 0x2: // ASR
                                    thumbMovASR();
                                break;

                                case 0x3: // ADD or SUB
                                    thumbOPADDorSUB();
                                break;

                                default:
                                    printf("Error!  This should not happen!\n");
                                    opcodeError = true;
                                break;
                            }
                        break;

                        case 1: // 001
                            op000Thumb = currentThumbOpcode >> 11;
                            op000Thumbu = op000Thumb.to_ulong();
                            switch(op000Thumbu)
                            {
                                case 0:
                                    thumbMov8Immediate();
                                break;

                                case 1:
                                    thumbOP001CMP();
                                break;

                                case 2:
                                    thumbOP001ADD();
                                break;

                                case 3:
                                    thumbOP001SUB();
                                break;

                                default:
                                    printf("IF THIS SHOWS UP, SOMETHING IS VERY BROKEN!\n");
                                    opcodeError = true;
                                break;
                            }
                        break;
                    }
                break;

                case 1: // 01
                    switch(op16bit[13])
                    {
                        case 0: // 010
                            switch(op16bit[12])
                            {
                                case 0: // 0100
                                    switch(op16bit[11])
                                    {
                                        case 0: // 01000
                                            switch(op16bit[10])
                                            {
                                                case 0:
                                                    op01000Thumb = currentThumbOpcode >> 6;
                                                    op01000Thumbu = op01000Thumb.to_ulong();
                                                    switch(op01000Thumbu)
                                                    {
                                                        case 0xE:
                                                            thumbOPBIC();
                                                        break;

                                                        default:
                                                            printf("UNIMPLEMENTED 010000 THUMB OPCODE!\n");
                                                            opcodeError = true;
                                                        break;
                                                    }
                                                break;

                                                case 1:
                                                    op000Thumb = currentThumbOpcode >> 8;
                                                    op000Thumbu = op000Thumb.to_ulong();
                                                    switch(op000Thumbu)
                                                    {
                                                        case 2:
                                                            thumbOPMoveHI();
                                                        break;

                                                        case 3:
                                                            thumbOPBranchOrExchange();
                                                        break;

                                                        default:
                                                            printf("UNIMPLEMENTED 010001 THUMB OPCODE!\n");
                                                            opcodeError = true;
                                                        break;
                                                    }
                                                break;
                                            }
                                        break;

                                        case 1: // 01001 PC RELATIVE LOAD
                                            pcRelativeLoad();
                                        break;
                                    }
                                break;

                                case 1: // 0101
                                    printf("Unimplemented Thumb Instruction 0101END!\n");
                                    opcodeError = true;
                                break;
                            }
                        break;

                        case 1: // 011
                            thumbOP001strORldr();
                        break;
                    }
                break;
            }
        break;

        case 1: // 1
            switch(op16bit[14])
            {
                case 0: // 10
                    switch(op16bit[13])
                    {
                        case 0: // 100
                            printf("Unimplemented Thumb Instruction 100END!\n");
                            opcodeError = true;
                        break;

                        case 1: // 101
                            switch(op16bit[12])
                            {
                                case 0: // 1010
                                    printf("Unimplemented Thumb Instruction 1010END!\n");
                                    opcodeError = true;
                                break;

                                case 1: // 1011
                                    switch(op16bit[10])
                                    {
                                        case 0: // 1011X0
                                            printf("Unimplemented Thumb Instruction 1011X0END!\n");
                                            opcodeError = true;
                                        break;

                                        case 1: // 1011X1
                                            thumbOPPushPopReg();
                                        break;
                                    }
                                break;
                            }
                        break;
                    }
                break;

                case 1: // 11
                    switch(op16bit[13])
                    {
                        case 0: // 110
                            switch(op16bit[12])
                            {
                                case 0: // 1100
                                    thumbOPSTMIAorLDMIA();
                                break;

                                case 1: // 1101
                                    thumbConditionalBranch();
                                break;
                            }
                        break;

                        case 1: // 111
                            switch(op16bit[12])
                            {
                                case 0:
                                printf("Unimplemented Thumb 1110!\n");
                                opcodeError = true;
                                break;

                                case 1: // 1111
                                    thumbLongBranchWithLink();
                                break;
                            }
                        break;
                    }
                break;
            }
        break;
    }
}