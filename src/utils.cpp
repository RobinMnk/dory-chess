//
// Created by Robin on 29.06.2022.
//

#include "utils.h"

namespace Utils {

    /*
    void add_piece_optimized(ByteBoard byteboard, BB pieces, char letter) {
        BB copied_pieces = pieces;
        Bitloop(copied_pieces) {
            const square sq = SquareOf(copied_pieces);
            byteboard[sq] = letter;
        }
    }*/

    void add_piece(ByteBoard &byteboard, BB pieces, char letter) {
        BB selector = 1;
        for (int i = 0; i < 64; i++) {
            if (pieces & selector) {
                if (byteboard[i]) {
                    throw std::runtime_error(
                            "two pieces on " + squarename(newMask(i)) + ": " + static_cast<char>(byteboard[i]) +
                            " and " + letter);
                }
                byteboard[i] = letter;
            }
            selector <<= 1;
        }
    }

    ByteBoard to_byteboard(Board board) {
        ByteBoard byteboard{};
        add_piece(byteboard, board.wPawns, 'P');
        add_piece(byteboard, board.bPawns, 'p');
        add_piece(byteboard, board.wKnights, 'N');
        add_piece(byteboard, board.bKnights, 'n');
        add_piece(byteboard, board.wBishops, 'B');
        add_piece(byteboard, board.bBishops, 'b');
        add_piece(byteboard, board.wRooks, 'R');
        add_piece(byteboard, board.bRooks, 'r');
        add_piece(byteboard, board.wQueens, 'Q');
        add_piece(byteboard, board.bQueens, 'q');
        add_piece(byteboard, board.wKing, 'K');
        add_piece(byteboard, board.bKing, 'k');
        return byteboard;
    }

    void print_board(ByteBoard &&byteboard) {
        int rank = 7;
        while (rank >= 0) {
            for (int i = 0; i < 8; i++) {
                uint8_t letter = byteboard.at(i + rank * 8);
                if (letter != 0) std::cout << letter << " ";
                else std::cout << "  ";
            }
            std::cout << "\n";
            rank--;
        }
        std::cout << std::endl;
    }


    void print_board(Board &board) {
        print_board(to_byteboard(board));
    }

    void printOcc(BB occ) {
        int rank = 7;
        while (rank >= 0) {
            for (int file = 0; file < 8; file++) {
                if (hasBitAt(occ, 8 * rank + file))
                    std::cout << squarename(file, rank) << " ";
                else std::cout << "  ";
            }
            std::cout << "\n";
            rank--;
        }
        std::cout << std::endl;
    }

    std::string squarename(int file, int rank) {
        std::ostringstream oss;
        oss << filename(file) << (rank + 1);
        return oss.str();
    }

    std::string squarename(BB board) {
        int index = singleBitOf(board);
        return squarename(fileOf(index), rankOf(index));
    }

    int sqId(std::string_view name) {
        int file{0};
        for (char c: FILE_NAMES) {
            if (name.at(0) == c) break;
            file++;
        }
        int rank{name.at(1) - '0' - 1};
        return 8 * rank + file;
    }

    BB sqBB(std::string &name) {
        return newMask(sqId(name));
    }

    BB sqBB(std::string &&name) {
        return newMask(sqId(name));
    }

}