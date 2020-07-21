#include "include.hpp"
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
char romName[50];
int romSize;
void loadROM()
{
    printf("Welcome to nopGBA!\nEnter Rom Name: ");
    scanf("%s",romName);
    FILE* rom = fopen(romName,"r");
    if(rom == NULL)
    {
        printf("Error! Rom not found!\n");
        loadROM();
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
}
//          001 Data Opcodes
uint8_t op001code;
std::bitset<32> rotation8mov;
std::bitset<4> rotatebitAmount;
bool rotation8back;
uint8_t rotationMovAmount;
uint8_t rMovAmBack;
uint8_t movIntoReg;
void movOP() // 001-1101
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
        printf("Handle Flag Stub for 001-1101!\n");
    }
    if(rMovAmBack != 0)
    {
        printf("Does Carry 001-1101 Work?\n");
        gbaREG.cpsr[29] = rotation8mov[31];
    }
    gbaREG.rNUM[15] += 4;
}
void loadStoreOp()
{
    printf("LOAD OR STORE!\n");
    uint32_t lsAddr;
    std::bitset<12> lsAddBit = currentOpcode;
    lsAddr = lsAddBit.to_ulong();
    std::bitset<4> lsAddBit2 = currentOpcode >> 16;
    uint8_t RnLS = lsAddBit2.to_ulong();
    std::bitset<32> lsAddBit3 = currentOpcode;
    bool loadStore = lsAddBit3[20];
    bool byteWordLS = lsAddBit3[22];
    uint8_t PWLSMode = lsAddBit3[24] << 1 | lsAddBit3[21];
    if(PWLSMode == 0x02)
    {
        if(lsAddBit3[23] == 1)
        {
            lsAddr += gbaREG.rNUM[RnLS];
        }
        if(lsAddBit3[23] == 0)
        {
            lsAddr -= gbaREG.rNUM[RnLS];
        }
    }
    if(PWLSMode != 0x02)
    {
        opcodeError = true;
        printf("Addressing PWLSMode for Load/Store UNKNOWN 0x%X\n",PWLSMode);
    }
    lsAddBit2 = currentOpcode >> 12;
    uint8_t RdLS = lsAddBit2.to_ulong();
    printf("lsAddr: 0x%02X\n",lsAddr);
    if(byteWordLS == 0 && loadStore == 0)
    {
        writeMem(2,lsAddr,gbaREG.rNUM[RdLS]);
    }
    if(byteWordLS == 1 && loadStore == 0)
    {
        writeMem(0,lsAddr,gbaREG.rNUM[RdLS]);
    }
    if(loadStore == 1)
    {
        opcodeError = true;
        printf("load Not implemented.\n");
    }
    gbaREG.rNUM[15] += 4;
}
bool linkBranch;
int32_t branchOffset;
void branchOp() // 0x5
{
    op32bit = currentOpcode;
    linkBranch = op32bit[24];
    if(linkBranch == true)
    {
        gbaREG.rNUM[14] = gbaREG.rNUM[15] + 4;
    }
    op32bit = op32bit << 2;
    op32bit[31] = op32bit[25];
    op32bit[25] = 0;
    op32bit[26] = 0;
    op32bit[27] = 0;
    op32bit[28] = 0;
    op32bit[29] = 0;
    op32bit[30] = 0;
    branchOffset = op32bit.to_ulong();
    //printf("branchOffset: %X\n",branchOffset);
    gbaREG.rNUM[15] += branchOffset + 8;
    //printf("PC: 0x%02X\n",gbaREG.rNUM[15]);
}
std::bitset<32> bitOpcode;
std::bitset<1> currentOpcodeBit;
std::bitset<4> opcode001;
std::bitset<4> check11to8;
uint8_t opcode0018B;
void dataProcessingORpsrTransfer(uint32_t opcode)
{
    opcode001 = opcode >> 21;
    opcode0018B = opcode001.to_ulong();
    switch(opcode0018B) // XXXX001????
    {
        case 0xD:
            movOP();
        break;

        default:
            printf("UNIMPLEMENTED OP XXXX00? then 0x%X!\n",opcode0018B);
            std::cout<<"Opcode: "<<bitOpcode<<std::endl;
            opcodeError = true;
        break;
    }
}
void findAndDoOpcode(uint32_t opcode)
{
    // This will likely become a mess, but in the interest of just getting the CPU working, i'm going to use it until it works.
    // This code looks awful, but it's a lot better than the last implementation.
    bitOpcode = opcode;
    currentOpcodeBit = opcode >> 27;
    //std::cout<<currentOpcodeBit<<std::endl;
    switch(currentOpcodeBit[0])
    {
        case 0:
            currentOpcodeBit = opcode >> 26;
            switch(currentOpcodeBit[0])
            {
                case 0:
                    currentOpcodeBit = opcode >> 25;
                    switch(currentOpcodeBit[0])
                    {
                        case 0:
                            currentOpcodeBit = opcode >> 4;
                            switch(currentOpcodeBit[0])
                            {
                                case 0:
                                    dataProcessingORpsrTransfer(opcode);
                                break;

                                case 1:
                                    currentOpcodeBit = opcode >> 7;
                                    switch(currentOpcodeBit[0])
                                    {
                                        case 0:
                                            check11to8 = opcode >> 8;
                                            switch(check11to8.to_ulong())
                                            {
                                                case 0xF:
                                                    printf("UNIMPLEMENTED OP XXXX000 B7 = 0 & B4 = 1 & B8-11 = 1!\n");
                                                    std::cout<<"Opcode: "<<bitOpcode<<std::endl;
                                                    opcodeError = true;
                                                break;

                                                default:
                                                    dataProcessingORpsrTransfer(opcode);
                                                break;
                                            }
                                        break;

                                        case 1:
                                            printf("UNIMPLEMENTED OP XXXX000 B7 = 1 & B4 = 1!\n");
                                            std::cout<<"Opcode: "<<bitOpcode<<std::endl;
                                            opcodeError = true;
                                        break;
                                    }
                                break;
                            }
                        break;

                        case 1: // XXXX001
                            dataProcessingORpsrTransfer(opcode);
                        break;
                    }
                break;

                case 1:
                    currentOpcodeBit = opcode >> 25;
                    switch(currentOpcodeBit[0])
                    {
                        case 0:
                            loadStoreOp();
                        break;

                        case 1: // XXX011  if bit 4 is 1, undefined, otherwise loadstore.
                            currentOpcodeBit = opcode >> 4;
                            switch(currentOpcodeBit[0])
                            {
                                case 0:
                                    loadStoreOp();
                                break;

                                case 1:
                                    printf("UNDEFINED OP XXX011 with B4 = 1!\n");
                                    std::cout<<"Opcode: "<<bitOpcode<<std::endl;
                                    opcodeError = true;
                                break;
                            }
                        break;
                    }
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
                            printf("UNIMPLEMENTED OP XXXX100END!\n");
                            std::cout<<"Opcode: "<<bitOpcode<<std::endl;
                            opcodeError = true;
                        break;

                        case 1:
                            branchOp(); // XXX101END
                        break;
                    }
                break;

                case 1:
                    printf("UNIMPLEMENTED OP XXXX11END!\n");
                    std::cout<<"Opcode: "<<bitOpcode<<std::endl;
                    opcodeError = true;
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

