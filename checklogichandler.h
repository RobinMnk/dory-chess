//
// Created by Robin on 29.06.2022.
//

#include "board.h"
#include "piecesteps.h"

#ifndef CHESSENGINE_CHECKLOGICHANDLER_H
#define CHESSENGINE_CHECKLOGICHANDLER_H

struct PinData {
    bool isDoubleCheck{false}, blockEP{false};
    BB attacked{0}, checkMask{0}, targetSquares{0}, pinsStr{0}, pinsDiag{0};

    [[nodiscard]] bool isCheck() const { return checkMask != FULL_BB; }
};

class CheckLogicHandler {
    template<State state, bool diag>
    static BB addPins(const Board& board, int kingSquare, bool& blockEP){
        constexpr bool white = state.whiteToMove;
        std::array<BB, 8> kingLines = PieceSteps::LINES[kingSquare];
        auto dirs = diag ? PieceSteps::diagonal : PieceSteps::straight;
        BB pieces = board.enemySliders<white, diag>();
        BB mask = 0;

        for(int dir_id: dirs) {
            int dir_off = PieceSteps::directions[dir_id];
            BB line = kingLines[dir_id];
            BB sol = line & pieces;
            if(sol) {
                int ix = dir_off > 0 ? firstBitOf(sol) : lastBitOf(sol);
                BB kl = line & PieceSteps::FROM_TO[kingSquare][ix];
                if(
                    bitCount(kl & board.enemyPieces<white>()) == 1             // only enemyPieces piece on line is the slider
                    && bitCount(kl & board.myPieces<white>()) == 1             // I only have one piece on line (excluding king)
                ) mask |= kl;

                // handle very special case of two sideways pinned epPawns
                // add pin line through two pawns to prevent pinned en passant
                else if(board.enPassantField
                        && (dir_id == PieceSteps::DIR_LEFT || dir_id == PieceSteps::DIR_RIGHT)
                        && rankOf(kingSquare) == epRankNr<white>()
                        && bitCount(kl & board.pawns<white>()) == 1         // one own pawn
                        && bitCount(kl & board.enemyPawns<white>()) == 1    // one enemy pawn
                        && bitCount(kl & board.occ()) == 3  // 2 pawns + 1 king = 3 total pieces on line
                ) {
                    blockEP = true;
                }
            }
        }
        return mask;
    }

public:
    template<State state>
    static PinData reload(Board& board){
        constexpr bool white = state.whiteToMove;
        BB attacked{0}, checkMask{0}, mask;
        int numChecks = 0;
        bool blockEP = false;
        int kingSquare = board.kingSquare<white>();

        BB pawnBB = board.enemyPawns<white>();
        BB knightBB = board.enemyKnights<white>();
        BB bishopBB = board.enemyBishops<white>();
        BB rookBB = board.enemyRooks<white>();
        BB queenBB = board.enemyQueens<white>();
        BB kingBB = board.enemyKing<white>();

        BB myKing = board.king<white>();

        // IS THE KING IN CHECK

        attacked |= PieceSteps::KING_MOVES[lastBitOf(kingBB)];

        // Pawns
        mask = pawnAtkLeft<!white>(pawnBB);     // pawn attack to the left
        attacked |= mask;
        if(mask & myKing) {
            numChecks++;
            checkMask |= pawnInvAtkLeft<!white>(myKing);
        }

        mask = pawnAtkRight<!white>(pawnBB);    // pawn attack to the right
        attacked |= mask;
        if(mask & myKing) {
            numChecks++;
            checkMask |= pawnInvAtkRight<!white>(myKing);
        }

        // Knights
        Bitloop(knightBB) {
            int ix = lastBitOf(knightBB);
            mask = PieceSteps::KNIGHT_MOVES[ix];
            attacked |= mask;
            if(mask & myKing) {
                numChecks++;
                setBit(checkMask, ix);
            }
        }

        // Bishops & (diagonal) Queens
        BB pieces = bishopBB | queenBB;
        Bitloop(pieces) {
            int ix = lastBitOf(pieces);
            mask = PieceSteps::slideMask<white, true, true>(board, ix);
            attacked |= mask;
            if(mask & myKing) {
                numChecks++;
                checkMask |= PieceSteps::FROM_TO[kingSquare][ix];
            }
        }

        // Rooks & (straight) Queens
        pieces = rookBB | queenBB;
        Bitloop(pieces) {
            int ix = lastBitOf(pieces);
            mask = PieceSteps::slideMask<white, false, true>(board, ix);
            attacked |= mask;
            if(mask & myKing) {
                numChecks++;
                checkMask |= PieceSteps::FROM_TO[kingSquare][ix];
            }
        }

        bool isDoubleCheck = numChecks > 1;
        if(isDoubleCheck) checkMask = 0;

        BB pinsDiagonal = addPins<state, true>(board, kingSquare, blockEP);
        BB pinsStraight = addPins<state, false>(board, kingSquare, blockEP);
        if(numChecks == 0) checkMask = FULL_BB;
        BB targetSquares = board.enemyOrEmpty<state.whiteToMove>() & checkMask;

        return { isDoubleCheck, blockEP, attacked, checkMask, targetSquares, pinsStraight, pinsDiagonal };
    }
};

#endif //CHESSENGINE_CHECKLOGICHANDLER_H
