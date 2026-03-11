#include <bits/stdc++.h>
using namespace std;

map<string, pair<int, int>> opcode;
void mapping_function_opcode(map<string, pair<int, int>> &opcode);

//  for removing leading and trailing whitespaces
void strip(string &line);

// n-base numbers conversion
int n_baseConversion(string &n, int start, int initial_base);

//  convert integers to hex and store in string
string itohex(int n);
string finalinstruction(const pair<int, int> p);

map<string, int> SETlabels;

void fetchh(string &oprnd, int &arg, map<string, int> &labels, int op, int pc);
// Function to fetch all instructions and labels from input file
void extractInstrLabels(map<int, pair<string, string>> &instructions, fstream &fptr, map<string, int> &labels, map<int, string> &errors, int &pc, int &line, map<int, int> &pctoline)
{
    string ln;
    // Read and store file line by line and store in ln
    while (getline(fptr, ln))
    {
        line++;
        // removing commented part of code
        ln = regex_replace(ln, regex(";.*$"), string(""));
        // removing leading and trailing whitespaces
        strip(ln);
        // if empty string then continue to next line
        if (ln == "")
            continue;
        pctoline[pc] = line;
        // checking for label
        bool label_found = false;
        // i is position of ':' in ln
        size_t i = 0;
        string templabel;
        for (; i < ln.length(); i++)
        {
            if (ln[i] == ':')
            {
                label_found = true;
                templabel = ln.substr(0, i);
                // check for correctnes of label name
                if (!isalpha(templabel[0]) || find_if(templabel.begin(), templabel.end(), [](char c)
                                                      { return !(isalnum(c)); }) != templabel.end())
                {
                    errors[line] += "Error: Label naming rules violated\n";
                    label_found = false;
                }
                // check for empty label
                if (templabel == "")
                {
                    errors[line] += "Error: Empty label detected\n";
                    label_found = false;
                }
                // check for duplicate label
                if (labels.count(templabel))
                {
                    errors[line] += "Error: Duplicate label found\n";
                    label_found = false;
                }

                if (label_found)
                    labels[templabel] = pc;
                break;
            }
        }
        if (i == ln.length())
            i = -1;

        // pushing only the instruction part of a line
        string temp(ln.begin() + i + 1, ln.end());
        strip(temp);
        if (temp == "")
            continue;
        // splitting temp into mnemonic and operand
        int diff_pos = temp.length();
        for (size_t i = 0; i < temp.length(); i++)
        {
            if (temp[i] == ' ')
            {
                diff_pos = i;
                break;
            }
        }
        string mnemonic(temp.begin(), temp.begin() + diff_pos), oprnd(temp.begin() + diff_pos, temp.end());
        strip(mnemonic);
        strip(oprnd);

        // check for errors in mnemonic and operand
        bool instrOK = true;
        if (opcode.find(mnemonic) == opcode.end())
        {
            errors[line] += "Error: Mnemonic is wrong\n";
            instrOK = false;
        }
        if (instrOK && opcode[mnemonic].second == 1 && oprnd == "")
        {
            errors[line] += "Error: Missing operand\n";
            instrOK = false;
        }
        if (instrOK && opcode[mnemonic].second == 0 && oprnd.length())
        {
            errors[line] += "Error: Unexpected operand present\n";
            instrOK = false;
        }
        if (mnemonic == "SET")
        {
            if (i == -1)
            {
                errors[line] += "No label present but SET is used\n";
            }
            else
            {
                int arg;
                fetchh(oprnd, arg, labels, -1, pc);
                SETlabels.insert({templabel, arg});
            }
        }
        else
        {
            instructions[pc] = {mnemonic, oprnd};
            pc++;
        }
    }
}

