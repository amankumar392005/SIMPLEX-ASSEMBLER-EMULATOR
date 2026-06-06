/*************************
TITLE: Emulator
AUTHOR: AMAN KUMAR
ROLL NO : 2401CS08
*************************/

#include <bits/stdc++.h>
using namespace std;


map<int, string> opcodeMap;


int mem[10000], regA = 0, regB = 0, PC = 0, SP = 9999;


void initOpcodeMap() {
    opcodeMap[0] = "ldc"; 
    opcodeMap[1] = "adc";
     opcodeMap[2] = "ldl"; 
     opcodeMap[3] = "stl";

    opcodeMap[4] = "ldnl";
     opcodeMap[5] = "stnl";
      opcodeMap[6] = "add"; 
      opcodeMap[7] = "sub";

    opcodeMap[8] = "shl";
     opcodeMap[9] = "shr"; 
     opcodeMap[10] = "adj";
      opcodeMap[11] = "a2sp";

    opcodeMap[12] = "sp2a";
     opcodeMap[13] = "call";
      opcodeMap[14] = "return";
       opcodeMap[15] = "brz";

    opcodeMap[16] = "brlz"; 
    opcodeMap[17] = "br";
     opcodeMap[18] = "HALT";

    opcodeMap[-1] = "data"; 
    opcodeMap[-2] = "SET";
}

// Convert string number of any base to integer

int convertBase(string &num, int start, int base) {

    if (base == 10) {

        int sign = 1;

        if (num[0] == '+' || num[0] == '-') start++;

        if (num[0] == '-') sign = -1;

        return sign * stoi(num.substr(start), 0, base);
    }
    return stoi(num, 0, base);
}

// Convert integer to 8-character hex string
string toHex(int n) {

    string hexChars = "0123456789abcdef";

    string res = "";

    for (int i = 0; i < 8; i++) {

        res = hexChars[n & 15] + res;
        n >>= 4;
    }

    return res;
}

// Dump memory contents to console and trace file
void memoryDump(ofstream &traceFile, int size) {

    cout << "\n\t\tMemory Dump\t\t\n";

    traceFile << "\n\t\tMemory Dump\t\t\n";
    for (int i = 0; i < size; i++) {

        if (i % 4) {
            cout << toHex(mem[i]) << " "; traceFile << toHex(mem[i]) << " ";

        } else {
            cout << "\n" << toHex(i) << "\t" << toHex(mem[i]) << " ";

            traceFile << "\n" << toHex(i) << "\t" << toHex(mem[i]) << " ";
        }
    }
    cout << endl;
}

// Trace program execution
void runTrace(ofstream &traceFile, int pcStart) {

    cout << "\n\t\t---Tracing Instructions---\t\t\n\n";

    traceFile << "\n\t\t---Tracing Instructions---\t\t\n\n";

    set<int> pcOffsetOpcodes{13, 15, 16, 17};

    bool halt = false;

    int instrCount = 0;

    while (true) {
        int instr = mem[PC];

        int maxHex = 0xffffffff, maxOp = 0xff;


        int opcodeByte = instr & 0xff;

        string hexStr = toHex(opcodeByte).substr(6, 2);

        int opCode = stoi(hexStr, nullptr, 16);

        if (hexStr[0] >= '8') opCode = -(0xff - opCode + 1);

        int operand = instr & 0xffffff00;
        if (operand & (1 << 31)) operand = -(maxHex - operand + 1);
        operand >>= 8;

        if (opcodeMap.count(opCode)) {
            cout << "PC: " << toHex(PC) << "\tSP: " << toHex(SP)

                 << "\tA: " << toHex(regA) << "\tB: " << toHex(regB)

                 << "\t" << opcodeMap[opCode] << " " << operand << "\n\n";

            traceFile << "PC: " << toHex(PC) << "\tSP: " << toHex(SP)

                      << "\tA: " << toHex(regA) << "\tB: " << toHex(regB)

                      << "\t" << opcodeMap[opCode] << " " << operand << "\n\n";
        } else halt = true;

        // Execute opcode
        switch(opCode) {

            case 0: regB = regA; regA = operand; break;

            case 1: regA += operand; break;

            case 2: regB = regA; regA = mem[SP + operand]; break;

            case 3: mem[SP + operand] = regA; regA = regB; break;

            case 4: regA = mem[regA + operand]; break;

            case 5: mem[regA + operand] = regB; break;

            case 6: regA += regB; break;
            
            case 7: regA = regB - regA; break;

            case 8: regA = regB << regA; break;

            case 9: regA = regB >> regA; break;

            case 10: SP += operand; break;

            case 11: SP = regA; regA = regB; break;

            case 12: regB = regA; regA = SP; break;

            case 13: regB = regA; regA = PC; PC += operand; break;

            case 14: if (PC == regA && regA == regB) halt = true; PC = regA; regA = regB; break;

            case 15: if (regA == 0) PC += operand; break;

            case 16: if (regA < 0) PC += operand; break;

            case 17: PC += operand; break;

            case 18: halt = true; break;
        }

        if (SP >= 10000) { cout << "SP exceeded memory at PC: " << PC << endl; halt = true; }

        if (pcOffsetOpcodes.count(opCode) && operand == -1) { cout << "Infinite loop detected\n"; halt = true; }

        if (halt) break;

        PC++; instrCount++;
    }

    cout << instrCount << " instructions executed!\n";
}

