//
// Created by Robin on 29.06.2022.
//

#include <array>
#include <iostream>
#include <sstream>
#include "board.h"

#ifndef CHESSENGINE_UTILS_H
#define CHESSENGINE_UTILS_H

namespace Utils {

    using ByteBoard = std::array<uint8_t, 64>;

    static const std::array<char, 14> PIECE_TO_CHAR{
        ' ', 'K', 'Q', 'R', 'B', 'N', 'P',
        ' ', 'k', 'q', 'r', 'b', 'n', 'p',
    };
    static const std::array<char, 8> FILE_NAMES{
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };

    void print_board(Board& board);

    void print_board(ByteBoard&& byteboard);

    ByteBoard to_byteboard(Board board);

    template<bool white>
    constexpr char piece_as_char(uint8_t piece) {
        return PIECE_TO_CHAR.at(white ? piece : (piece + 7));
    }

    constexpr char filename(int file) {
        return FILE_NAMES.at(file);
    }

    std::string squarename(int file, int rank);

    std::string squarename(BB board);

    template<bool whiteMoving>
    void printMove(Move m) {
        char piece = piece_as_char<whiteMoving>(m.piece);
        std::cout << piece << ": " << squarename(m.from) << " -> " << squarename(m.to) << std::endl;
    }

    template<bool whiteMoving>
    std::string moveName(Move m) {
        char piece = piece_as_char<whiteMoving>(m.piece);
        std::stringstream bss{};
        bss << piece << ": " << squarename(m.from) << " -> " << squarename(m.to);
        return bss.str();
    }

    template<bool whiteMoving>
    std::string moveNameShort(Move m) {
        std::stringstream bss{};
        bss << squarename(m.from) << squarename(m.to);
        return bss.str();
    }

    void printOcc(BB occ);

    int sqId(std::string_view name);
    BB sqBB(std::string& name);
    BB sqBB(std::string&& name);

}

#endif //CHESSENGINE_UTILS_H
