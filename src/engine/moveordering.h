//
// Created by robin on 14.07.2024.
//

#ifndef DORY_MOVEORDERING_H
#define DORY_MOVEORDERING_H

#include "../core/checklogichandler.h"
#include "engineparams.h"

namespace Dory::Search {
    class MoveOrderer {
        static constexpr int Large = 1000000;
        static constexpr int NumKillers = 4;

        std::array<std::array<Move, NumKillers>, 128> killerMoves{};
        std::array<int, 128> kmPositions{};

    public:
        Move priorityMove{NULLMOVE};

        void reset() {
            kmPositions.fill(0);
        }

        void addKillerMove(Move move, int depth) {
            killerMoves[depth][kmPositions[depth]++] = move;
            kmPositions[depth] %= NumKillers;
        }

        template<bool whiteToMove, Piece_t piece, Flag_t flags = MOVEFLAG_Silent>
        [[nodiscard]] int moveHeuristic(const Board &board, BB from, BB to, const PinData& pd, int depth) const {
            int heuristic_val = 0;

            const int fromIndex = firstBitOf(from);
            const int toIndex   = firstBitOf(to);
            const bool isCapture = to & board.enemyPieces<whiteToMove>();

            // Priority move
            if (priorityMove.is<piece, flags>(from, to)) {
                heuristic_val += Large * 8;
            }

            // Captures
            if (isCapture) {
                int victimValue = 0;
                if (board.enemyPawns<whiteToMove>() & to)      victimValue = 100;
                else if (board.enemyKnights<whiteToMove>() & to) victimValue = 300;
                else if (board.enemyBishops<whiteToMove>() & to) victimValue = 300;
                else if (board.enemyRooks<whiteToMove>() & to)   victimValue = 500;
                else if (board.enemyQueens<whiteToMove>() & to)  victimValue = 900;

                int attackerValue = pieceValue<piece>();
                heuristic_val += Large + (victimValue - attackerValue); // MVV-LVA

                if (victimValue - attackerValue >= 0)
                    heuristic_val += Large / 2;
            }

            // Killer moves
            if (!isCapture) {
                for (int i = 0; i < kmPositions[depth]; i++) {
                    if (killerMoves[depth][i].is<piece, flags>(from, to)) {
                        heuristic_val += Large / (i + 1); // first killer higher
                    }
                }
            }

            // Gives check
            BB attacks = 0;
            BB nextOcc = board.occ() ^ (from | to);
            if constexpr (piece == PIECE_Pawn) {
                attacks = pawnAtkLeft<whiteToMove>(toIndex & pawnCanGoLeft<whiteToMove>()) | pawnAtkRight<whiteToMove>(toIndex & pawnCanGoRight<whiteToMove>());
            } else if constexpr (piece == PIECE_Knight) {
                attacks = PieceSteps::KNIGHT_MOVES[toIndex];
            } else if constexpr (piece == PIECE_Bishop || piece == PIECE_Queen) {
                attacks = PieceSteps::slideMask<true>(nextOcc, toIndex);
            } else if constexpr (piece == PIECE_Rook || piece == PIECE_Queen) {
                attacks |= PieceSteps::slideMask<false>(nextOcc, toIndex);
            } else if constexpr (piece == PIECE_King) {
                attacks = PieceSteps::KING_MOVES[toIndex];
            }

            if (attacks & board.enemyKing<whiteToMove>()) {
                heuristic_val += Large / 2;
            }

            // Promotions
            if constexpr (isPromotion<flags>()) {
                static constexpr int promotionBonus[4] = {7000, 5000, 3200, 3000}; // queen, rook, bishop, knight
                heuristic_val += Large + promotionBonus[flags - 6];
            }

            // Piece-square
            heuristic_val += ENGINE_PARAMS.middleGamePieceTable<piece, whiteToMove>(toIndex)
                             - ENGINE_PARAMS.middleGamePieceTable<piece, whiteToMove>(fromIndex);
            heuristic_val += ENGINE_PARAMS.endGamePieceTable<piece, whiteToMove>(toIndex)
                             - ENGINE_PARAMS.endGamePieceTable<piece, whiteToMove>(fromIndex);

            // Avoid moving to attacked squares
            if (to & pd.pawnAtk) {
                heuristic_val -= pieceValue<piece>() * 4;
            } else if (to & pd.attacked) {
                heuristic_val -= pieceValue<piece>();
            }

            return heuristic_val;
        }

    }; // class MoveOrderer

} // namespace Dory::Search

#endif //DORY_MOVEORDERING_H
