#include<iostream>
#include<vector>
#include<string>
#include<list>
#include "m68k_instructions.h"

int main() {
    std::vector<std::list<std::string>> types(65536, std::list<std::string>{"none"});
    for(int i=0;i<65536;i++) {
        for(auto& j: instrs) {
            int c = (i & (j.mask_bits));
            if(c == j.id_bits) {
                types[i].push_back(j.op_name);
            }
        }
        std::cout<<i<<": ";
        for(auto& j: types[i]) {
            std::cout<<j<<", ";
        }
        std::cout<<"\n";
    }
    return 0;
}
