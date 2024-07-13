//
// Created by Robin on 13.07.2024.
//

#include <string>
#include <iostream>

int main() {
    std::string cmd;
    while(cmd != "quit") {
        std::cin >> cmd;

        std::cout << "  you said:  " << cmd << std::endl;
    }
}