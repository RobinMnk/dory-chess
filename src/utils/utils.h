//
// Created by Robin on 29.06.2022.
//

#ifndef DORY_UTILS_H
#define DORY_UTILS_H

#include <array>
#include <chrono>
#include <ranges>
#include <sstream>
#include "../core/board.h"

namespace Dory::Utils {

    using ByteBoard = std::array<uint8_t, 64>;

    static const std::array<char, 14> PIECE_TO_CHAR{
            ' ', 'K', 'Q', 'R', 'B', 'N', 'P',
            ' ', 'k', 'q', 'r', 'b', 'n', 'p',
    };
    static const std::array<char, 8> FILE_NAMES{
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };

    constexpr char filename(int file) {
        return FILE_NAMES.at(file);
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

    ByteBoard to_byteboard(const Board &board) {
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
        add_piece(byteboard, newMask(board.wKingSq), 'K');
        add_piece(byteboard, newMask(board.bKingSq), 'k');
        return byteboard;
    }

    void print_board(const ByteBoard &&byteboard) {
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

    std::string specialMove(Flag_t flags) {
        switch (flags) {
            case MOVEFLAG_PromoteBishop:
                return "=B";
            case MOVEFLAG_PromoteKnight:
                return "=N";
            case MOVEFLAG_PromoteRook:
                return "=R";
            case MOVEFLAG_PromoteQueen:
                return "=Q";
            case MOVEFLAG_ShortCastling:
                return "0-0";
            case MOVEFLAG_LongCastling:
                return "0-0-0";
            default:
                return "";
        }
    }

    std::string pieceString(Piece_t piece) {
        switch (piece) {
            case PIECE_King:
                return "K";
            case PIECE_Queen:
                return "Q";
            case PIECE_Rook:
                return "R";
            case PIECE_Bishop:
                return "B";
            case PIECE_Knight:
                return "N";
            default:
                return "";
        }
    }

    void printMove(const Move m) {
        if (m.from() + m.to() == 0) std::cout << "NULL" << std::endl;
        else if (m.flags == MOVEFLAG_ShortCastling) std::cout << specialMove(MOVEFLAG_ShortCastling) << std::endl;
        else if (m.flags == MOVEFLAG_LongCastling) std::cout << specialMove(MOVEFLAG_LongCastling) << std::endl;
        else
            std::cout << pieceString(m.piece) << squarename(m.from()) << "-" << squarename(m.to())
                      << specialMove(m.flags) << std::endl;
    }

    void print_board(const Board &board) {
        print_board(to_byteboard(board));
    }

    void printMoveList(const std::vector<Move> &moves) {
        for (auto &m: moves) {
            if (m.flags == MOVEFLAG_ShortCastling) std::cout << specialMove(MOVEFLAG_ShortCastling);
            else if (m.flags == MOVEFLAG_LongCastling) std::cout << specialMove(MOVEFLAG_LongCastling);
            else
                std::cout << pieceString(m.piece) << squarename(m.from()) << "-" << squarename(m.to())
                          << specialMove(m.flags);
            std::cout << " - ";
        }
        std::cout << std::endl;
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

    template<bool white>
    constexpr char piece_as_char(uint8_t piece) {
        return PIECE_TO_CHAR.at(white ? piece : (piece + 7));
    }

    template<bool whiteMoving>
    std::string moveName(Move m) {
        char piece = piece_as_char<whiteMoving>(m.piece);
        std::stringstream bss{};
        bss << piece << ": " << squarename(m.from()) << " -> " << squarename(m.from());
        return bss.str();
    }

    std::string moveNameShort(Move m) {
        std::stringstream bss{};
        bss << squarename(m.from()) << squarename(m.to());
        return bss.str();
    }

    std::string moveNameNotation(Move m) {
        std::stringstream bss{};
        if (m.flags == MOVEFLAG_ShortCastling) bss << specialMove(MOVEFLAG_ShortCastling);
        else if (m.flags == MOVEFLAG_LongCastling) bss << specialMove(MOVEFLAG_LongCastling);
        else bss << pieceString(m.piece) << squarename(m.from()) << "-" << squarename(m.to());
        bss << specialMove(m.flags);
        return bss.str();
    }

    std::string moveNameShortNotation(Move m) {
        std::stringstream bss{};
        if (m.flags == MOVEFLAG_ShortCastling) bss << specialMove(MOVEFLAG_ShortCastling);
        else if (m.flags == MOVEFLAG_LongCastling) bss << specialMove(MOVEFLAG_LongCastling);
        else bss << pieceString(m.piece) << squarename(m.to());
        bss << specialMove(m.flags);
        return bss.str();
    }

    void printLine(const std::vector<Move>& line, int eval) {
        if (eval == INF-1) {
            std::cout << "Checkmate - White wins!" << std::endl;
            return;
        }
        if (eval == -INF+1) {
            std::cout << "Checkmate - Black wins!" << std::endl;
            return;
        }
        if(eval > INF - 50) {
            std::cout << "M" << (INF - eval) / 2 ;
        } else if(eval < -INF + 50) {
            std::cout << "-M" << (INF + eval) / 2;
        } else
            std::cout << static_cast<float>(eval) / 100;
        std::cout << ":  ";
        for (auto& it : std::ranges::reverse_view(line)) {
            std::cout << Utils::moveNameShortNotation(it) << " ";
        }
        std::cout << std::endl;
    }

//    template<typename Collector, State state, int depth>
//    void time_movegen(const Board &board) {
//        auto t1 = std::chrono::high_resolution_clock::now();
//        Collector::template generateGameTree<state, depth>(board);
//        auto t2 = std::chrono::high_resolution_clock::now();
//
//        /* Getting number of milliseconds as an integer. */
//        auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);
//
//        /* Getting number of milliseconds as a double. */
//        std::chrono::duration<double, std::milli> ms_double = t2 - t1;
//
//        /* Getting number of milliseconds as a double. */
//        std::chrono::duration<double> seconds = t2 - t1;
//        double mnps = (static_cast<double>(Collector::totalNodes) / 1000000) / seconds.count();
//
//        std::cout << "Generated " << Collector::totalNodes << " nodes in " << ms_int.count() << "ms\n";
//        std::cout << mnps << " M nps\n\n";
//    }
//
//
//    template<State state>
//    struct MoveSimulator {
//        Board board;
//
//        explicit MoveSimulator(Board &bd) : board{bd} {}
//
//        template<Piece_t piece, Flag_t flag = MOVEFLAG_Silent>
//        MoveSimulator<getNextState<state, flag>()> move(std::string_view from, std::string_view to) {
//            BB fromBB = newMask(sqId(from));
//            BB toBB = newMask(sqId(to));
//            Board nextBoard = board.fork<state, piece, flag>(fromBB, toBB);
//            return MoveSimulator<getNextState<state, flag>()>(nextBoard);
//        }
//
//        constexpr State getState() {
//            return state;
//        }
//    };
}

#endif //DORY_UTILS_H
