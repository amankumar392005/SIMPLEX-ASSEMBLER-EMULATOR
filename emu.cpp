#include <bits/stdc++.h>
using namespace std;

map<int, string> opcodeToMnemonic;
int memory[10000], A = 0, B = 0, PC = 0, SP = 9999;

void populateopcodemap()
{
    opcodeToMnemonic[0] = "ldc";
    opcodeToMnemonic[1] = "adc";
    opcodeToMnemonic[2] = "ldl";
    opcodeToMnemonic[3] = "stl";
    opcodeToMnemonic[4] = "ldnl";
    opcodeToMnemonic[5] = "stnl";
    opcodeToMnemonic[6] = "add";
    opcodeToMnemonic[7] = "sub";
    opcodeToMnemonic[8] = "shl";
    opcodeToMnemonic[9] = "shr";
    opcodeToMnemonic[10] = "adj";
    opcodeToMnemonic[11] = "a2sp";
    opcodeToMnemonic[12] = "sp2a";
    opcodeToMnemonic[13] = "call";
    opcodeToMnemonic[14] = "return";
    opcodeToMnemonic[15] = "brz";
    opcodeToMnemonic[16] = "brlz";
    opcodeToMnemonic[17] = "br";
    opcodeToMnemonic[18] = "HALT";
    opcodeToMnemonic[-1] = "data";
    opcodeToMnemonic[-2] = "SET";
}

// n-base numbers conversion
int n_baseConversion(string &n, int start, int initial_base)
{
    if (initial_base == 10)
    {
        start = 0;
        int sign = 1;
        if (n[0] == '+' || n[0] == '-')
            start++;
        if (n[0] == '-')
            sign = -1;
        string temp(n.begin() + start, n.end());
        int ans = sign * stoi(temp, 0, initial_base);
        return ans;
    }
    else
    {
        return stoi(n, 0, initial_base);
    }
}

//  convert integers to hex and store in string
string itoHex(int n)
{
    ostringstream ss;
    int len = 8;
    ss << hex << setw(len) << setfill('0') << n;
    string res = ss.str();
    if (len < res.length())
    {
        res = res.substr(res.length() - len, len);
    }
    return res;
}

void Memory_dump(ofstream &traceFile, int PC)
{
    cout << "\n\t\t   Dumping from memory   \t\t\n";
    traceFile << "\n\t\t   Dumping from memory   \t\t\n";
    for (int i = 0; i < PC; i++)
    {
        if (i % 4)
        {
            cout << itoHex(memory[i]) << " ";
            traceFile << itoHex(memory[i]) << " ";
        }
        else
        {
            cout << "\n"
                 << itoHex(i) << "\t" << itoHex(memory[i]) << " ";
            traceFile << "\n"
                      << itoHex(i) << "\t" << itoHex(memory[i]) << " ";
        }
    }
    cout << endl;
}

