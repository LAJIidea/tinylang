//
// Created by BY210033 on 2022/11/9.
//
#include "tinylang/AST/AST.h"
#include <string>
#include <iostream>
#include <llvm/ADT/StringMap.h>


using namespace tinylang;
using namespace llvm;
int main()
{
    StringMap<std::string> map;
    map.insert(std::pair<StringRef, std::string>("aa", "bb"));

    bool c1 = map.insert(std::pair<StringRef, std::string>("aa", "bb")).second;
    std::cout << c1 << std::endl;



    bool c2 = map.insert(std::pair<StringRef, std::string>("aa", "b2")).second;
    std::cout << c2 << std::endl;

    std::cout << map.find("aa")->second << std::endl;

    bool c3 = map.insert(std::pair<StringRef, std::string>("a2", "bb")).second;
    std::cout << c3 << std::endl;
}
