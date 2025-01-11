//
// Created by robin on 14.07.2024.
//

#ifndef DORY_MOVEORDERING_H
#define DORY_MOVEORDERING_H

#include "../core/checklogichandler.h"
#include "engineparams.h"

namespace Dory::Search {

    class MoveOrderer {
        const int Large = 1000000;

        template<bool whiteToMove>
        int isForwardMove(BB from, BB to) {
            if constexpr (whiteToMove) {
                return (singleBitOf(to) - singleBitOf(from)) / 8;
            } else {
                return (singleBitOf(from) - singleBitOf(to)) / 8;
            }
        }

        std::array<std::array<Move, 4>, 128> killerMoves;
        std::array<int, 128> kmPositions{};
    public:
        Move priorityMove;

        void reset() {
            kmPositions.fill(0);
        }

        void addKillerMove(Move move, int dp) {
            killerMoves[dp][kmPositions[dp]++] = move;
            kmPositions[dp] %= 4;
        }

        template<typename T>
        void sort(std::vector<std::pair<float, T>> &moves) {
            std::sort(moves.begin(), moves.end(),
                  [](const std::pair<float, T>& a, const std::pair<float, T>& b) { return a.first > b.first; }
            );
        }

        template<bool whiteToMove, Piece_t piece, Flag_t flags = MOVEFLAG_Silent>
        int moveHeuristic(const Board &board, BB from, BB to, const PDptr &pd, int depth) {
            if (priorityMove.is<piece, flags>(from, to)) {
                return INF;
            }

            for(int i = 0; i < kmPositions[depth]; i++) {
                if(killerMoves[depth][i].is<piece, flags>(from, to)) {
                    return 64 * Large;
                }
            }

            int heuristic_val{0};

            // See if move results in a check!
            // -> That is the case when the 'to' square can reach the enemyKing for the given piece Type


            /// Captures
            if (to & board.enemyPieces<whiteToMove>()) {
                int valueDiff = -pieceValue<piece>();

                if (board.enemyPawns<whiteToMove>() & to)
                    valueDiff += pieceValue<PIECE_Pawn>();
                else if (board.enemyKnights<whiteToMove>() & to)
                    valueDiff += pieceValue<PIECE_Knight>();
                else if (board.enemyBishops<whiteToMove>() & to)
                    valueDiff += pieceValue<PIECE_Bishop>();
                else if (board.enemyRooks<whiteToMove>() & to)
                    valueDiff += pieceValue<PIECE_Rook>();
                else if (board.enemyQueens<whiteToMove>() & to)
                    valueDiff += pieceValue<PIECE_Queen>();

                heuristic_val += 2 * Large + valueDiff;

                // Good Capture
                if (valueDiff >= -25) {
                    heuristic_val += 8 * Large;
                }
//            } else {
//                for(int i = 0; i < kmPositions[depth]; i++)
//                    if(killerMoves[depth][i].is<piece, flags>(from, to)) {
//                        heuristic_val += Large;
//                        break;
//                    }
            }

            /// Promotions
            if constexpr (flags == MOVEFLAG_PromoteBishop) {
                heuristic_val += 3 * Large + 3200;
            }
            if constexpr (flags == MOVEFLAG_PromoteKnight) {
                heuristic_val += 3 * Large + 3000;
            }
            if constexpr (flags == MOVEFLAG_PromoteRook) {
                heuristic_val += 3 * Large + 5000;
            }
            if constexpr (flags == MOVEFLAG_PromoteQueen) {
                heuristic_val += 7 * Large;
            }

            //        heuristic_val += pieceValue<piece>() / 200;

            /// Do not move to an attacked square
            if (to & pd->pawnAtk) {
                heuristic_val -= pieceValue<piece>() * 4;
            } else if (to & pd->attacked) {
                heuristic_val -= 25; // pieceValue<piece>();
            }

            /// Move towards enemy king
            heuristic_val += isForwardMove<whiteToMove>(from, to) / 4;
            heuristic_val += (8 - PieceSteps::DIST[singleBitOf(from)][board.enemyKingSquare<whiteToMove>()]) * 64;


            //        if(to & pd->attacked) {
            //            heuristic_val -= pieceValue<piece>() / 1024;
            //        }

            /// Activity difference
            int activity_diff_mg = ENGINE_PARAMS.middleGamePieceTable<piece, whiteToMove>(firstBitOf(to))
                                   - ENGINE_PARAMS.middleGamePieceTable<piece, whiteToMove>(firstBitOf(from));
            int activity_diff_eg = ENGINE_PARAMS.endGamePieceTable<piece, whiteToMove>(firstBitOf(to))
                                   - ENGINE_PARAMS.endGamePieceTable<piece, whiteToMove>(firstBitOf(from));

            heuristic_val += activity_diff_mg + activity_diff_eg;

            return heuristic_val;
        }

    }; // class MoveOrderer

} // namespace Dory::Search

#endif //DORY_MOVEORDERING_H