void fetchh(string &oprnd, int &arg, map<string, int> &labels, int op, int pc)
{
    // if oprnd is a label
    if (((oprnd[0] <= 'z' && oprnd[0] >= 'a') || (oprnd[0] <= 'Z' && oprnd[0] >= 'A')))
    {
        // if label's value is ovrwritten by SET instruction then we need to fetch its value from here
        if (SETlabels.count(oprnd))
            arg = SETlabels[oprnd];
        else
            arg = labels[oprnd];
    }
    // oprnd is a value
    else
    {
        // hex
        if (oprnd.substr(0, 2) == "0x")
        {
            arg = n_baseConversion(oprnd, 2, 16);
        }
        // oct
        else if (oprnd[0] == '0')
        {
            arg = n_baseConversion(oprnd, 1, 8);
        }
        // dec
        else
        {
            arg = n_baseConversion(oprnd, 0, 10);
        }
    }
    // if instruction is pc offset then arg needs to be changed
    if (op == 13 || op == 15 || op == 16 || op == 17)
        arg = arg - (pc + 1);
    return;
}

void check_operands(map<int, pair<string, string>> &instructions, map<string, int> &labels, map<int, string> &errors, map<int, int> pctoline)
{
    for (auto &&i : instructions)
    {
        bool labelpresent = false, instrOK = false;
        string oprnd = i.second.second;
        bool commapresent = any_of(oprnd.begin(), oprnd.end(), [](char c)
                                   { return c == ','; });
        if (commapresent)
        {
            errors[pctoline[i.first]] += "Error: Extra operand present\n";
        }

        if (((oprnd[0] <= 'z' && oprnd[0] >= 'a') || (oprnd[0] <= 'Z' && oprnd[0] >= 'A')))
        {
            if (!labels.count(oprnd))
                errors[pctoline[i.first]] += "Error: No such label present!\n";
            labelpresent = true;
        }
        if (!labelpresent)
        {
            // hex
            if (oprnd.substr(0, 2) == "0x")
            {
                for (int j = 2; j < oprnd.length(); j++)
                {
                    char d = oprnd[j];
                    bool check = (d <= 'f' && d >= 'a') || (d <= 'F' && d >= 'A') || (d >= '0' && d <= '9');
                    if (!check)
                    {
                        instrOK = false;
                        errors[pctoline[i.first]] += "Error: Operand not correct\n";
                        break;
                    }
                }
            }
            // oct
            else if (oprnd[0] == '0')
            {
                for (int j = 1; j < oprnd.length(); j++)
                {
                    char d = oprnd[j];
                    bool check = (d <= '7' && d >= '0');
                    if (!check)
                    {
                        instrOK = false;
                        errors[pctoline[i.first]] += "Error: Operand not correct\n";
                        break;
                    }
                }
            }
            // dec
            else
            {
                for (auto &&j : oprnd)
                {
                    if (j == '+' || j == '-')
                    {
                        continue;
                    }
                    bool check = (j >= '0' && j <= '9');
                    if (!check)
                    {
                        instrOK = false;
                        errors[pctoline[i.first]] += "Error: Operand not correct\n";
                        break;
                    }
                }
            }
        }
    }
}

void second_pass(map<int, pair<string, string>> &instructions, map<string, int> &labels, map<int, int> &pctoline, vector<pair<int, int>> &finalcodes)
{
    // here i = PC
    for (size_t i = 0; i < instructions.size(); i++)
    {
        int ln = pctoline[i];
        string mnemo = instructions[i].first, oprnd = instructions[i].second;
        int opc = opcode[mnemo].first;
        int arg;
        if (opcode[mnemo].second)
            fetchh(oprnd, arg, labels, opc, i);
        else
            arg = 0;
        if (mnemo == "data")
        {
            opc = arg & 0b11111111;
            arg >>= 8;
        }
        finalcodes.push_back({arg, opc});
    }
}

