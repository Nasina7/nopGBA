#include "rendering.hpp"
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
std::bitset<33> resultBitset;
void handleCarryFlag(uint64_t original, uint64_t result, uint8_t operation, uint64_t valueUsed)
{
    /*
        Operation Values
        0 = Logical Shift Left
        1 = Auto Set Zero
        2 = Add
        3 = Subtract
        4 = Logical Shift Right
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
            //printf("orig: 0x%X\n",original);
            //printf("valUse: 0x%X\n",valueUsed);
            /*
            resultBitset = original + valueUsed;  IS THIS CORRECT?
            gbaREG.cpsr[29] = resultBitset[32];
            */
            /*
            result64 = original + valueUsed;
            if(result64 < original)
            {
                gbaREG.cpsr[29] = 1;
            }
            if(result64 >= original)
            {
                gbaREG.cpsr[29] = 0;
            }
            */
            result64 = original + valueUsed;
            if(result64 > 0xFFFFFFFF)
            {
                gbaREG.cpsr[29] = 1;
            }
            if(result64 <= 0xFFFFFFFF)
            {
                gbaREG.cpsr[29] = 0;
            }
            //gbaREG.cpsr[29] = 0;
            //if(result < original)
            //{
            //    gbaREG.cpsr[29] = 1;
            //}
            /*
            if(((original ^ valueUsed ^ result) & 0x100000000) == 0x100000000)
            {
                gbaREG.cpsr[29] = 1;
            }
            if(((original ^ valueUsed ^ result) & 0x100000000) != 0x100000000)
            {
                gbaREG.cpsr[29] = 0;
            }
            */
        break;

        case 3:
            result64 = original - valueUsed;
            if(result64 > 0xFFFFFFFF)
            {
                gbaREG.cpsr[29] = 0;
            }
            if(result64 <= 0xFFFFFFFF)
            {
                gbaREG.cpsr[29] = 1;
            }
            //gbaREG.cpsr[29] = resultBitset[32];
            //gbaREG.cpsr.flip(29);
            /*
            if(((original ^ valueUsed ^ result) & 0x100000000) == 0x100000000)
            {
                gbaREG.cpsr[29] = 0;
            }
            if(((original ^ valueUsed ^ result) & 0x100000000) != 0x100000000)
            {
                gbaREG.cpsr[29] = 1;
            }
            */
        break;

        case 4:
            gbaREG.cpsr[29] = 0;
            if(result == 0)
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
int64_t result64T;
int32_t result32T;
void handleOverflowFlag(int32_t original, int32_t result, uint8_t operation, int32_t valueUsed)
{
    /*
        Operation Values
        0 = Logical Shift Left
        1 = Auto Set Zero
        2 = Add
        3 = Subtract
        4 = Logical Shift Right
    */
    switch(operation)
    {
        case 0:
            gbaREG.cpsr[28] = 0;
            if(valueUsed > 0)
            {
                gbaREG.cpsr[28] = 1;
            }
        break;

        case 1:
            gbaREG.cpsr[28] = 0;
        break;

        case 2:
            result64T = original + valueUsed;
            gbaREG.cpsr[28] = 0;
            if(original > 0 && result64T < 0)
            {
                gbaREG.cpsr[28] = 1;
            }
        break;

        case 3:
            result32T = original;
            result32T -= valueUsed;
            gbaREG.cpsr[28] = 0;
            /*
            if(result64T < -2147483648)
            {
                gbaREG.cpsr[28] = 1;
            }
            */
            if(result32T > original)
            {
                gbaREG.cpsr[28] = 1;
            }
            //gbaREG.cpsr[28] = 0;
            //if(result > original)
            //{
            //    gbaREG.cpsr[28] = 1;
            //}
        break;

        case 4:
            gbaREG.cpsr[28] = 0;
            if(result == 0)
            {
                gbaREG.cpsr[28] = 1;
            }
        break;

        default:
            printf("Unimplemented Carry Flag Operation!\n");
            opcodeError = true;
        break;
    }
}
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
void doSoftwareInterrupt()
{
    //breakpoint = true;
    gbaREG.R1314_svc[1] = gbaREG.rNUM[15] + 2;
    //gbaREG.R1314_svc[0] = gbaREG.rNUM[13];
    gbaREG.spsr_svc = gbaREG.cpsr.to_ulong();
    gbaREG.rNUM[15] = 0x8;
    gbaREG.cpsr[5] = 0;
    gbaREG.cpsr[4] = 1;
    gbaREG.cpsr[3] = 0;
    gbaREG.cpsr[2] = 0;
    gbaREG.cpsr[1] = 1;
    gbaREG.cpsr[0] = 1;
    //breakpoint = true;
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

        case 0x3:
            if(gbaREG.cpsr[29] == 0)
            {
                doConditionBranch();
            }
            if(gbaREG.cpsr[29] == 1)
            {
                gbaREG.rNUM[15] += 2;
            }
        break;

        case 0x4:
            if(gbaREG.cpsr[31] == 1)
            {
                doConditionBranch();
            }
            if(gbaREG.cpsr[31] == 0)
            {
                gbaREG.rNUM[15] += 2;
            }
        break;

        case 0x5:
            if(gbaREG.cpsr[31] == 0)
            {
                doConditionBranch();
            }
            if(gbaREG.cpsr[31] == 1)
            {
                gbaREG.rNUM[15] += 2;
            }
        break;

        case 0x8:
            if(gbaREG.cpsr[29] == 1 && gbaREG.cpsr[30] == 0)
            {
                doConditionBranch();
                break;
            }
            gbaREG.rNUM[15] += 2;
        break;

        case 0x9:
            if(gbaREG.cpsr[29] == 0 || gbaREG.cpsr[30] == 1)
            {
                doConditionBranch();
                break;
            }
            gbaREG.rNUM[15] += 2;
        break;

        case 0xA:
            printf("WARN 2!\n");
            if(gbaREG.cpsr[31] == gbaREG.cpsr[28])
            {
                doConditionBranch();
            }
            if(gbaREG.cpsr[31] != gbaREG.cpsr[28])
            {
                gbaREG.rNUM[15] += 2;
            }
        break;

        case 0xB:
            printf("WARN B!\n");
            if(gbaREG.cpsr[31] == 1 && gbaREG.cpsr[28] == 0)
            {
                doConditionBranch();
                break;
            }
            if(gbaREG.cpsr[31] == 0 && gbaREG.cpsr[28] == 1)
            {
                doConditionBranch();
                break;
            }
            gbaREG.rNUM[15] += 2;
        break;

        case 0xC:
            printf("WARN C!\n");
            if(gbaREG.cpsr[31] == 1 && gbaREG.cpsr[28] == 1 && gbaREG.cpsr[30] == 0)
            {
                doConditionBranch();
                break;
            }
            if(gbaREG.cpsr[31] == 0 && gbaREG.cpsr[28] == 0 && gbaREG.cpsr[30] == 0)
            {
                doConditionBranch();
                break;
            }
            gbaREG.rNUM[15] += 2;
        break;

        case 0xD:
            printf("WARN D!\n");
            if(gbaREG.cpsr[30] == 1)
            {
                doConditionBranch();
                break;
            }
            if(gbaREG.cpsr[31] == 1 && gbaREG.cpsr[28] == 0)
            {
                doConditionBranch();
                break;
            }
            if(gbaREG.cpsr[31] == 0 && gbaREG.cpsr[28] == 1)
            {
                doConditionBranch();
                break;
            }
            gbaREG.rNUM[15] += 2;
        break;

        case 0xF:
            offsetBranchBit = currentThumbOpcode;
            printf("SOFTWARE IS ATTEMPTING BIOS CALL 0x%X!\n",offsetBranchBit.to_ulong());
            //printRegs();
            if(offsetBranchBit.to_ulong() == 0xB)
            {
                printf("CPUSET!\nSource Address: 0x%X\nDestination Address: 0x%X\nLM: 0x%X\n",
                       gbaREG.rNUM[0],
                       gbaREG.rNUM[1],
                       gbaREG.rNUM[2]);
                if(gbaREG.rNUM[1] == 0x5DCE0B4)
                {
                    breakpoint = true;
                }
            }
            if(offsetBranchBit.to_ulong() == 0xC)
            {
                printf("CPUSET!\nSource Address: 0x%X\nDestination Address: 0x%X\nLM: 0x%X\n",
                       gbaREG.rNUM[0],
                       gbaREG.rNUM[1],
                       gbaREG.rNUM[2]);
            }
            doSoftwareInterrupt();
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
        //printf("0x%i\n",offset11uN);
        gbaREG.rNUM[14] = (gbaREG.rNUM[15] + offset11uN);
        //printf("CUrrent R14: 0x%X\n",gbaREG.rNUM[14]);

        afterThisOP = readMem(1,gbaREG.rNUM[15] + 2);
        //afterThisOP = gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 3] << 8 |
        //              gbaRAM.flashROM[(gbaREG.rNUM[15] - 0x08000000) + 2];
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
        printf("ThumbLongBranchWithLinkError\n");
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
    handleOverflowFlag(ZOriginal,comparethumb001CMP,3,offset8Immed);
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
    //if(op16bit[12] == 0)
    //{
    //    printf("NOTE!  THIS IS POTENTIALLY INCORRECT IN 001LDRSTRTHUMB\n");
    //    offset5[1] = 0;
    //    offset5[0] = 0;
    //}
    offset5u = offset5.to_ulong();
    if(op16bit[12] == 0)
    {
        offset5u = offset5u << 2;
    }
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
std::bitset<32> fixPOPPC;
void thumbOPPushPopReg()
{
    //printf("PUSH POP\n");
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
        MIAWriteLoopReg = 7;
        while(MIAWriteLoopReg != 0xFF)
        {
            if(regWriteList[MIAWriteLoopReg] == 1)
            {
                gbaREG.rNUM[13] -= 4;
                writeMem(2,gbaREG.rNUM[13],gbaREG.rNUM[MIAWriteLoopReg]);
            }
            MIAWriteLoopReg--;
        }
    }
    if(op16bit[11] == 1)
    {
        MIAWriteLoopReg = 0;
        while(MIAWriteLoopReg != 0x8)
        {
            if(regWriteList[MIAWriteLoopReg] == 1)
            {
                gbaREG.rNUM[MIAWriteLoopReg] = readMem(2,gbaREG.rNUM[13]);
                gbaREG.rNUM[13] += 4;
            }
            MIAWriteLoopReg++;
        }
        MIAWriteLoopReg = 0;
    }
    if(op16bit[8] == 1 && op16bit[11] == 1)
    {
        //printf("POP PC!\n");
        fixPOPPC = readMem(2,gbaREG.rNUM[13]);
        fixPOPPC[0] = 0;
        gbaREG.rNUM[15] = fixPOPPC.to_ulong();
        gbaREG.rNUM[15] -= 2;
        gbaREG.rNUM[13] += 4;
    }
    gbaREG.rNUM[15] += 2;
}
std::bitset<32> origValBit;
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
    origValBit = ZOriginal;
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal, gbaREG.rNUM[opRD]);
    handleCarryFlag(ZOriginal,gbaREG.rNUM[opRD],4,thumboffset5);
    //gbaREG.cpsr[29] = origValBit[thumboffset5];
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
std::bitset<32> fixThumbMovHI;
uint32_t cmpResultThumb;
void thumbOP010001CMP()
{
    op16bit = currentThumbOpcode;

    opRDET = currentThumbOpcode;
    opRDET[3] = op16bit[7];
    opRD = opRDET.to_ulong();
    opRDET = currentThumbOpcode >> 3;
    opRDET[3] = op16bit[6];
    opRS = opRDET.to_ulong();
    handleCarryFlag(gbaREG.rNUM[opRD],gbaREG.rNUM[opRD] - gbaREG.rNUM[opRS],3,gbaREG.rNUM[opRS]);
    handleOverflowFlag(gbaREG.rNUM[opRD],gbaREG.rNUM[opRD] - gbaREG.rNUM[opRS],3,gbaREG.rNUM[opRS]);
    cmpResultThumb = gbaREG.rNUM[opRD] - gbaREG.rNUM[opRS];
    handleSignFlag(cmpResultThumb);
    handleZeroFlag(gbaREG.rNUM[opRD],cmpResultThumb);
    gbaREG.rNUM[15] += 2;
}
void thumbOP010001ADD()
{
    op16bit = currentThumbOpcode;

    opRDET = currentThumbOpcode;
    opRDET[3] = op16bit[7];
    opRD = opRDET.to_ulong();
    opRDET = currentThumbOpcode >> 3;
    opRDET[3] = op16bit[6];
    opRS = opRDET.to_ulong();
    comparethumb001CMP = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRS] + gbaREG.rNUM[opRD];
    handleCarryFlag(comparethumb001CMP,gbaREG.rNUM[opRD],2,gbaREG.rNUM[opRS]);
    handleOverflowFlag(gbaREG.rNUM[opRD],gbaREG.rNUM[opRD] - gbaREG.rNUM[opRS],2,gbaREG.rNUM[opRS]);
    cmpResultThumb = gbaREG.rNUM[opRD] + gbaREG.rNUM[opRS];
    handleSignFlag(cmpResultThumb);
    handleZeroFlag(comparethumb001CMP,cmpResultThumb);
    gbaREG.rNUM[15] += 2;
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
    if(opRD != 0xF)
    {
        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRS];
    }
    if(opRD == 0xF)
    {
        fixThumbMovHI = gbaREG.rNUM[opRS];
        fixThumbMovHI[0] = 0;
        gbaREG.rNUM[opRD] = fixThumbMovHI.to_ulong();
    }
    gbaREG.rNUM[15] += 2;
}
void thumbOP010000CMP()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    handleCarryFlag(gbaREG.rNUM[opRD],gbaREG.rNUM[opRD] - gbaREG.rNUM[opRS],3,gbaREG.rNUM[opRS]);
    handleOverflowFlag(gbaREG.rNUM[opRD],gbaREG.rNUM[opRD] - gbaREG.rNUM[opRS],3,gbaREG.rNUM[opRS]);
    cmpResultThumb = gbaREG.rNUM[opRD] - gbaREG.rNUM[opRS];
    handleSignFlag(cmpResultThumb);
    handleZeroFlag(gbaREG.rNUM[opRD],cmpResultThumb);
    gbaREG.rNUM[15] += 2;
}
void thumbOP010000MUL()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    handleCarryFlag(gbaREG.rNUM[opRD],gbaREG.rNUM[opRD] * gbaREG.rNUM[opRS],3,gbaREG.rNUM[opRS]);
    handleOverflowFlag(gbaREG.rNUM[opRD],gbaREG.rNUM[opRD] * gbaREG.rNUM[opRS],3,gbaREG.rNUM[opRS]);
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] * gbaREG.rNUM[opRS];
    handleSignFlag(cmpResultThumb);
    handleZeroFlag(gbaREG.rNUM[opRD],cmpResultThumb);
    gbaREG.rNUM[15] += 2;
}
void thumbOP010000TST()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    cmpResultThumb = gbaREG.rNUM[opRD] & gbaREG.rNUM[opRS];
    handleSignFlag(cmpResultThumb);
    handleZeroFlag(gbaREG.rNUM[opRD],cmpResultThumb);
    handleCarryFlag(gbaREG.rNUM[opRD],cmpResultThumb,3,gbaREG.rNUM[opRS]);
    gbaREG.rNUM[15] += 2;
}
void thumbOP010000CMN()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    cmpResultThumb = gbaREG.rNUM[opRD] + gbaREG.rNUM[opRS];
    handleSignFlag(cmpResultThumb);
    handleZeroFlag(gbaREG.rNUM[opRD],cmpResultThumb);
    handleCarryFlag(gbaREG.rNUM[opRD],cmpResultThumb,2,gbaREG.rNUM[opRS]);
    gbaREG.rNUM[15] += 2;
}
bool signVal;
uint16_t addValforStack;
std::bitset<7> getSWord7;
void thumbOPAddToStackPointer()
{
    getSWord7 = currentThumbOpcode;
    addValforStack = getSWord7.to_ulong();
    addValforStack = addValforStack << 2;
    getSWord7 = currentThumbOpcode >> 7;
    signVal = getSWord7[0];
    if(signVal == 1)
    {
        gbaREG.rNUM[13] -= addValforStack;
    }
    if(signVal == 0)
    {
        gbaREG.rNUM[13] += addValforStack;
    }
    gbaREG.rNUM[15] += 2;
}
uint32_t storeLocationOpHalfwordThumb;
void thumbOPLoadStoreHalfword()
{
    offset5 = currentThumbOpcode >> 6;
    offset5u = offset5.to_ulong();
    offset5u = offset5u << 1;
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRB = opRDET3B.to_ulong();
    storeLocationOpHalfwordThumb = gbaREG.rNUM[opRB] + offset5u;
    op16bit = currentThumbOpcode;
    lBit = op16bit[11];
    if(lBit == 1)
    {
        gbaREG.rNUM[opRD] = readMem(1,storeLocationOpHalfwordThumb);
    }
    if(lBit == 0)
    {
        writeMem(1,storeLocationOpHalfwordThumb,gbaREG.rNUM[opRD]);
    }
    gbaREG.rNUM[15] += 2;
}
void thumbOPNeg()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = 0 - gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
void thumbOPAND()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] & gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
void thumbOPMVN()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = ~gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
void thumbOPLSL()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] << gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
void thumbOPLSR()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] >> gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
void thumbOPASR()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = ASLvaluebyNum(gbaREG.rNUM[opRD],gbaREG.rNUM[opRS]);
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
void thumbOPOR()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] | gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
void thumbOPEOR()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRS = opRDET3B.to_ulong();
    ZOriginal = gbaREG.rNUM[opRD];
    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] ^ gbaREG.rNUM[opRS];
    handleSignFlag(gbaREG.rNUM[opRD]);
    handleZeroFlag(ZOriginal,gbaREG.rNUM[opRD]);
    handleCarryFlag(0,0,1,0);
    gbaREG.rNUM[15] += 2;
}
std::bitset<8> word8;
uint16_t word8u;
uint32_t SPrelLoadStoreLocate;
void thumbOPSPrelLoadStore()
{
    word8 = currentThumbOpcode;
    word8u = word8.to_ulong();
    word8u = word8u << 2;
    opRDET3B = currentThumbOpcode >> 8;
    opRD = opRDET3B.to_ulong();
    op16bit = currentThumbOpcode;
    SPrelLoadStoreLocate = gbaREG.rNUM[13] + word8u;
    if(op16bit[11] == 1)
    {
        gbaREG.rNUM[opRD] = readMem(2,SPrelLoadStoreLocate);
    }
    if(op16bit[11] == 0)
    {
        writeMem(2,SPrelLoadStoreLocate,gbaREG.rNUM[opRD]);
    }
    gbaREG.rNUM[15] += 2;
}
uint32_t fixPC1;
std::bitset<32> fixPC2;
void thumbOPloadAddress()
{
    word8 = currentThumbOpcode;
    word8u = word8.to_ulong();
    word8u = word8u << 2;
    opRDET3B = currentThumbOpcode >> 8;
    opRD = opRDET3B.to_ulong();
    op16bit = currentThumbOpcode;
    if(op16bit[11] == 0)
    {
        fixPC2 = gbaREG.rNUM[15] + 4;
        fixPC2[1] = 0;
        fixPC1 = fixPC2.to_ulong();
        SPrelLoadStoreLocate = fixPC1 + word8u;
    }
    if(op16bit[11] == 1)
    {
        SPrelLoadStoreLocate = gbaREG.rNUM[13] + word8u;
    }
    gbaREG.rNUM[opRD] = SPrelLoadStoreLocate;
    gbaREG.rNUM[15] += 2;
}
uint8_t LBoption;
void thumbOPloadStorewRegisterOffset()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRB = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 6;
    opRO = opRDET3B.to_ulong();
    op16bit = currentThumbOpcode;
    LBoption = op16bit[11] << 1 | op16bit[10];
    printf("This is probably incorrect\n");
    switch(LBoption)
    {
        case 0:
            writeMem(2,gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO],gbaREG.rNUM[opRD]);
        break;

        case 1:
            writeMem(0,gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO],gbaREG.rNUM[opRD]);
        break;

        case 2:
            gbaREG.rNUM[opRD] = readMem(2,gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO]);
        break;

        case 3:
            gbaREG.rNUM[opRD] = readMem(0,gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO]);
        break;
    }
    gbaREG.rNUM[15] += 2;

}
std::bitset<16> offset11B;
std::bitset<11> offset11B2;
int16_t offset11N;
void thumbOPUnconditionalBranch()
{
    offset11B2 = currentThumbOpcode;
    offset11B = offset11B2.to_ulong() << 1;
    if(offset11B[11] == 1)
    {
        offset11B[16] = 1;
        //offset11B[11] = 0;
        offset11B[15] = 1;
        offset11B[14] = 1;
        offset11B[13] = 1;
        offset11B[12] = 1;
    }
    offset11N = offset11B.to_ulong();
    gbaREG.rNUM[15] = gbaREG.rNUM[15] + offset11N;
    gbaREG.rNUM[15] += 4;
    //printf("BRANCH: %i\n",offset11N);
    //opcodeError = true;
}
void thumbOPLoadStoreSignExtendByteOrHalfword()
{
    opRDET3B = currentThumbOpcode;
    opRD = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 3;
    opRB = opRDET3B.to_ulong();
    opRDET3B = currentThumbOpcode >> 6;
    opRO = opRDET3B.to_ulong();
    switch(op16bit[10])
    {
        case 0: // Unsigned
            switch(op16bit[11])
            {
                case 0: // Unsigned Store
                    writeMem(1, gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO], gbaREG.rNUM[opRD]);
                break;

                case 1: // Unsigned Load
                    gbaREG.rNUM[opRD] = readMem(1,gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO]);
                    gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] & 0x0000FFFF;
                break;
            }
        break;

        case 1: // Signed
            switch(op16bit[11])
            {
                case 0: // Signed Load Byte
                    gbaREG.rNUM[opRD] = readMem(0,gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO]);
                    if(gbaREG.rNUM[opRD] & 0x80 != 0)
                    {
                        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] | 0xFFFFFF00;
                    }
                    printf("Are these supposed to be swapped?\n");
                break;

                case 1: // Signed Load Halfword
                    gbaREG.rNUM[opRD] = readMem(1,gbaREG.rNUM[opRB] + gbaREG.rNUM[opRO]);
                    if(gbaREG.rNUM[opRD] & 0x8000 != 0)
                    {
                        gbaREG.rNUM[opRD] = gbaREG.rNUM[opRD] | 0xFFFF0000;
                    }
                    printf("Are these supposed to be swapped?\n");
                break;
            }
        break;
    }
    gbaREG.rNUM[0xF] += 2;
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

                                                        case 0x0:
                                                            thumbOPAND();
                                                        break;

                                                        case 0x1:
                                                            thumbOPEOR();
                                                        break;

                                                        case 0x2:
                                                            thumbOPLSL();
                                                        break;

                                                        case 0x3:
                                                            thumbOPLSR();
                                                        break;

                                                        case 0x4:
                                                            thumbOPASR();
                                                        break;

                                                        case 0x8:
                                                            thumbOP010000TST();
                                                        break;

                                                        case 0x9:
                                                            thumbOPNeg();
                                                            //breakpoint = true;
                                                        break;

                                                        case 0xA:
                                                            thumbOP010000CMP();
                                                        break;

                                                        case 0xB:
                                                            thumbOP010000CMN();
                                                        break;

                                                        case 0xC:
                                                            thumbOPOR();
                                                        break;

                                                        case 0xD:
                                                            thumbOP010000MUL();
                                                        break;

                                                        case 0xE:
                                                            thumbOPBIC();
                                                        break;

                                                        case 0xF:
                                                            thumbOPMVN();
                                                        break;

                                                        default:
                                                            printf("UNIMPLEMENTED 010000 THUMB OPCODE 0x%X!\n",op01000Thumbu);
                                                            opcodeError = true;
                                                        break;
                                                    }
                                                break;

                                                case 1: // 010001
                                                    op000Thumb = currentThumbOpcode >> 8;
                                                    op000Thumbu = op000Thumb.to_ulong();
                                                    switch(op000Thumbu)
                                                    {
                                                        case 0:
                                                            thumbOP010001ADD();
                                                        break;

                                                        case 1:
                                                            thumbOP010001CMP();
                                                        break;

                                                        case 2:
                                                            thumbOPMoveHI();
                                                        break;

                                                        case 3:
                                                            thumbOPBranchOrExchange();
                                                        break;

                                                        default:
                                                            printf("UNIMPLEMENTED 010001 THUMB OPCODE 0x%X!\n",op000Thumbu);
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
                                    switch(op16bit[9])
                                    {
                                        case 0:
                                            thumbOPloadStorewRegisterOffset();
                                        break;

                                        case 1:
                                            thumbOPLoadStoreSignExtendByteOrHalfword();
                                        break;
                                    }
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
                            switch(op16bit[12])
                            {
                                case 0: // 1000
                                    thumbOPLoadStoreHalfword();
                                break;

                                case 1: // 1001
                                    thumbOPSPrelLoadStore();
                                break;
                            }
                        break;

                        case 1: // 101
                            switch(op16bit[12])
                            {
                                case 0: // 1010
                                    thumbOPloadAddress();
                                break;

                                case 1: // 1011
                                    switch(op16bit[10])
                                    {
                                        case 0: // 1011X0
                                            thumbOPAddToStackPointer();
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
                                    thumbOPUnconditionalBranch();
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
