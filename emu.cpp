#include <bits/stdc++.h>
using namespace std;

map<string,pair<int,int>> opcode;
map<string,int> labels, SETlabels;
map<int,string> errors;

string trim(string s){
    int l=0,r=s.size()-1;
    while(l<=r && isspace(s[l])) l++;
    while(r>=l && isspace(s[r])) r--;
    if(l>r) return "";
    return s.substr(l,r-l+1);
}

string toHex(int x){
    stringstream ss;
    ss<<hex<<setw(8)<<setfill('0')<<x;
    string s=ss.str();
    if(s.size()>8) s=s.substr(s.size()-8);
    return s;
}

void initOpcode(){

    opcode["ldc"]={0,1};
    opcode["adc"]={1,1};
    opcode["ldl"]={2,1};
    opcode["stl"]={3,1};
    opcode["ldnl"]={4,1};
    opcode["stnl"]={5,1};
    opcode["add"]={6,0};
    opcode["sub"]={7,0};
    opcode["shl"]={8,0};
    opcode["shr"]={9,0};
    opcode["adj"]={10,1};
    opcode["a2sp"]={11,0};
    opcode["sp2a"]={12,0};
    opcode["call"]={13,1};
    opcode["return"]={14,0};
    opcode["brz"]={15,1};
    opcode["brlz"]={16,1};
    opcode["br"]={17,1};
    opcode["HALT"]={18,0};
    opcode["data"]={-1,1};
    opcode["SET"]={-2,1};
}

bool validLabel(string s){

    if(s=="" || !isalpha(s[0])) return false;

    for(char c:s)
        if(!isalnum(c)) return false;

    return true;
}

bool validOperand(string s){

    if(s=="") return false;

    if(isalpha(s[0])) return true;

    if(s.size()>2 && s.substr(0,2)=="0x"){
        for(int i=2;i<s.size();i++)
            if(!isxdigit(s[i])) return false;
        return true;
    }

    if(s[0]=='0' && s.size()>1){
        for(int i=1;i<s.size();i++)
            if(s[i]<'0'||s[i]>'7') return false;
        return true;
    }

    for(char c:s)
        if(!isdigit(c) && c!='+' && c!='-')
            return false;

    return true;
}

int getValue(string s,int pc,int op){

    int val;

    if(isalpha(s[0])){
        if(SETlabels.count(s)) val=SETlabels[s];
        else val=labels[s];
    }
    else{
        if(s.substr(0,2)=="0x") val=stoi(s,nullptr,16);
        else if(s[0]=='0' && s.size()>1) val=stoi(s,nullptr,8);
        else val=stoi(s);
    }

    if(op==13||op==15||op==16||op==17)
        val -= (pc+1);

    return val;
}

int main(int argc,char* argv[]){

    if(argc!=2){
        cout<<"Usage: ./asm file.asm\n";
        return 0;
    }

    initOpcode();

    string file=argv[1];
    ifstream fin(file);

    map<int,pair<string,string>> instr;
    map<int,int> pctoline;

    string line;
    int pc=0, ln=0;

    while(getline(fin,line)){

        ln++;

        int c=line.find(';');
        if(c!=string::npos)
            line=line.substr(0,c);

        line=trim(line);
        if(line=="") continue;

        pctoline[pc]=ln;

        string label="";
        int pos=line.find(':');

        if(pos!=-1){

            label=trim(line.substr(0,pos));

            if(!validLabel(label))
                errors[ln]+="Error: Label naming rules violated\n";
            else if(labels.count(label))
                errors[ln]+="Error: Duplicate label found\n";
            else
                labels[label]=pc;

            line=line.substr(pos+1);
        }

        line=trim(line);
        if(line=="") continue;

        stringstream ss(line);

        string mnem,op;
        ss>>mnem;
        getline(ss,op);
        op=trim(op);

        if(!opcode.count(mnem)){
            errors[ln]+="Error: Mnemonic is wrong\n";
            continue;
        }

        if(opcode[mnem].second && op=="")
            errors[ln]+="Error: Missing operand\n";

        if(!opcode[mnem].second && op!="")
            errors[ln]+="Error: Unexpected operand present\n";

        if(op.find(',')!=string::npos){
            errors[ln]+="Error: Extra operand present\n";
            if(!validOperand(op))
                errors[ln]+="Error: Operand not correct\n";
        }

        else if(op!=""){

            if(isalpha(op[0])){
                if(!labels.count(op))
                    errors[ln]+="Error: No such label present!\n";
            }
            else{
                if(!validOperand(op))
                    errors[ln]+="Error: Operand not correct\n";
            }
        }

        if(mnem=="SET"){
            if(label!="")
                SETlabels[label]=stoi(op);
        }
        else{
            instr[pc]={mnem,op};
            pc++;
        }
    }

    string base=file.substr(0,file.find('.'));
    ofstream log(base+".log");

    if(!errors.empty()){

        log<<"Assembly failed due to errors:\n";

        for(auto &e:errors)
            log<<"line "<<e.first<<" : "<<e.second<<"\n";

        return 0;
    }

    log<<"Compiled successfully";
    log.close();

    vector<pair<int,int>> code;

    for(int i=0;i<instr.size();i++){

        string m=instr[i].first;
        string o=instr[i].second;

        int opc=opcode[m].first;
        int arg=0;

        if(opcode[m].second)
            arg=getValue(o,i,opc);

        if(m=="data"){
            opc=arg & 255;
            arg >>=8;
        }

        code.push_back({arg,opc});
    }

    ofstream obj(base+".o",ios::binary);
    ofstream lst(base+".lst");

    for(auto &c:code)
        obj<<toHex((c.first<<8)|c.second);

    for(int i=0;i<code.size();i++){

        string pc_hex=toHex(i);

        lst<<pc_hex<<" ";

        for(auto &l:labels)
            if(l.second==i)
                lst<<"        "<<l.first<<":\n"<<pc_hex<<" ";

        lst<<toHex((code[i].first<<8)|code[i].second)<<" ";
        lst<<instr[i].first<<" "<<instr[i].second<<"\n";
    }

    obj.close();
    lst.close();
}