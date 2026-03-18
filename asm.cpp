/*

TITLE        : SIMPLEX Two Pass Assembler
FILE NAME    : asm.cpp

AUTHOR       : Harendra Kumar
ROLL NO.     : 2401CS61

Declaration of Authorship:
This asm.cpp file is part of the CS2206 (Computer Architecture) mini-project
at the Department of Computer Science and Engineering, IIT Patna.

*/

#include <bits/stdc++.h>
using namespace std;

// globals renamed for a fresh look
map<string, pair<int, int>> isa_lookup;
map<string, int> definition_labels;

// func prototypes
void setup_instruction_table(map<string, pair<int, int>> &table);
void clean_string(string &content);
int convert_base_to_int(string &val, int offset, int base_type);
string format_as_hex(int val);
string build_machine_word(const pair<int, int> data_pair);
void fetch_data_value(string &token, int &out_val, map<string, int> &symbol_map, int op, int current_pc);

// Pass 1: syntax analysis and symbol mapping
void analyze_and_map(map<int, pair<string, string>> &instr_map, fstream &src, map<string, int> &symbol_map, map<int, string> &log_map, int &pc_val, int &line_idx, map<int, int> &pc_to_line) {
    string buffer;
    while (getline(src, buffer)) {
        line_idx++;
        // strip comments
        buffer = regex_replace(buffer, regex(";.*$"), string(""));
        clean_string(buffer);
        
        if (buffer == "") continue;
        
        pc_to_line[pc_val] = line_idx;
        bool found_id = false;
        size_t colon_idx = 0;
        string current_label;

        // scanner for labels
        for (; colon_idx < buffer.length(); colon_idx++) {
            if (buffer[colon_idx] == ':') {
                found_id = true;
                current_label = buffer.substr(0, colon_idx);
                
                // validate identifier naming
                if (!isalpha(current_label[0]) || find_if(current_label.begin(), current_label.end(), [](char c) { return !(isalnum(c)); }) != current_label.end()) {
                    log_map[line_idx] += "Fault: Invalid label syntax\n";
                    found_id = false;
                }
                if (current_label == "") {
                    log_map[line_idx] += "Fault: Null label detected\n";
                    found_id = false;
                }
                if (symbol_map.count(current_label)) {
                    log_map[line_idx] += "Fault: Duplicate identifier found\n";
                    found_id = false;
                }

                if (found_id) symbol_map[current_label] = pc_val;
                break;
            }
        }
        if (colon_idx == buffer.length()) colon_idx = -1;

        // isolate instruction
        string code_segment(buffer.begin() + colon_idx + 1, buffer.end());
        clean_string(code_segment);
        if (code_segment == "") continue;

        // split mnemonic and argument
        int split_pt = code_segment.length();
        for (size_t k = 0; k < code_segment.length(); k++) {
            if (code_segment[k] == ' ') {
                split_pt = k;
                break;
            }
        }
        
        string mnem(code_segment.begin(), code_segment.begin() + split_pt);
        string arg(code_segment.begin() + split_pt, code_segment.end());
        clean_string(mnem);
        clean_string(arg);

        // validation
        bool valid_cmd = true;
        if (isa_lookup.find(mnem) == isa_lookup.end()) {
            log_map[line_idx] += "Fault: Unrecognized mnemonic\n";
            valid_cmd = false;
        }
        if (valid_cmd && isa_lookup[mnem].second == 1 && arg == "") {
            log_map[line_idx] += "Fault: Required operand missing\n";
            valid_cmd = false;
        }
        if (valid_cmd && isa_lookup[mnem].second == 0 && arg.length() > 0) {
            log_map[line_idx] += "Fault: Instruction does not take operands\n";
            valid_cmd = false;
        }

        if (mnem == "SET") {
            if (colon_idx == -1)
                log_map[line_idx] += "Fault: SET used without label\n";
            else {
                int val_arg;
                fetch_data_value(arg, val_arg, symbol_map, -1, pc_val);
                definition_labels.insert({current_label, val_arg});
            }
        }
        else {
            instr_map[pc_val] = {mnem, arg};
            pc_val++;
        }
    }
}

