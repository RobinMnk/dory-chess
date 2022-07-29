//
// Created by robin on 17.07.2022.
//

#ifndef CHESSENGINE_MOVECOLLECTORS_H
#define CHESSENGINE_MOVECOLLECTORS_H

#include <unordered_map>
#include <vector>
#include "movegen.h"
#include "utils.h"

namespace MoveCollectors {
    template<bool saveList, bool print>
    class LimitedDFS {
    public:
        static unsigned long long totalNodes;
        static std::vector<Board> positions;

        template<State state, int depth>
        static void generateGameTree(Board& board) {
            totalNodes = 0;
            build<state, depth>(board);

            if constexpr (saveList)
                totalNodes = positions.size();
        }

        template<State state, int depth>
        static void build(Board& board) {
            if constexpr (depth > 0) {
                MoveGenerator<LimitedDFS<saveList, print>>::template generate<state, depth>(board);
            }
        }

        template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
        static void registerMove(const Board &board, BB from, BB to) {
            if constexpr (depth == 1) {
                if constexpr (saveList)
                    positions.push_back(board);
                else
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

    template<bool saveList, bool print>
    unsigned long long LimitedDFS<saveList, print>::totalNodes{0};
    template<bool saveList, bool print>
    std::vector<Board> LimitedDFS<saveList, print>::positions{};


    class OpenDFS {
    public:
        static unsigned long long totalNodes;
        static unsigned int maxDepth, currDepth;

        template<State state, int depth>
        static void generateGameTree(Board& board) {
            totalNodes = 0;
            maxDepth = depth;
            currDepth = 0;
            MoveGenerator<OpenDFS>::template generate<state, 1>(board);
        }

        template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
        static void registerMove([[maybe_unused]] const Board &board, BB from, BB to) {
            if (currDepth == maxDepth - 1) {
                totalNodes++;
            }
        }

        template<State nextState, int depth>
        static void next(Board& nextBoard) {
            currDepth++;
            if(currDepth < maxDepth)
                MoveGenerator<OpenDFS>::template generate<nextState, 1>(nextBoard);
            currDepth--;
        }
    };

    unsigned long long OpenDFS::totalNodes{0};
    unsigned int OpenDFS::maxDepth{0}, currDepth{0};



    class Divide {
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
                MoveGenerator<Divide>::template generate<state, depth>(board);
            }
        }

        template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
        static void registerMove(const Board &board, BB from, BB to) {

            if (depth == maxDepth) {
                Move m{from, to, piece, flags};
                moves.push_back(Utils::moveNameShort<state.whiteToMove>(m));
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
            for(unsigned int i{0}; i < curr; i++) {
                std::cout << moves.at(i) << ": " << nodes.at(i) << std::endl;
            }

            std::cout << "\nTotal nodes searched: " << totalNodes << std::endl;
        }
    };

    std::vector<std::string> Divide::moves{};
    std::vector<uint64_t> Divide::nodes{};
    unsigned long long Divide::curr{0};
    unsigned long long Divide::totalNodes{0};
    int Divide::maxDepth{1};
}

#endif //CHESSENGINE_MOVECOLLECTORS_H
