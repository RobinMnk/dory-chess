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
};

class CheckLogicHandler {
    int kingSquare{};
    bool blockEP{};

    template<State state, bool diag>
    BB addPins(Board& board){
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

                // add special pin line through two pawns to prevent pinned en passant
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

    [[nodiscard]] constexpr BB pruneEpPin(BB epPawns) const {
        return blockEP ? 0 : epPawns;
    }

public:

    template<State state>
    PinData reload(Board& board){
        constexpr bool white = state.whiteToMove;
        BB attacked{0}, checkMask{0};
        blockEP = false;
        kingSquare = board.kingSquare<white>();

        BB pawnBB = board.enemyPawns<white>();
        BB knightBB = board.enemyKnights<white>();
        BB bishopBB = board.enemyBishops<white>();
        BB rookBB = board.enemyRooks<white>();
        BB queenBB = board.enemyQueens<white>();
        BB kingBB = board.enemyKing<white>();
        BB myKingBB = board.king<white>();

        BB selector = 1;
        BB mask, atk;
        int numChecks = 0;
        for(int i = 0; i < 64; i++) {
            // KINGS
            if((kingBB & selector) != 0) {
                attacked |= PieceSteps::KING_MOVES[i];
            } else {
                // PAWNS
                mask = pawnBB & selector;
                if(mask != 0) {
                    atk = PieceSteps::PAWN_CAPTURES<!white>[i];
                    if((atk & myKingBB) != 0) {
                        numChecks++;
                        checkMask |= mask;
                    }
                    attacked |= atk;
                }
                // KNIGHTS
                mask = knightBB & selector;
                if(mask != 0) {
                    atk = PieceSteps::KNIGHT_MOVES[i];
                    if((atk & myKingBB) != 0) {
                        numChecks++;
                        checkMask |= mask;
                    }
                    attacked |= atk;
                }
                // BISHOPS & QUEENS
                if(((bishopBB | queenBB) & selector) != 0) {
                    atk = PieceSteps::slideMask<white, true, true>(board, i);
                    if((atk & myKingBB) != 0) {
                        numChecks++;
                        checkMask |= PieceSteps::FROM_TO[kingSquare][i];
                    }
                    attacked |= atk;
                }
                // ROOKS & QUEENS
                if(((rookBB | queenBB) & selector) != 0) {
                    atk = PieceSteps::slideMask< white, false, true>(board, i);
                    if((atk & myKingBB) != 0) {
                        numChecks++;
                        checkMask |= PieceSteps::FROM_TO[kingSquare][i];
                    }
                    attacked |= atk;
                }
            }

            selector <<= 1;
        }

        bool isDoubleCheck = numChecks > 1;
        if(isDoubleCheck) checkMask = 0;

        BB pinsDiagonal = addPins<state, true>(board);
        BB pinsStraight = addPins<state, false>(board);
        if(numChecks == 0) checkMask = FULL_BB;
        BB targetSquares = board.enemyOrEmpty<state.whiteToMove>() & checkMask;

        return {isDoubleCheck, blockEP, attacked, checkMask, targetSquares, pinsStraight, pinsDiagonal };
    }
};

#endif //CHESSENGINE_CHECKLOGICHANDLER_H
