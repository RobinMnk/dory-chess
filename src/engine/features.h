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

    template<bool whiteToMove>
    int material(const Board& board, Params params) {
        return bitCount(board.pawns<whiteToMove>()) * params.MATERIAL_WEIGHT_PAWN +
                bitCount(board.knights<whiteToMove>()) * params.MATERIAL_WEIGHT_KNIGHT +
                bitCount(board.bishops<whiteToMove>()) * params.MATERIAL_WEIGHT_BISHOP +
                bitCount(board.rooks<whiteToMove>()) * params.MATERIAL_WEIGHT_ROOK +
                bitCount(board.queens<whiteToMove>()) * params.MATERIAL_WEIGHT_QUEEN;
    }


    int mobility(const Board& board, const State state, Params params) {
        int mobilityScore{0};
        MoveListGenerator::countLegalMoves(board, state);
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