void fetch_data_value(string &token, int &out_val, map<string, int> &symbol_map, int op, int current_pc) {
    if (((token[0] <= 'z' && token[0] >= 'a') || (token[0] <= 'Z' && token[0] >= 'A'))) {
        if (definition_labels.count(token)) out_val = definition_labels[token];
        else out_val = symbol_map[token];
    }
    else {
        if (token.substr(0, 2) == "0x") out_val = convert_base_to_int(token, 2, 16);
        else if (token[0] == '0') out_val = convert_base_to_int(token, 1, 8);
        else out_val = convert_base_to_int(token, 0, 10);
    }
    
    // PC-relative offset adjustment
    if (op == 13 || op == 15 || op == 16 || op == 17)
        out_val = out_val - (current_pc + 1);
}

void validate_args(map<int, pair<string, string>> &instr_map, map<string, int> &symbol_map, map<int, string> &log_map, map<int, int> pc_to_line) {
    for (auto &&entry : instr_map) {
        bool is_label = false;
        string op_str = entry.second.second;
        
        if (any_of(op_str.begin(), op_str.end(), [](char c) { return c == ','; }))
            log_map[pc_to_line[entry.first]] += "Fault: Illegal comma in operand\n";

        if (((op_str[0] <= 'z' && op_str[0] >= 'a') || (op_str[0] <= 'Z' && op_str[0] >= 'A'))){
            if (!symbol_map.count(op_str))
                log_map[pc_to_line[entry.first]] += "Fault: Undefined symbol used\n";
            is_label = true;
        }
        
        if (!is_label && !op_str.empty()) {
            // hex/oct/dec validation loop
            if (op_str.substr(0, 2) == "0x") {
                for (int j = 2; j < op_str.length(); j++) {
                    char c = op_str[j];
                    if (!((c <= 'f' && c >= 'a') || (c <= 'F' && c >= 'A') || (c >= '0' && c <= '9'))) {
                        log_map[pc_to_line[entry.first]] += "Fault: Format error in Hex\n"; break;
                    }
                }
            } 
            else if (op_str[0] == '0') {
                for (int j = 1; j < op_str.length(); j++) {
                    if (!(op_str[j] <= '7' && op_str[j] >= '0')) {
                        log_map[pc_to_line[entry.first]] += "Fault: Format error in Octal\n"; break;
                    }
                }
            }
        }
    }
}

// pass 2
void finalize_encoding(map<int, pair<string, string>> &instr_map, map<string, int> &symbol_map, map<int, int> &pc_to_line, vector<pair<int, int>> &bin_list) {
    for (size_t i = 0; i < instr_map.size(); i++) {
        string m = instr_map[i].first;
        string a = instr_map[i].second;
        int opc = isa_lookup[m].first;
        int resolved_arg;
        
        if (isa_lookup[m].second) fetch_data_value(a, resolved_arg, symbol_map, opc, i);
        else resolved_arg = 0;
        
        if (m == "data") {
            opc = resolved_arg & 0xFF;
            resolved_arg >>= 8;
        }
        bin_list.push_back({resolved_arg, opc});
    }
}

