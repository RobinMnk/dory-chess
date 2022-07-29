//
// Created by Robin on 29.06.2022.
//

#include <array>
#include <iostream>
#include <sstream>
#include <chrono>
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

    template<typename Collector, State state, int depth>
    void time_movegen(Board& board) {
        auto t1 = std::chrono::high_resolution_clock::now();
        Collector::template generateGameTree<state, depth>(board);
        auto t2 = std::chrono::high_resolution_clock::now();

        /* Getting number of milliseconds as an integer. */
        auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

        /* Getting number of milliseconds as a double. */
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;

        /* Getting number of milliseconds as a double. */
        std::chrono::duration<double> seconds = t2 - t1;
        double mnps = (static_cast<double>(Collector::totalNodes) / 1000000) / seconds.count();

        std::cout << "Generated " << Collector::totalNodes << " totalNodes in " << ms_int.count() << "ms\n";
        std::cout << mnps << " M nps\n\n";
    }
}

#endif //CHESSENGINE_UTILS_H
