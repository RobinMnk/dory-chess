//
// Created by Robin on 24.08.2023.
//

#ifndef DORY_EVALUATION_H
#define DORY_EVALUATION_H

#include <iostream>

#include "../core/board.h"
#include "../core/piecesteps.h"
#include "engineparams.h"

namespace Dory::evaluation {

    template<Piece_t piece, bool whiteToMove>
    void addScoresForPiece(const Board& board, int& mgScore, int& egScore, int& gamePhase) {
        BB locations = board.getPieceBB<piece, whiteToMove>();
        Bitloop(locations) {
            mgScore += ENGINE_PARAMS.middleGamePieceTable<piece, whiteToMove>(firstBitOf(locations));
            egScore += ENGINE_PARAMS.endGamePieceTable<piece, whiteToMove>(firstBitOf(locations));
            gamePhase += ENGINE_PARAMS.gamePhaseIncrement<piece>();
        }
    }

    template<bool whiteToMove>
    int activity(const Board& board, int& gamePhase) {
        int mgScore{0}, egScore{0};

        addScoresForPiece<PIECE_Pawn, whiteToMove>(board, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Knight, whiteToMove>(board, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Bishop, whiteToMove>(board, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Rook, whiteToMove>(board, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Queen, whiteToMove>(board, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_King, whiteToMove>(board, mgScore, egScore, gamePhase);

        /* tapered eval */
        if (gamePhase > 24) gamePhase = 24; /* in case of early promotion */
        int egPhase = 24 - gamePhase;
        return (mgScore * gamePhase + egScore * egPhase) / 24;
    }

    template<bool whiteToMove>
    int material(const Board& board) {
        return bitCount(board.pawns<whiteToMove>()) * ENGINE_PARAMS.MATERIAL_WEIGHT_PAWN +
               bitCount(board.knights<whiteToMove>()) * ENGINE_PARAMS.MATERIAL_WEIGHT_KNIGHT +
               bitCount(board.bishops<whiteToMove>()) * ENGINE_PARAMS.MATERIAL_WEIGHT_BISHOP +
               bitCount(board.rooks<whiteToMove>()) * ENGINE_PARAMS.MATERIAL_WEIGHT_ROOK +
               bitCount(board.queens<whiteToMove>()) * ENGINE_PARAMS.MATERIAL_WEIGHT_QUEEN;
    }

    template<bool whiteToMove>
    int passedPawns(const Board& board) {
        int score{0};
        BB pawns = board.pawns<whiteToMove>();
        Bitloop(pawns) {
            int ix = firstBitOf(pawns);
            if((PieceSteps::PASSED_PAWN_MASK<whiteToMove>[ix] & board.enemyPawns<whiteToMove>()) == 0) {
                score += ENGINE_PARAMS.passedPawnScore<whiteToMove>(ix);
            }
        }
        return score;
    }


    /**
     * Gives estimate for position evaluation score for the side that is to move.
     * Positive value is good for the side to move (not necessarily good for white)
     */
    template<bool whiteToMove>
    int evaluatePosition(const Board& board) {
//        int matFriendly = material<whiteToMove>(board);
//        int matEnemy = material<!whiteToMove>(board);
        int gamePhase = 0;

        // includes material!
        int activityScore = activity<whiteToMove>(board, gamePhase) - activity<!whiteToMove>(board, gamePhase);

        int passedPawnsScore = passedPawns<whiteToMove>(board) - passedPawns<!whiteToMove>(board);

        int evalEstimate = activityScore + passedPawnsScore;

        return evalEstimate;
    }

} // namespace Dory::evaluation

#endif //DORY_EVALUATION_H
