//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_PARAMS_H
#define DORY_ENGINE_PARAMS_H

#include "../chess.h"

namespace engine_params {

    struct EvaluationParams {
        const int  MATERIAL_WEIGHT_PAWN = 100;
        const int  MATERIAL_WEIGHT_KNIGHT = 300;
        const int  MATERIAL_WEIGHT_BISHOP = 325;
        const int  MATERIAL_WEIGHT_ROOK = 500;
        const int  MATERIAL_WEIGHT_QUEEN = 900;
        const int  MATERIAL_QUANTIFIER = 1;

        const int  MOBILITY_WEIGHT_PAWN = 1;
        const int  MOBILITY_WEIGHT_KNIGHT = 3;
        const int  MOBILITY_WEIGHT_BISHOP = 4;
        const int  MOBILITY_WEIGHT_ROOK = 4;
        const int  MOBILITY_WEIGHT_QUEEN = 7;
        const int  MOBILITY_WEIGHT_KING = -10;
        const int  MOBILITY_QUANTIFIER = 1;

        const int  ACTIVITY_QUANTIFIER = 1;

        EvaluationParams() {
            for(int pc = 0; pc < 6; pc++) {
                for (int sq = 0; sq < 64; sq++) {
                    mg_table[pc][sq] = mg_value[pc] + mg_table[pc][sq];
                    eg_table[pc][sq] = eg_value[pc] + eg_table[pc][sq];
                }
            }
        }

        int mg_value[6] = { 82, 337, 365, 477, 1025,  0};
        int eg_value[6] = { 94, 281, 297, 512,  936,  0};

        int* mg_table[6] =
                {
                        mg_pawn_table,
                        mg_knight_table,
                        mg_bishop_table,
                        mg_rook_table,
                        mg_queen_table,
                        mg_king_table
                };

        int* eg_table[6] =
                {
                        eg_pawn_table,
                        eg_knight_table,
                        eg_bishop_table,
                        eg_rook_table,
                        eg_queen_table,
                        eg_king_table
                };

        int mg_pawn_table[64] = {
                0,   0,   0,   0,   0,   0,   0,   0,
                50,  50,  50,  50,  50,  50,  50,  50,
                10,  10,  20,  30,  30,  20,  10,  10,
                5,   5,  10,  25,  25,  10,   5,   5,
                0,   0,   0,  20,  20,   0,   0,   0,
                5,  -5, -10,   0,   0, -10,  -5,   5,
                5,  10,  10, -20, -20,  10,  10,   5,
                0,   0,   0,   0,   0,   0,   0,   0
        };

        int eg_pawn_table[64] = {
                0,   0,   0,   0,   0,   0,   0,   0,
                80,  80,  80,  80,  80,  80,  80,  80,
                50,  50,  50,  50,  50,  50,  50,  50,
                30,  30,  30,  30,  30,  30,  30,  30,
                20,  20,  20,  20,  20,  20,  20,  20,
                10,  10,  10,  10,  10,  10,  10,  10,
                10,  10,  10,  10,  10,  10,  10,  10,
                0,   0,   0,   0,   0,   0,   0,   0
        };

        int mg_knight_table[64] = {
                -50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  0, 10, 15, 15, 10,  0,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  0, 15, 20, 20, 15,  0,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50,
        };

        int eg_knight_table[64] = {
                -50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  0, 10, 15, 15, 10,  0,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  0, 15, 20, 20, 15,  0,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50,
        };

        int mg_bishop_table[64] = {
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10,  5,  5, 10, 10,  5,  5,-10,
                -10,  0, 10, 10, 10, 10,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-10,-10,-20,
        };

        int eg_bishop_table[64] = {
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10,  5,  5, 10, 10,  5,  5,-10,
                -10,  0, 10, 10, 10, 10,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-10,-10,-20,
        };

        int mg_rook_table[64] = {
                0,  0,  0,  0,  0,  0,  0,  0,
                5, 10, 10, 10, 10, 10, 10,  5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                0,  0,  0,  5,  5,  0,  0,  0
        };

        int eg_rook_table[64] = {
                0,  0,  0,  0,  0,  0,  0,  0,
                5, 10, 10, 10, 10, 10, 10,  5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                0,  0,  0,  5,  5,  0,  0,  0
        };