int main(int argc, char *argv[]) {  
    // argc - argument count (typed in terminal)
    // argv - argument vector(array of strings that you typed)

    if (argc != 2) {
        cout << "Usage: ./asm source.asm\n";
        return 0;
    }
    // argv[0] : assembler code file
    // argv[1] : object file
    string filename = argv[1];
    
    // --------- FILE OPEN ----------
    fstream input_stream(filename);

    // Extract the instructions from input file

    map<int, pair<string, string>> instruction_storage;  // instructions will store mnemonic and operand
    vector<pair<int, int>> encoded_instructions;  // to store {operand, opcode}
    map<string, int> labels_found;  // to store labels on each line { "label_name" : PC_address }
    map<int, string> error_registry;  // to store errors and line no. { line_number : "Error Message" }
    map<int, int> pc_linker;  // to map line number to pc { PC : line_number_in_original_file }

    setup_instruction_table(isa_lookup); // mapping mnemonics to opcodes

    int line_no = 0, curr_pc_address = 0;

    // pass 1: clean the code, identify lables, store ins., link pc to line
    analyze_and_map(instruction_storage, input_stream, labels_found, error_registry, curr_pc_address, line_no, pc_linker);
    // check for correctness of operands
    validate_args(instruction_storage, labels_found, error_registry, pc_linker);

    // If no errors found then run second pass
    if (error_registry.empty())
        // pass 2: convert instructions into binary code
        finalize_encoding(instruction_storage, labels_found, pc_linker, encoded_instructions);

    // Output fines generating
    string core_name = filename.substr(0, filename.find_last_of('.'));
    ofstream log_file(core_name + ".log"); // open for writing overwrite existing content 

    if (!error_registry.empty()) {
        log_file << "Errors encountered during assembly:\n";
        for (auto &&err : error_registry) log_file << "Line " << err.first << " : " << err.second;
        log_file.close();
    }
    else {
        log_file << "Assembly successful.";
        log_file.close();

        ofstream obj_file(core_name + ".o", ios::binary);
        ofstream lst_file(core_name + ".lst");

        for (size_t i = 0; i < encoded_instructions.size(); i++) {
            string hex_addr = format_as_hex(i);
            string hex_code = build_machine_word(encoded_instructions[i]);
            
            for (auto &&l : labels_found) 
                if (l.second == (int)i) 
                    lst_file << "         " << l.first << ":\n";
            
            obj_file << hex_code;
            lst_file << hex_addr << " " << hex_code << " " << instruction_storage[i].first << " " << instruction_storage[i].second << "\n";
        }
        obj_file.close();
        lst_file.close();
    }
    return 0;
}

// Instruction Set Architecture (mapper of instruction to opcode and operands)   
void setup_instruction_table(map<string, pair<int, int>> &table) {
    // table[string] = {opcode, operands}
    table["ldc"] = {0, 1}; table["adc"] = {1, 1}; table["ldl"] = {2, 1};
    table["stl"] = {3, 1}; table["ldnl"] = {4, 1}; table["stnl"] = {5, 1};
    table["add"] = {6, 0}; table["sub"] = {7, 0}; table["shl"] = {8, 0};
    table["shr"] = {9, 0}; table["adj"] = {10, 1}; table["a2sp"] = {11, 0};
    table["sp2a"] = {12, 0}; table["call"] = {13, 1}; table["return"] = {14, 0};
    table["brz"] = {15, 1}; table["brlz"] = {16, 1}; table["br"] = {17, 1};
    table["HALT"] = {18, 0}; table["data"] = {-1, 1}; table["SET"] = {-2, 1};
}

void clean_string(string &s) {
    s = regex_replace(s, regex("^\\s+"), ""); // remove spaces from start
    s = regex_replace(s, regex("\\s+$"), ""); // remove spaces from end
}

int convert_base_to_int(string &n, int start, int base) {  
    if (base == 10) { 
        int multiplier = 1;
        if (n[0] == '-') { multiplier = -1; start = 1; } // number is negative and skip first digit
        else if (n[0] == '+') { start = 1; } // number is positive and skip first digit
        return multiplier * stoi(n.substr(start), 0, 10); // convert string to integer and multiply with sign
    }
    return stoi(n, 0, base);
}

string format_as_hex(int val) {  // convert integer to 8 character hexadecimal string
    stringstream ss; // string builder
    ss << setfill('0') << setw(8) << hex << (unsigned int)val; // setw(8):(setwidth), hex: switch number decimal to hexadecimal
    return ss.str();
}

string build_machine_word(const pair<int, int> data_pair) {  
    int combined = (data_pair.first << 8) | (data_pair.second & 0xFF); // first 24-bit for operand and 8-bit for opcode
    return format_as_hex(combined);  // return in hexadecimal formate
}