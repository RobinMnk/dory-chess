//
// Created by Robin on 29.06.2022.
//

#include <array>
#include <iostream>
#include "board.h"

#ifndef CHESSENGINE_UTILS_H
#define CHESSENGINE_UTILS_H


using ByteBoard = std::array<uint8_t, 64>;

void print_board(Board board);

void print_board(ByteBoard&& byteboard);

ByteBoard to_byteboard(Board board);

#endif //CHESSENGINE_UTILS_H