        int mg_queen_table[64] = {
                -20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -5,   0,  5,  5,  5,  5,  0, -5,
                0,    0,  5,  5,  5,  5,  0, -5,
                -10,  5,  5,  5,  5,  5,  0,-10,
                -10,  0,  5,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20
        };

        int eg_queen_table[64] = {
                -20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -5,   0,  5,  5,  5,  5,  0, -5,
                0,    0,  5,  5,  5,  5,  0, -5,
                -10,  5,  5,  5,  5,  5,  0,-10,
                -10,  0,  5,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20
        };

        int mg_king_table2[64] = {
                -80, -70, -70, -70, -70, -70, -70, -80,
                -60, -60, -60, -60, -60, -60, -60, -60,
                -40, -50, -50, -60, -60, -50, -50, -40,
                -30, -40, -40, -50, -50, -40, -40, -30,
                -20, -30, -30, -40, -40, -30, -30, -20,
                -10, -20, -20, -20, -20, -20, -20, -10,
                20,  20,  -5,  -5,  -5,  -5,  20,  20,
                20,  30,  10,   0,   0,  10,  30,  20
        };
        int mg_king_table[64] = {
                -20, -10, -10, -10, -10, -10, -10, -20,
                -5,   0,   5,   5,   5,   5,   0,  -5,
                -10, -5,   20,  30,  30,  20,  -5, -10,
                -15, -10,  35,  45,  45,  35, -10, -15,
                -20, -15,  30,  40,  40,  30, -15, -20,
                -25, -20,  20,  25,  25,  20, -20, -25,
                -30, -25,   0,   0,   0,   0, -25, -30,
                -50, -30, -30, -30, -30, -30, -30, -50
        };

        int eg_king_table[64] = {
                -20, -10, -10, -10, -10, -10, -10, -20,
                -5,   0,   5,   5,   5,   5,   0,  -5,
                -10, -5,   20,  30,  30,  20,  -5, -10,
                -15, -10,  35,  45,  45,  35, -10, -15,
                -20, -15,  30,  40,  40,  30, -15, -20,
                -25, -20,  20,  25,  25,  20, -20, -25,
                -30, -25,   0,   0,   0,   0, -25, -30,
                -50, -30, -30, -30, -30, -30, -30, -50
        };


        template<bool whiteToMove>
        int adjustSquare(int square) {
            if constexpr (!whiteToMove) return square;
            else return square^56;
        }

        template<Piece_t piece, bool whiteToMove>
        int middleGamePieceTable(int square) {
            if constexpr (piece == Piece::Pawn)
                return mg_pawn_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Knight)
                return mg_knight_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Bishop)
                return mg_bishop_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Rook)
                return mg_rook_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Queen)
                return mg_queen_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::King)
                return mg_king_table[adjustSquare<whiteToMove>(square)];
            return 0;
        }

        template<Piece_t piece, bool whiteToMove>
        int endGamePieceTable(int square) {
            if constexpr (piece == Piece::Pawn)
                return eg_pawn_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Knight)
                return eg_knight_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Bishop)
                return eg_bishop_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Rook)
                return eg_rook_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::Queen)
                return eg_queen_table[adjustSquare<whiteToMove>(square)];
            else if constexpr (piece == Piece::King)
                return eg_king_table[adjustSquare<whiteToMove>(square)];
            return 0;
        }

        template<Piece_t piece>
        int gamePhaseIncrement() {
            if constexpr (piece == Piece::Knight || piece == Piece::Bishop)
                return 1;
            else if constexpr (piece == Piece::Rook)
                return 2;
            else if constexpr (piece == Piece::Queen)
                return 4;
            return 0;
        }
    };

    template<Piece_t piece>
    constexpr int pieceValue(EvaluationParams& params) {
        if constexpr (piece == Piece::Pawn) {
            return params.MATERIAL_WEIGHT_PAWN;
        }
        if constexpr (piece == Piece::Knight) {
            return params.MATERIAL_WEIGHT_KNIGHT;
        }
        if constexpr (piece == Piece::Bishop) {
            return params.MATERIAL_WEIGHT_BISHOP;
        }
        if constexpr (piece == Piece::Rook) {
            return params.MATERIAL_WEIGHT_ROOK;
        }
        if constexpr (piece == Piece::Queen) {
            return params.MATERIAL_WEIGHT_QUEEN;
        }
        return 0;
    }

}

#endif //DORY_ENGINE_PARAMS_H
