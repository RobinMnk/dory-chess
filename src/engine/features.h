//
// Created by Robin on 24.08.2023.
//

#ifndef DORY_FEATURES_H
#define DORY_FEATURES_H

#include "engine_params.h"
//#include "../core/movegen.h"

namespace Dory::features {
    using Params = engine_params::EvaluationParams;

    template<Piece_t piece, bool whiteToMove>
    void addScoresForPiece(const Board& board, Params& params, int& mgScore, int& egScore, int& gamePhase) {
        BB locations = board.getPieceBB<piece, whiteToMove>();
        Bitloop(locations) {
            mgScore += params.middleGamePieceTable<piece, whiteToMove>(firstBitOf(locations));
            egScore += params.endGamePieceTable<piece, whiteToMove>(firstBitOf(locations));
            gamePhase += params.gamePhaseIncrement<piece>();
        }
//        BB locationsEnemy = board.getPieceBB<piece, !whiteToMove>();
//        Bitloop(locationsEnemy) {
//            mgScore -= params.middleGamePieceTable<piece, !whiteToMove>(firstBitOf(locationsEnemy));
//            egScore -= params.endGamePieceTable<piece, !whiteToMove>(firstBitOf(locationsEnemy));
//            gamePhase += params.gamePhaseIncrement<piece>();
//        }
    }

    template<bool whiteToMove>
    int activity(const Board& board, Params& params) {
        int mgScore{0}, egScore{0}, gamePhase{0};

        addScoresForPiece<PIECE_Pawn, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Knight, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Bishop, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Rook, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_Queen, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<PIECE_King, whiteToMove>(board, params, mgScore, egScore, gamePhase);

        /* tapered eval */
        if (gamePhase > 24) gamePhase = 24; /* in case of early promotion */
        int egPhase = 24 - gamePhase;
        return (mgScore * gamePhase + egScore * egPhase) / 24;
    }


    template<bool whiteToMove>
    int material(const Board& board, Params& params) {
        return bitCount(board.pawns<whiteToMove>()) * params.MATERIAL_WEIGHT_PAWN +
                bitCount(board.knights<whiteToMove>()) * params.MATERIAL_WEIGHT_KNIGHT +
                bitCount(board.bishops<whiteToMove>()) * params.MATERIAL_WEIGHT_BISHOP +
                bitCount(board.rooks<whiteToMove>()) * params.MATERIAL_WEIGHT_ROOK +
                bitCount(board.queens<whiteToMove>()) * params.MATERIAL_WEIGHT_QUEEN;
    }


    template<bool whiteToMove>
    int passedPawns(const Board& board, const Params& params, int oppMaterial) {
        int score{0};
        BB pawns = board.pawns<whiteToMove>();
        Bitloop(pawns) {
            int ix = firstBitOf(pawns);
            if((PieceSteps::PASSED_PAWN_MASK<whiteToMove>[ix] & board.enemyPawns<whiteToMove>()) == 0) {
                score += params.passedPawnScore<whiteToMove>(ix);
            }
        }
        return static_cast<int>(score * (1 - (static_cast<float>(4 * oppMaterial) / params.initialWeight)));
    }



//    template<bool whiteToMove>
//    int mobility(const Board& board, Params& params) {
//        // Ignoring castling for simplicity
//        State state(whiteToMove, false, false, false, false);
//        MoveListGenerator::countLegalMoves(board, state);
//
//        unsigned int mobilityScore{0};
//        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(PIECE_Pawn) * params.MOBILITY_WEIGHT_PAWN;
//        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(PIECE_Knight) * params.MOBILITY_WEIGHT_KNIGHT;
//        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(PIECE_Bishop) * params.MOBILITY_WEIGHT_BISHOP;
//        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(PIECE_Rook) * params.MOBILITY_WEIGHT_ROOK;
//        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(PIECE_Queen) * params.MOBILITY_WEIGHT_QUEEN;
//        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(PIECE_King) * params.MOBILITY_WEIGHT_KING;
//        return mobilityScore;
//    }
}

#endif //DORY_FEATURES_H
