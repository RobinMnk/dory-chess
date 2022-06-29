#include <iostream>
#include "board.h"
#include "utils.h"

int main() {
    std::cout << "Chess Engine" << std::endl;

    Board start = STARTBOARD;

    print_board(start);

    return 0;
}
