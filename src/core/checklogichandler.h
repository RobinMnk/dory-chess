//
// Created by Robin on 29.06.2022.
//

#ifndef DORY_CHECKLOGICHANDLER_H
#define DORY_CHECKLOGICHANDLER_H

#include <memory>
#include "board.h"
#include "../utils/piecesteps.h"

namespace Dory {

    struct PinData {
        bool isDoubleCheck{false}, blockEP{false};
        BB attacked{0}, checkMask{0}, targetSquares{0}, pinsStr{0}, pinsDiag{0}, pawnAtk{0};

        [[nodiscard]] bool inCheck() const {
            return checkMask != FULL_BB;
        }
    };

    using PDptr = std::unique_ptr<PinData>;


    class CheckLogicHandler {
        template<bool, bool>
        static BB addPins(const Board &board, int kingSquare, bool &blockEP);

    public:
        template<bool>
        static void reload(const Board &board, const PDptr &pd);
    };

    template<bool whiteToMove, int dir>
    void handlePin(const Board &board, BB line, BB pieces, int kingSquare, BB &mask, bool &blockEP) {
        constexpr int dir_off = PieceSteps::directions[dir];
        BB sol = line & pieces;
        if (sol) {
            int ix;
            if constexpr (dir_off > 0) ix = firstBitOf(sol);
            else ix = lastBitOf(sol);

            BB kl = line & PieceSteps::FROM_TO[kingSquare][ix];
            if (bitCount(kl & board.enemyPieces<whiteToMove>()) ==
                1             // only enemyPieces piece on line is the slider
                && bitCount(kl & board.myPieces<whiteToMove>()) ==
                   1            // I only have one piece on line (excluding king)
                    )
                mask |= kl;

                // handle very special case of two sideways pinned epPawns
                // add pin line through two pawns to prevent pinned en passant
            else {
                if constexpr (dir == PieceSteps::DIR_LEFT || dir == PieceSteps::DIR_RIGHT) {
                    if (board.hasEnPassant()
                        && hasBitAt(epRank<whiteToMove>(), kingSquare)
                        && bitCount(kl & board.pawns<whiteToMove>()) == 1         // one own pawn
                        && bitCount(kl & board.enemyPawns<whiteToMove>()) == 1    // one enemy pawn
                        && bitCount(kl & board.occ()) == 3  // 2 pawns + 1 king = 3 total pieces on line
                            )
                        blockEP = true;
                }
            }
        }
    }


    template<bool whiteToMove, bool diag>
    BB CheckLogicHandler::addPins(const Board &board, int kingSquare, bool &blockEP) {
        std::array<BB, 8> kingLines = PieceSteps::LINES[kingSquare];
        BB pieces = board.enemySliders<whiteToMove, diag>();
        BB mask = 0;

        if constexpr (diag) {
            handlePin<whiteToMove, 1>(board, kingLines[1], pieces, kingSquare, mask, blockEP);
            handlePin<whiteToMove, 3>(board, kingLines[3], pieces, kingSquare, mask, blockEP);
            handlePin<whiteToMove, 5>(board, kingLines[5], pieces, kingSquare, mask, blockEP);
            handlePin<whiteToMove, 7>(board, kingLines[7], pieces, kingSquare, mask, blockEP);
        } else {
            handlePin<whiteToMove, 0>(board, kingLines[0], pieces, kingSquare, mask, blockEP);
            handlePin<whiteToMove, 2>(board, kingLines[2], pieces, kingSquare, mask, blockEP);
            handlePin<whiteToMove, 4>(board, kingLines[4], pieces, kingSquare, mask, blockEP);
            handlePin<whiteToMove, 6>(board, kingLines[6], pieces, kingSquare, mask, blockEP);
        }

        return mask;
    }

    template<bool whiteToMove>
    void CheckLogicHandler::reload(const Board &board, const PDptr &pd) {
        BB attacked{0}, checkMask{0}, mask, pawnAtk{0};
        int numChecks = 0;
        int kingSquare = board.kingSquare<whiteToMove>();

        BB pawnBB = board.enemyPawns<whiteToMove>();
        BB knightBB = board.enemyKnights<whiteToMove>();
        BB bishopBB = board.enemyBishops<whiteToMove>();
        BB rookBB = board.enemyRooks<whiteToMove>();
        BB queenBB = board.enemyQueens<whiteToMove>();

        BB myKing = board.king<whiteToMove>();

        // IS THE KING IN CHECK

        attacked |= PieceSteps::KING_MOVES[board.kingSquare<!whiteToMove>()];

        // Pawns
        mask = pawnAtkLeft<!whiteToMove>(pawnBB & pawnCanGoLeft<!whiteToMove>());     // pawn attack to the left
        attacked |= mask;
        pawnAtk |= mask;
        if (mask & myKing) {
            numChecks++;
            checkMask |= pawnInvAtkLeft<!whiteToMove>(myKing);
        }

        mask = pawnAtkRight<!whiteToMove>(pawnBB & pawnCanGoRight<!whiteToMove>());    // pawn attack to the right
        attacked |= mask;
        pawnAtk |= mask;
        if (mask & myKing) {
            numChecks++;
            checkMask |= pawnInvAtkRight<!whiteToMove>(myKing);
        }

        // Knights
        Bitloop(knightBB) {
            int ix = firstBitOf(knightBB);
            mask = PieceSteps::KNIGHT_MOVES[ix];
            attacked |= mask;
            if (mask & myKing) {
                numChecks++;
                setBit(checkMask, ix);
            }
        }

        // Bishops & (diagonal) Queens
        BB pieces = bishopBB | queenBB;
        Bitloop(pieces) {
            int ix = firstBitOf(pieces);
            mask = PieceSteps::slideMask<true>(board.occ() ^ myKing, ix);
            attacked |= mask;
            if (mask & myKing) {
                numChecks++;
                checkMask |= PieceSteps::FROM_TO[kingSquare][ix];
            }
        }

        // Rooks & (straight) Queens
        pieces = rookBB | queenBB;
        Bitloop(pieces) {
            int ix = firstBitOf(pieces);
            mask = PieceSteps::slideMask<false>(board.occ() ^ myKing, ix);
            attacked |= mask;
            if (mask & myKing) {
                numChecks++;
                checkMask |= PieceSteps::FROM_TO[kingSquare][ix];
            }
        }

        pd->isDoubleCheck = numChecks > 1;
        if (pd->isDoubleCheck) checkMask = 0;
        if (numChecks == 0) checkMask = FULL_BB;

        bool blockEP = false;
        pd->pinsDiag = addPins<whiteToMove, true>(board, kingSquare, blockEP);
        pd->pinsStr = addPins<whiteToMove, false>(board, kingSquare, blockEP);
        pd->targetSquares = board.enemyOrEmpty<whiteToMove>() & checkMask;
        pd->attacked = attacked;
        pd->checkMask = checkMask;
        pd->blockEP = blockEP;
        pd->pawnAtk = pawnAtk;
    }

} // namespace Dory

#endif //DORY_CHECKLOGICHANDLER_H
