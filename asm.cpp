#include <bits/stdc++.h>
using namespace std;

// Map of mnemonics to opcode and number of operands
map<string, pair<int, int>> opcodeMap;

// Map for labels defined using SET instruction
map<string, int> setLabels;

// Function declarations
void setupOpcodeMap(map<string, pair<int, int>> &opcodeMap);
void trim(string &s);
int convertToBase(string &num, int start, int base);
string intToHex(int n);
string encodeInstruction(const pair<int, int> &p);
void fetchOperand(string &operand, int &arg, map<string, int> &labels, int op, int pc);
void extractInstructionsAndLabels(map<int, pair<string, string>> &instr, fstream &file, map<string, int> &labels, map<int, string> &errors, int &pc, int &lineNo, map<int, int> &pcToLine);

// ------------------- MAIN FUNCTION -------------------
int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: ./asm filename.asm\n";
        return 0;
    }

    string inputFile = argv[1];
    fstream inFile(inputFile);

    map<int, pair<string, string>> instructions; // PC -> {mnemonic, operand}
    vector<pair<int, int>> machineCode;          // final binary code
    map<string, int> labels;                     // label -> PC
    map<int, string> errors;                     // line -> error message
    map<int, int> pcToLine;                      // PC -> line number

    int line = 0, pc = 0;

    // Initialize opcode mapping
    setupOpcodeMap(opcodeMap);

    // Step 1: Extract instructions and labels
    extractInstructionsAndLabels(instructions, inFile, labels, errors, pc, line, pcToLine);

    // Step 2: Validate operands
    for (auto &entry : instructions) {
        string operand = entry.second.second;
        int ln = pcToLine[entry.first];

        // Check for comma in operand
        if (any_of(operand.begin(), operand.end(), [](char c){ return c == ','; }))
            errors[ln] += "Error: Extra operand detected\n";

        // If operand is a label
        if (isalpha(operand[0])) {
            if (!labels.count(operand))
                errors[ln] += "Error: Label not found\n";
        } else { 
            // Validate number format
            if (operand.substr(0, 2) == "0x") {
                for (size_t j = 2; j < operand.size(); j++) {
                    char c = operand[j];
                    if (!isalnum(c) || (c >= 'g' && c <= 'z') || (c >= 'G' && c <= 'Z')) {
                        errors[ln] += "Error: Invalid hex operand\n";
                        break;
                    }
                }
            } else if (operand[0] == '0') {
                for (size_t j = 1; j < operand.size(); j++)
                    if (operand[j] < '0' || operand[j] > '7') {
                        errors[ln] += "Error: Invalid octal operand\n";
                        break;
                    }
            } else {
                for (char c : operand)
                    if (!isdigit(c) && c != '+' && c != '-') {
                        errors[ln] += "Error: Invalid decimal operand\n";
                        break;
                    }
            }
        }
    }

    // Step 3: Convert instructions to machine code if no errors
    if (errors.empty()) {
        for (size_t i = 0; i < instructions.size(); i++) {
            string mnemo = instructions[i].first;
            string oprnd = instructions[i].second;
            int opc = opcodeMap[mnemo].first;
            int arg = 0;

            if (opcodeMap[mnemo].second)
                fetchOperand(oprnd, arg, labels, opc, i);

            if (mnemo == "data") {
                opc = arg & 0xFF;
                arg >>= 8;
            }

            machineCode.push_back({arg, opc});
        }
    }

    // Step 4: Generate log, object, and listing files
    string baseName = inputFile.substr(0, inputFile.find('.'));
    ofstream logFile(baseName + ".log");

    if (!errors.empty()) {
        logFile << "Assembly failed due to errors:\n";
        for (auto &e : errors)
            logFile << "line " << e.first << " : " << e.second;
        logFile.close();
        return 0;
    }

    logFile << "Compiled successfully\n";
    logFile.close();

    ofstream objFile(baseName + ".o", ios::out | ios::binary);
    ofstream listFile(baseName + ".lst");

    for (auto &code : machineCode)
        objFile << encodeInstruction(code)<<endl;

    for (size_t i = 0; i < machineCode.size(); i++) {
        string pcStr = intToHex(i);
        listFile << pcStr << " ";

        for (auto &l : labels) {
            if (l.second == i) {
                listFile << "         " << l.first << ":\n" << pcStr << " ";
                break;
            }
        }

        listFile << encodeInstruction(machineCode[i]) << " "
                 << instructions[i].first << " " << instructions[i].second << "\n";
    }

    objFile.close();
    listFile.close();

    return 0;
}

// ------------------- HELPER FUNCTIONS -------------------

