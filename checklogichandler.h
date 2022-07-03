//
// Created by Robin on 29.06.2022.
//

#include "board.h"
#include "piecesteps.h"

#ifndef CHESSENGINE_CHECKLOGICHANDLER_H
#define CHESSENGINE_CHECKLOGICHANDLER_H

class CheckLogicHandler {
    int kingSquare{};

    template<Board board, State state, bool diag>
    BB addPins(){
        std::array<BB, 8> kingLines = PieceSteps::LINES[kingSquare];
        auto dirs = diag ? PieceSteps::diagonal : PieceSteps::straight;
        BB pieces = board.enemySliders<state.whiteToMove, diag>();
        BB mask = 0;

        for(int dir_id: dirs) {
            int dir_off = PieceSteps::directions[dir_id];
            BB line = kingLines[dir_id];
            BB sol = line & pieces;
            if(sol) {
                int ix = dir_off > 0 ? firstBitOf(sol) : lastBitOf(sol);
                BB kl = line & PieceSteps::FROM_TO[kingSquare][ix];
                if(
                    bitCount(kl & board.enemyPieces<state.whiteToMove>()) == 1             // only enemyPieces piece on line is the slider
                    && bitCount(kl & board.myPieces<state.whiteToMove>()) == 1             // I only have one piece on line (excluding king)
                ) mask |= kl;

                // add special pin line through two pawns to prevent pinned en passant
                else if(state.enPassantField
                        && (dir_id == PieceSteps::DIR_LEFT || dir_id == PieceSteps::DIR_RIGHT)
                        && rankOf(kingSquare) == epRankNr<state.whiteToMove>()
                        && bitCount(kl & board.pawns<state.whiteToMove>()) == 1         // one own pawn
                        && bitCount(kl & board.enemyPawns<state.whiteToMove>()) == 1    // one enemy pawn
                        && bitCount(kl & board.occ()) == 3  // 2 pawns + 1 king = 3 total pieces on line
                ) {
                    blockEP = true;
                }
            }
        }
        return mask;
    }

    [[nodiscard]] constexpr bool isAttacked(int index) const {
        return CHECK_BIT(attacked, index);
    }
public:
    bool isDoubleCheck{}, blockEP{};
    BB attacked{}, checkMask{}, pinsStraight{}, pinsDiagonal{};

    template<Board board, State state>
    void reload(){
        attacked = 0;
        checkMask = 0;
        pinsStraight = 0;
        pinsDiagonal = 0;
        blockEP = false;
        isDoubleCheck = false;
        kingSquare = board.kingSquare<state.whiteToMove>();

        BB pawnBB = board.enemyPawns<state.whiteToMove>();
        BB knightBB = board.enemyKnights<state.whiteToMove>();
        BB bishopBB = board.enemyBishops<state.whiteToMove>();
        BB rookBB = board.enemyRooks<state.whiteToMove>();
        BB queenBB = board.enemyQueens<state.whiteToMove>();
        BB kingBB = board.enemyKing<state.whiteToMove>();
        BB myKingBB = board.king<state.whiteToMove>();

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
                    atk = PieceSteps::PAWN_CAPTURES<!state.whiteToMove>[i];
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
                    atk = PieceSteps::slideMask<board, state.whiteToMove, true, true>(i);
                    if((atk & myKingBB) != 0) {
                        numChecks++;
                        checkMask |= PieceSteps::FROM_TO[kingSquare][i];
                    }
                    attacked |= atk;
                }
                // ROOKS & QUEENS
                if(((rookBB | queenBB) & selector) != 0) {
                    atk = PieceSteps::slideMask<board, state.whiteToMove, false, true>(i);
                    if((atk & myKingBB) != 0) {
                        numChecks++;
                        checkMask |= PieceSteps::FROM_TO[kingSquare][i];
                    }
                    attacked |= atk;
                }
            }

            selector <<= 1;
        }

        isDoubleCheck = numChecks > 1;
        if(isDoubleCheck) checkMask = 0;

        pinsDiagonal = addPins<board, state, true>();
        pinsStraight = addPins<board, state, false>();
    }

    [[nodiscard]] constexpr BB pruneEpPin(BB epPawns) const {
        return blockEP ? 0 : epPawns;
    }

    [[nodiscard]] constexpr bool isCheck() const {
        return isAttacked(kingSquare);
    }

    [[nodiscard]] constexpr BB getCheckMask() const {
        return isCheck() ? checkMask : FULL_BB;
    }

    [[nodiscard]] constexpr BB allPins() const {
        return pinsStraight | pinsDiagonal;
    }
};

#endif //CHESSENGINE_CHECKLOGICHANDLER_H