//  MAIN FUNCTION 
int main(int argc, char *argv[]) {
    if (argc != 3) {

        cout << "Usage: ./emu [option] file.o\n";

        cout << "[option] -t : trace execution\n\t-b : memory dump before\n\t-a : memory dump after\n\t-I : display ISA\n";

        return 0;
    }

    initOpcodeMap();

    string mode = argv[1], inputFile = argv[2];

    int dotPos = inputFile.find('.');

    if (dotPos == -1 || inputFile.substr(dotPos) != ".o") {

        cout << "Invalid object file\n"; return 0;
    }

    string traceFileName = inputFile.substr(0, dotPos) + ".trace";

    ifstream inFile(inputFile); ofstream outFile(traceFileName);

    if (!inFile || !outFile) { cout << "Error opening files\n"; return 0; }

    
    string line;

    int maxOperand = stoi("ffffff", nullptr, 16), maxOpcode = stoi("ff", nullptr, 16);

     int lineNo = 0;

    while (getline(inFile, line)) {

    if (line.empty()) continue;

    string hexStr = line.substr(0, 8);

    unsigned long val = stoul(hexStr, nullptr, 16);

    int instr = static_cast<int>(val);

    int opc = stoi(hexStr.substr(6, 2), nullptr, 16);

    if (hexStr[6] >= '8') opc = -(maxOpcode - opc + 1);

    int opr = stoi(hexStr.substr(0, 6), nullptr, 16);

    if (hexStr[0] >= '8') opr = -(maxOperand - opr + 1);

    mem[lineNo] = (opc < 0 ? opr : instr);

    lineNo++;
    }

if (lineNo == 0) {

    cout << "File is empty\n";

    return -1;
}

    if (mode[1] == 'I') {
        cout << "Opcode  Mnemonic  Operand\n"
             << "        data      value\n"
             << "0       ldc       value\n"
             << "1       adc       value\n"
             << "2       ldl       value\n"
             << "3       stl       value\n"
             << "4       ldnl      value\n"
             << "5       stnl      value\n"
             << "6       add\n"
             << "7       sub\n"
             << "8       shl\n"
             << "9       shr\n"
             << "10      adj       value\n"
             << "11      a2sp\n"
             << "12      sp2a\n"
             << "13      call      offset\n"
             << "14      return\n"
             << "15      brz       offset\n"
             << "16      brlz      offset\n"
             << "17      br        offset\n"
             << "18      HALT\n"
             << "        SET       value\n";
    }

    if (mode[1] == 'b') memoryDump(outFile, lineNo);

    if (mode[1] == 't') runTrace(outFile, PC);

    if (mode[1] == 'a') { runTrace(outFile, PC); memoryDump(outFile, lineNo); }

    return 0;
}