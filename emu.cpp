/*

TITLE        : SIMPLEX Machine Emulator
FILE NAME    : emu.cpp

AUTHOR       : Harendra Kumar
ROLL NO.     : 2401CS61

Declaration of Authorship:
This emu.cpp file is part of the CS2206 (Computer Architecture) mini-project
at the Department of Computer Science and Engineering, IIT Patna.

*/

#include <bits/stdc++.h>
using namespace std;

// Renamed Globals
map<int, string> mnemonic_map;
int sys_mem[10000];
int regA = 0, regB = 0, regPC = 0, regSP = 9999;


// Instruction Set Architecture mapping
void init_isa_table() {
    mnemonic_map[0] = "ldc";   mnemonic_map[1] = "adc";
    mnemonic_map[2] = "ldl";   mnemonic_map[3] = "stl";
    mnemonic_map[4] = "ldnl";  mnemonic_map[5] = "stnl";
    mnemonic_map[6] = "add";   mnemonic_map[7] = "sub";
    mnemonic_map[8] = "shl";   mnemonic_map[9] = "shr";
    mnemonic_map[10] = "adj";  mnemonic_map[11] = "a2sp";
    mnemonic_map[12] = "sp2a"; mnemonic_map[13] = "call";
    mnemonic_map[14] = "return";mnemonic_map[15] = "brz";
    mnemonic_map[16] = "brlz"; mnemonic_map[17] = "br";
    mnemonic_map[18] = "HALT"; mnemonic_map[-1] = "data";
    mnemonic_map[-2] = "SET";
}


// convert integer to 8 character hexadecimal string
string format_hex8(int val) {
    stringstream stream;
    stream << setfill('0') << setw(8) << hex << (uint32_t)val;
    return stream.str();
}


// creating file of current memory state
void execute_memory_dump(ofstream &log, int limit) {
    string header = "\n--- MEMORY STATE DUMP ---\n";
    cout << header; log << header;

    for (int i = 0; i < limit; i++)
    {
        if (i % 4 == 0) {
            string addr_label = "\n" + format_hex8(i) + "\t";
            cout << addr_label; log << addr_label;
        }
        string val_str = format_hex8(sys_mem[i]) + " ";
        cout << val_str; log << val_str;
    }
    cout << endl;
}


// execution engine
void run_emulator_logic(ofstream &trace_out, int initial_pc) {
    "\n--- EXECUTION TRACE ---\n\n";
    cout << "\n--- EXECUTION TRACE ---\n\n"; 
    trace_out << "\n--- EXECUTION TRACE ---\n\n";

    set<int> branch_ops{13, 15, 16, 17};
    bool HALT = false;
    int line_cnt = 0;

    while (true) {
        int raw_instr = sys_mem[regPC];

        // extract Opcode (8-bit) and handle signs
        int32_t op_byte = raw_instr & 0xFF;
        if (op_byte >= 128) op_byte -= 256; 

        // extract Operand (24-bit) with sign extension
        int32_t operand = (int32_t)raw_instr >> 8;

        if (mnemonic_map.count(op_byte)) {
            stringstream status;
            status << "PC: " << format_hex8(regPC) << " SP: " << format_hex8(regSP)
                   << " A: " << format_hex8(regA) << " B: " << format_hex8(regB)
                   << " " << mnemonic_map[op_byte] << " " << operand << "\n";
            
            cout << status.str();
            trace_out << status.str();
        }
        else HALT = true;

        // ins. execution logic
        switch (op_byte) {
            case 0:  regB = regA; regA = operand; break;
            case 1:  regA += operand; break;
            case 2:  regB = regA; regA = sys_mem[regSP + operand]; break;
            case 3:  sys_mem[regSP + operand] = regA; regA = regB; break;
            case 4:  regA = sys_mem[regA + operand]; break;
            case 5:  sys_mem[regA + operand] = regB; break;
            case 6:  regA += regB; break;
            case 7:  regA = regB - regA; break;
            case 8:  regA = regB << regA; break;
            case 9:  regA = regB >> regA; break;
            case 10: regSP += operand; break;
            case 11: regSP = regA; regA = regB; break;
            case 12: regB = regA; regA = regSP; break;
            case 13: regB = regA; regA = regPC; regPC += operand; break;
            case 14: if (regPC == regA && regA == regB) HALT = true;
                     regPC = regA; regA = regB; break;
            case 15: if (regA == 0) regPC += operand; break;
            case 16: if (regA < 0) regPC += operand; break;
            case 17: regPC += operand; break;
            case 18: HALT = true; break;
            default: break;
        }

        if (regSP >= 10000) {
            cout << "Critical: Stack Overflow at PC " << regPC << endl;
            HALT = true;
        }
        if (branch_ops.count(op_byte) && operand == -1) {
            cout << "Warning: Infinite loop trap detected" << endl;
            HALT = true;
        }

        if (HALT) break;
        regPC++;
        line_cnt++;
    }
    cout << "\nFinished. Total Instructions: " << line_cnt << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Usage: ./emu [option] input.o\nOptions:\n\t-t: Trace\n\t-b: Before Dump\n\t-a: After Dump\n\t-ISA: Show ISA\n";
        return 0;
    }

    init_isa_table();
    string cmd = argv[1];
    string file_in = argv[2];

    size_t ext_pos = file_in.find_last_of('.');
    if (ext_pos == string::npos || file_in.substr(ext_pos) != ".o") {
        cerr << "Error: Invalid object file format.\n";
        return 1;
    }

    ifstream data_src(file_in);
    ofstream data_trace(file_in.substr(0, ext_pos) + ".trace");

    if (!data_src.is_open()) { cerr << "Source file missing.\n"; return 1; }

    string binary_blob;
    getline(data_src, binary_blob);
    if (binary_blob.empty()) { cout << "File is empty.\n"; return 0; }

    // load binary into virtual memory
    int addr = 0;
    for (size_t i = 0; i < binary_blob.length(); i += 8) {
        string chunk = binary_blob.substr(i, 8);
        uint32_t val = stoul(chunk, nullptr, 16);
        
        // Check if it's a data directive or encoded instruction
        int32_t op_check = val & 0xFF;
        if (op_check >= 128) op_check -= 256;

        if (op_check < 0) sys_mem[addr] = (int32_t)val >> 8;
        else sys_mem[addr] = (int32_t)val;
        
        addr++;
    }

    // command handling

    if (cmd == "-ISA") {
        cout << "OPCODE\tMNEMONIC\tARG\n";
        for(int k=0; k<=18; k++) cout << k << "\t" << mnemonic_map[k] << endl;
    }
    else if (cmd == "-b") execute_memory_dump(data_trace, addr);
    else if (cmd == "-t") run_emulator_logic(data_trace, regPC);
    else if (cmd == "-a") {
        run_emulator_logic(data_trace, regPC);
        execute_memory_dump(data_trace, addr);
    }
    return 0;
}