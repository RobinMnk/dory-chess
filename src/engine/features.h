//
// Created by robin on 24.08.2023.
//

#ifndef DORY_FEATURES_H
#define DORY_FEATURES_H

#include "../board.h"
#include "engine_params.h"
#include "../movegen.h"

using Params = engine_params::EvaluationParams;

namespace features {


    template<Piece_t piece, bool whiteToMove>
    void addScoresForPiece(const Board& board, Params params, int& mgScore, int& egScore, int& gamePhase) {
        BB locations = board.getPieceBB<piece, whiteToMove>();
        Bitloop(locations) {
            mgScore += params.middleGamePieceTable<piece, whiteToMove>(firstBitOf(locations));
//            egScore += params.endGamePieceTable<piece, whiteToMove>(firstBitOf(locations));
//            gamePhase += params.gamePhaseIncrement<piece>();
        }
    }

    template<bool whiteToMove>
    int activity(const Board& board, Params params) {
        int mgScore{0}, egScore{0}, gamePhase{0};

        addScoresForPiece<Piece::Pawn, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<Piece::Knight, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<Piece::Bishop, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<Piece::Rook, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<Piece::Queen, whiteToMove>(board, params, mgScore, egScore, gamePhase);
        addScoresForPiece<Piece::King, whiteToMove>(board, params, mgScore, egScore, gamePhase);

        return mgScore;
//        int mgPhase = gamePhase;
//        if (mgPhase > 24) mgPhase = 24; /* in case of early promotion */
//        int egPhase = 24 - mgPhase;
//        return ((mgScore * mgPhase + egScore * egPhase) / 24) / bitCount(board.myPieces<whiteToMove>());
    }


    template<bool whiteToMove>
    int material(const Board& board, Params params) {
        return bitCount(board.pawns<whiteToMove>()) * params.MATERIAL_WEIGHT_PAWN +
                bitCount(board.knights<whiteToMove>()) * params.MATERIAL_WEIGHT_KNIGHT +
                bitCount(board.bishops<whiteToMove>()) * params.MATERIAL_WEIGHT_BISHOP +
                bitCount(board.rooks<whiteToMove>()) * params.MATERIAL_WEIGHT_ROOK +
                bitCount(board.queens<whiteToMove>()) * params.MATERIAL_WEIGHT_QUEEN;
    }


    template<bool whiteToMove>
    int mobility(const Board& board, Params params) {
        // Ignoring castling for simplicity
        State state(whiteToMove, false, false, false, false);
        MoveListGenerator::countLegalMoves(board, state);

        int mobilityScore{0};
        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Pawn) * params.MOBILITY_WEIGHT_PAWN;
        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Knight) * params.MOBILITY_WEIGHT_KNIGHT;
        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Bishop) * params.MOBILITY_WEIGHT_BISHOP;
        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Rook) * params.MOBILITY_WEIGHT_ROOK;
        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Queen) * params.MOBILITY_WEIGHT_QUEEN;
        mobilityScore += MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::King) * params.MOBILITY_WEIGHT_KING;
        return mobilityScore;
    }
}

#endif //DORY_FEATURES_H