// Function to trace individual instructions
void trace(ofstream &traceptr, int PC)
{
    cout << "\n\t\t---Tracing instructions---\t\t\n\n";
    traceptr << "\n\t\t---Tracing instructions---\t\t\n\n";

    set<int> PCoffset{15, 13, 16, 17};
    // Loop till halt is true
    bool halt = false;
    int line = 0;

    while (1)
    {
        int instruction = memory[PC];

        // for calculation of negative numbers in hexadecimal
        int opupper = 0xff;
        unsigned long hex_max = stoul("ffffffff", nullptr, 16);
        int32_t all_max = static_cast<int32_t>(hex_max);

        int32_t tempo = instruction & 0xff;
        string s_hexcode = itoHex(tempo).substr(6, 2);

        int32_t opCode = stoi(s_hexcode, nullptr, 16);
        if (s_hexcode[0] >= '8')

            opCode = -(opupper - opCode + 1);

        int32_t oparnd = instruction & 0xffffff00;
        if (oparnd & (1 << 31))
        {
            oparnd = -(all_max - oparnd + 1);
        }
        oparnd = oparnd >> 8;
        if (opcodeToMnemonic.find(opCode) != opcodeToMnemonic.end())
        {
            cout << "PC: " << itoHex(PC) << "\tSP: " << itoHex(SP) << "\tA: "
                 << itoHex(A) << "\tB: " << itoHex(B) << "\t" << opcodeToMnemonic[opCode]
                 << " " << oparnd << endl
                 << endl;

            traceptr << "PC: " << itoHex(PC) << "\tSP: " << itoHex(SP) << "\tA: "
                     << itoHex(A) << "\tB: " << itoHex(B) << "\t" << opcodeToMnemonic[opCode]
                     << " " << oparnd << endl
                     << endl;
        }
        else
        {
            halt = true;
        }

        if (opCode == 0)
        {
            B = A;
            A = oparnd;
        }
        else if (opCode == 1)
        {
            A += oparnd;
        }
        else if (opCode == 2)
        {
            B = A;
            A = memory[SP + oparnd];
        }
        else if (opCode == 3)
        {
            memory[SP + oparnd] = A;
            A = B;
        }
        else if (opCode == 4)
        {
            A = memory[A + oparnd];
        }
        else if (opCode == 5)
        {
            memory[A + oparnd] = B;
        }
        else if (opCode == 6)
        {
            A += B;
        }
        else if (opCode == 7)
        {
            A = B - A;
        }
        else if (opCode == 8)
        {
            A = B << A;
        }
        else if (opCode == 9)
        {
            A = B >> A;
        }
        else if (opCode == 10)
        {
            SP += oparnd;
        }
        else if (opCode == 11)
        {
            SP = A;
            A = B;
        }
        else if (opCode == 12)
        {
            B = A;
            A = SP;
        }
        else if (opCode == 13)
        {
            B = A;
            A = PC;
            PC += oparnd;
        }
        else if (opCode == 14)
        {
            if (PC == A && A == B)
                halt = true;
            PC = A;
            A = B;
        }
        else if (opCode == 15)
        {
            if (A == 0)
                PC += oparnd;
        }
        else if (opCode == 16)
        {
            if (A < 0)
                PC += oparnd;
        }
        else if (opCode == 17)
        {
            PC += oparnd;
        }
        else if (opCode == 18)
        {
            halt = true;
        }

        if (SP >= 10000)
        {
            cout << "SP exceeding the memory at PC: " << PC << endl;
            halt = true;
        }
        if (PCoffset.count(opCode) && oparnd == -1)
        {
            cout << "Infinite loop detected" << endl;
            halt = true;
        }
        if (halt)
            break;
        PC++;
        line++;
    }
    cout << line << " number of instructions executed!" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Instructions for use:\n./emu [option] file.o\n";
        cout << "[option] can be one of the following:\n";
        cout << "\t-t : print trace of the code executed." << endl;
        cout << "\t-b : Print memory dump before program execution." << endl;
        cout << "\t-a : Show memory dump after program execution." << endl;
        cout << "\t-ISA\tdisplay ISA" << endl;
        return 0;
    }

    populateopcodemap();

    string mode = argv[1], input_file = argv[2];
    int dotpos = -1;
    for (size_t i = 0; i < input_file.length(); i++)
    {
        if (input_file[i] == '.')
        {
            dotpos = i;
            break;
        }
    }
    if (dotpos == -1)
    {
        cout << "Input filename is wrong\n";
        return 0;
    }
    if (input_file.substr(dotpos, 2) != ".o")
    {
        cout << "Not an object file\n";
        return 0;
    }

    string traceFile = input_file.substr(0, dotpos) + ".trace";

    ifstream inptr(input_file);
    ofstream outptr(traceFile);

    if (!inptr)
    {
        cout << "Error in opening file " << input_file << "\n";
        return 0;
    }
    if (!outptr)
    {
        cout << "Error in opening file " << traceFile << "\n";
        return 0;
    }
    string ln;
    getline(inptr, ln);
    // File is empty
    if (!ln.length())
    {
        cout << "File is empty\n";
        exit(-1);
    }

    int maxi_opr = stoi("ffffff", nullptr, 16), line_no = 0, maxi_op = stoi("ff", nullptr, 16), i = 0;

    while (i < ln.length())
    {
        string hexcode = ln.substr(i, 8);
        unsigned long valinhex = stoul(hexcode, nullptr, 16);
        int instr = static_cast<int>(valinhex);

        string xyz = hexcode.substr(6, 2);
        int32_t opc = stoi(xyz, nullptr, 16);
        if (xyz[0] >= '8')
        {
            opc = -(maxi_op - opc + 1);
        }

        xyz = hexcode.substr(0, 6);
        int32_t opr = stoi(xyz, nullptr, 16);
        if (xyz[0] >= '8')
        {
            opr = -(maxi_opr - opr + 1);
        }

        if (opc < 0)
        {
            memory[line_no] = opr;
        }
        else
        {
            memory[line_no] = instr;
        }
        i += 8;
        line_no++;
    }

    if (mode[1] == 'I')
    {
        cout << "Opcode  Mnemonic  Operand\n";
        cout << "        data      value\n";
        cout << "0       ldc       value\n";
        cout << "1       adc       value\n";
        cout << "2       ldl       value\n";
        cout << "3       stl       value\n";
        cout << "4       ldnl      value\n";
        cout << "5       stnl      value\n";
        cout << "6       add\n";
        cout << "7       sub\n";
        cout << "8       shl\n";
        cout << "9       shr\n";
        cout << "10      adj       value\n";
        cout << "11      a2sp\n";
        cout << "12      sp2a\n";
        cout << "13      call      offset\n";
        cout << "14      return\n";
        cout << "15      brz       offset\n";
        cout << "16      brlz      offset\n";
        cout << "17      br        offset\n";
        cout << "18      HALT\n";
        cout << "        SET       value\n";
    }
    if (mode[1] == 'b')
    {
        Memory_dump(outptr, line_no);
    }
    if (mode[1] == 't')
    {
        trace(outptr, PC);
    }
    if (mode[1] == 'a')
    {
        trace(outptr, PC);
        Memory_dump(outptr, line_no);
    }

    return 0;
}