// Setup opcode map
void setupOpcodeMap(map<string, pair<int, int>> &opcodeMap) {
    opcodeMap["ldc"] = {0, 1}; opcodeMap["adc"] = {1, 1};
    opcodeMap["ldl"] = {2, 1}; opcodeMap["stl"] = {3, 1};
    opcodeMap["ldnl"] = {4, 1}; opcodeMap["stnl"] = {5, 1};
    opcodeMap["add"] = {6, 0}; opcodeMap["sub"] = {7, 0};
    opcodeMap["shl"] = {8, 0}; opcodeMap["shr"] = {9, 0};
    opcodeMap["adj"] = {10, 1}; opcodeMap["a2sp"] = {11, 0};
    opcodeMap["sp2a"] = {12, 0}; opcodeMap["call"] = {13, 1};
    opcodeMap["return"] = {14, 0}; opcodeMap["brz"] = {15, 1};
    opcodeMap["brlz"] = {16, 1}; opcodeMap["br"] = {17, 1};
    opcodeMap["HALT"] = {18, 0}; opcodeMap["data"] = {-1, 1};
    opcodeMap["SET"] = {-2, 1};
}

// Trim spaces from both ends
void trim(string &s) {
    s = regex_replace(s, regex("^\\s+|\\s+$"), "");
}

// Convert string number to integer of given base
int convertToBase(string &num, int start, int base) {
    if (base == 10) {
        int sign = 1;
        if (num[0] == '+' || num[0] == '-') start++;
        if (num[0] == '-') sign = -1;
        return sign * stoi(num.substr(start), 0, base);
    }
    return stoi(num, 0, base);
}

// Convert integer to 8-char hex string
string intToHex(int n) {
    ostringstream oss;
    oss << hex << setw(8) << setfill('0') << n;
    string res = oss.str();
    return res.size() > 8 ? res.substr(res.size() - 8) : res;
}

// Encode instruction into hex string
string encodeInstruction(const pair<int, int> &p) {
    int code = (p.first << 8) | p.second;
    return intToHex(code);
}

// Fetch operand value
void fetchOperand(string &operand, int &arg, map<string, int> &labels, int op, int pc) {
    if (isalpha(operand[0])) {
        if (setLabels.count(operand))
            arg = setLabels[operand];
        else
            arg = labels[operand];
    } else if (operand.substr(0, 2) == "0x") {
        arg = convertToBase(operand, 2, 16);
    } else if (operand[0] == '0') {
        arg = convertToBase(operand, 1, 8);
    } else {
        arg = convertToBase(operand, 0, 10);
    }

    if (op == 13 || op == 15 || op == 16 || op == 17)
        arg -= (pc + 1);
}

// Extract instructions and labels from file
void extractInstructionsAndLabels(map<int, pair<string, string>> &instr, fstream &file, map<string, int> &labels, map<int, string> &errors, int &pc, int &lineNo, map<int, int> &pcToLine) {
    string lineStr;
    while (getline(file, lineStr)) {
        lineNo++;
        lineStr = regex_replace(lineStr, regex(";.*$"), "");
        trim(lineStr);
        if (lineStr.empty()) continue;

        pcToLine[pc] = lineNo;
        size_t colonPos = lineStr.find(':');
        string lbl;
        if (colonPos != string::npos) {
            lbl = lineStr.substr(0, colonPos);
            trim(lbl);

            if (lbl.empty()) errors[lineNo] += "Error: Empty label\n";
            else if (!isalpha(lbl[0]) || find_if(lbl.begin(), lbl.end(), [](char c){ return !isalnum(c); }) != lbl.end())
                errors[lineNo] += "Error: Invalid label\n";
            else if (labels.count(lbl)) errors[lineNo] += "Error: Duplicate label\n";
            else labels[lbl] = pc;

            lineStr = lineStr.substr(colonPos + 1);
            trim(lineStr);
        }

        if (lineStr.empty()) continue;

        size_t spacePos = lineStr.find(' ');
        string mnemonic = lineStr.substr(0, spacePos);
        string operand = spacePos == string::npos ? "" : lineStr.substr(spacePos + 1);
        trim(mnemonic);
        trim(operand);

        if (opcodeMap.find(mnemonic) == opcodeMap.end())
            errors[lineNo] += "Error: Unknown mnemonic\n";
        else if (opcodeMap[mnemonic].second == 1 && operand.empty())
            errors[lineNo] += "Error: Operand missing\n";
        else if (opcodeMap[mnemonic].second == 0 && !operand.empty())
            errors[lineNo] += "Error: Unexpected operand\n";

        if (mnemonic == "SET") {
            if (lbl.empty()) errors[lineNo] += "Error: SET without label\n";
            else {
                int val;
                fetchOperand(operand, val, labels, -1, pc);
                setLabels[lbl] = val;
            }
        } else {
            instr[pc++] = {mnemonic, operand};
        }
    }
}