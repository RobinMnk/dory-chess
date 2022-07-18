//
// Created by robin on 17.07.2022.
//

#ifndef CHESSENGINE_MOVECOLLECTORS_H
#define CHESSENGINE_MOVECOLLECTORS_H

#include <unordered_map>
#include "movegen.h"

template<bool print>
class MoveCollectorStandard {
public:
    static unsigned long long totalNodes;

    template<State state, int depth>
    static void generateGameTree(Board& board) {
        totalNodes = 0;
        build<state, depth>(board);
    }

    template<State state, int depth>
    static void build(Board& board) {
        if constexpr (depth > 0) {
            MoveGenerator<MoveCollectorStandard<print>>::template generate<state, depth>(board);
        }
    }

    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {
        if constexpr (depth == 1) {
            totalNodes++;
        }

        if constexpr (print) {
            for(int i = 0; i < 3 - depth; i++){
                std::cout << "\t";
            }
            Move m {from, to, piece, flags};
            printMove<state.whiteToMove>(m);
        }
    }

    template<State nextState, int depth>
    static void next(Board& nextBoard) {
        build<nextState, depth-1>(nextBoard);
    }
};

template<bool print>
unsigned long long MoveCollectorStandard<print>::totalNodes{0};


class MoveCollectorDivide {
public:
    static std::vector<std::string> moves;
    static std::vector<uint64_t> nodes;
    static unsigned long long curr, totalNodes;
    static int maxDepth;

    template<State state, int depth>
    static void generateGameTree(Board& board) {
        maxDepth = depth;
        build<state, depth>(board);
    }

    template<State state, int depth>
    static void build(Board& board) {
        if constexpr (depth > 0) {
            MoveGenerator<MoveCollectorDivide>::template generate<state, depth>(board);
        }
    }

    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {

        if (depth == maxDepth) {
            Move m{from, to, piece, flags};
            moves.push_back(moveNameShort<state.whiteToMove>(m));
            nodes.push_back(0);
            curr++;
        }

        if constexpr(depth == 1) {
            nodes.back()++;
            totalNodes++;
        }
    }

    template<State nextState, int depth>
    static void next(Board& nextBoard) {
        build<nextState, depth-1>(nextBoard);
    }

    static void print() {
        for(int i{0}; i < curr; i++) {
            std::cout << moves.at(i) << ": " << nodes.at(i) << std::endl;
        }

        std::cout << "\nTotal nodes searched: " << totalNodes << std::endl;
    }
};

std::vector<std::string> MoveCollectorDivide::moves{};
std::vector<uint64_t> MoveCollectorDivide::nodes{};
unsigned long long MoveCollectorDivide::curr{0};
unsigned long long MoveCollectorDivide::totalNodes{0};
int MoveCollectorDivide::maxDepth{1};

#endif //CHESSENGINE_MOVECOLLECTORS_H