int main(int argc, char *argv[])
{
    // argv[1] => assembler code file (input file)
    // argv[2] => object file
    if (argc != 2)
    {
        cout << "Correct way to use:\n./asm anyname.asm\nwhere anyname.asm is the source file\n";
        return 0;
    }
    string input_file = argv[1];

    // Step 1: Open the input file
    fstream fptr(input_file);

    // Step 2: Extract the instructions from input file

    // instructions will store mnemonic and operand
    map<int, pair<string, string>> instructions;

    // to store {operand, opcode}
    vector<pair<int, int>> finalcodes;

    // to store labels on each line
    map<string, int> labels;

    // to store errors and line no.
    map<int, string> errors;

    // mapping mnemonics to their opcodes
    mapping_function_opcode(opcode);

    // to map line number to pc
    map<int, int> pctoline;

    int line = 0, pc = 0;

    // routine function to extract instructions and labels
    extractInstrLabels(instructions, fptr, labels, errors, pc, line, pctoline);

   

    // check for correctness of operands
    check_operands(instructions, labels, errors, pctoline);

    // Step 3: Convert the instructions into binary code
    // If no errors found then run second pass
    if (!errors.size())
        second_pass(instructions, labels, pctoline, finalcodes);

    // generating the output files
    string basename = "";
    for (size_t i = 0; i < input_file.length(); i++)
    {
        if (input_file[i] == '.')
        {
            break;
        }
        else
            basename.push_back(input_file[i]);
    }

    string logfilename = basename + ".log";

    ofstream logfptr(logfilename);
    if (!logfptr)
    {
        cout << "Error in opening log file!\n";
        return 0;
    }

    if (errors.size())
    {
        logfptr << "Assembly failed due to errors:\n";
        for (auto &&l : errors)
        {
            logfptr << "line " << l.first << " : " << l.second << "\n";
        }
        logfptr.close();
    }
    else
    {
        logfptr << "Compiled successfully";
        logfptr.close();

        string objectfilename = basename + ".o";
        string listingfilename = basename + ".lst";

        ofstream objfptr(objectfilename, ios::out | ios::binary);
        ofstream listfptr(listingfilename);

        if (!objfptr)
        {
            cout << "Error in opening object code file!\n";
            return 0;
        }
        if (!listfptr)
        {
            cout << "Error in opening listing file!\n";
            return 0;
        }

        // printing to object file
        for (auto &&i : finalcodes)
        {
            objfptr << finalinstruction(i);
        }

        // writing to listing file
        for (size_t i = 0; i < finalcodes.size(); i++)
        {
            // writing pc in one column
            string pc_8char = itohex(i);
            listfptr << pc_8char << " ";

            // printing instruction and labels
            for (auto &&label : labels)
            {
                if (label.second == i)
                {
                    listfptr << "         ";
                    listfptr << label.first << ":\n";
                    listfptr << pc_8char << " ";
                    break;
                }
            }

            // writing instruction as hexcode
            listfptr << finalinstruction(finalcodes[i]) << " ";

            // writing mnemonics to file
            listfptr << instructions[i].first << " " << instructions[i].second << "\n";
        }

        objfptr.close();

        listfptr.close();
    }

    return 0;
}

// Mapping  of instruction to opcode
void mapping_function_opcode(map<string, pair<int, int>> &opcode)
{
    // pair.first represents opcode and second represents number of operands
    opcode["ldc"] = {0, 1};
    opcode["adc"] = {1, 1};
    opcode["ldl"] = {2, 1};
    opcode["stl"] = {3, 1};
    opcode["ldnl"] = {4, 1};
    opcode["stnl"] = {5, 1};
    opcode["add"] = {6, 0};
    opcode["sub"] = {7, 0};
    opcode["shl"] = {8, 0};
    opcode["shr"] = {9, 0};
    opcode["adj"] = {10, 1};
    opcode["a2sp"] = {11, 0};
    opcode["sp2a"] = {12, 0};
    opcode["call"] = {13, 1};
    opcode["return"] = {14, 0};
    opcode["brz"] = {15, 1};
    opcode["brlz"] = {16, 1};
    opcode["br"] = {17, 1};
    opcode["HALT"] = {18, 0};
    opcode["data"] = {-1, 1};
    opcode["SET"] = {-2, 1};
}
//  remove whitespaces from left and right side of a string
void strip(string &line)
{
    line = regex_replace(line, regex("^\\s+"), string(""));
    line = regex_replace(line, regex("\\s+$"), string(""));
    return;
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

// convert integers to hex and store in string
string itohex(int n)
{
    // to store result on
    ostringstream oss;
    int len = 8;
    oss << hex << setw(len) << setfill('0') << n;
    string ans = oss.str();
    if (len < ans.length())
    {
        ans = ans.substr(ans.length() - len, len);
    }
    return ans;
}
string finalinstruction(const pair<int, int> p)
{
    int num = (p.first << 8) | p.second;
    return itohex(num);
}