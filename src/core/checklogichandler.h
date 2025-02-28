//
// Created by Robin on 29.06.2022.
//

#ifndef DORY_CHECKLOGICHANDLER_H
#define DORY_CHECKLOGICHANDLER_H

#include "board.h"
#include "piecesteps.h"

namespace Dory {

    struct PinData {
        BB attacked{0}, checkMask{0}, targetSquares{0}, pinsStr{0}, pinsDiag{0}, pawnAtk{0};
        bool blockEP{false};

        [[nodiscard]] bool inline inCheck() const {
            return checkMask != FULL_BB;
        }

        [[nodiscard]] bool inline inDoubleCheck() const {
            return checkMask == 0;
        }
    };

    class CheckLogicHandler {
        template<bool, bool>
        static inline BB addPins(const Board &board, int kingSquare, bool &blockEP);

        template<bool, int>
        static void handlePin(const Board &board, BB line, BB pieces, int kingSquare, BB &mask, bool &blockEP);

    public:
        CheckLogicHandler() = delete;

        template<bool>
        static void reload(const Board &board, PinData& pd);
    };

    template<bool whiteToMove, int dir>
    void CheckLogicHandler::handlePin(const Board &board, const BB line, const BB pieces, const int kingSquare, BB &mask, bool &blockEP) {
        constexpr int dir_off = PieceSteps::directions[dir];
        BB sol = line & pieces;
        if (sol) {
            int ix;
            if constexpr (dir_off > 0) ix = firstBitOf(sol);
            else ix = lastBitOf(sol);

            BB kl = line & PieceSteps::FROM_TO[kingSquare][ix];
            if (bitCount(kl & board.enemyPieces<whiteToMove>()) == 1             // only enemyPieces piece on line is the slider
                && bitCount(kl & board.myPieces<whiteToMove>()) == 1            // I only have one piece on line (excluding king)
            ) mask |= kl;

            // handle very special case of two sideways pinned epPawns
            // add pin line through two pawns to prevent pinned en passant
            else {
                if constexpr (dir == PieceSteps::DIR_LEFT || dir == PieceSteps::DIR_RIGHT) {
                    if (board.hasEnPassant()
                        && hasBitAt(epRank<whiteToMove>(), kingSquare)
                        && bitCount(kl & board.pawns<whiteToMove>()) == 1         // one own pawn
                        && bitCount(kl & board.enemyPawns<whiteToMove>()) == 1    // one enemy pawn
                        && bitCount(kl & board.occ()) == 3  // 2 pawns + 1 king = 3 total pieces on line
                    ) blockEP = true;
                }
            }
        }
    }


    template<bool whiteToMove, bool diag>
    inline BB CheckLogicHandler::addPins(const Board &board, int kingSquare, bool &blockEP) {
        const std::array<BB, 8>& kingLines = PieceSteps::LINES[kingSquare];
        const BB pieces = board.enemySliders<whiteToMove, diag>();
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
    void CheckLogicHandler::reload(const Board &board, PinData& pd) {
        int kingSquare = board.kingSquare<whiteToMove>();

        BB pawnBB = board.enemyPawns<whiteToMove>();
        BB knightBB = board.enemyKnights<whiteToMove>();
        BB bishopBB = board.enemyBishops<whiteToMove>();
        BB rookBB = board.enemyRooks<whiteToMove>();
        BB queenBB = board.enemyQueens<whiteToMove>();

        BB myKing = board.king<whiteToMove>();

        BB mask{0};
        pd.attacked = pd.pawnAtk = 0;

        // IS THE KING IN CHECK

        pd.attacked |= PieceSteps::KING_MOVES[board.kingSquare<!whiteToMove>()];

        pd.checkMask = FULL_BB;

        // Pawns
        mask = pawnAtkLeft<!whiteToMove>(pawnBB & pawnCanGoLeft<!whiteToMove>());     // pawn attack to the left
        pd.attacked |= mask;
        pd.pawnAtk |= mask;
        if(mask & myKing) pd.checkMask &= pawnInvAtkLeft<!whiteToMove>(myKing);

        // Can be made branch-less (but is slower):
//        flag = - static_cast<BB>((mask & myKing) != 0);  // flag becomes all 1’s if (mask & myKing) != 0, or 0 if (mask & myKing) == 0
//        pd.checkMask &= pawnInvAtkLeft<!whiteToMove>(myKing) | ~flag;


        mask = pawnAtkRight<!whiteToMove>(pawnBB & pawnCanGoRight<!whiteToMove>());    // pawn attack to the right
        pd.attacked |= mask;
        pd.pawnAtk |= mask;
        if(mask & myKing) pd.checkMask &= pawnInvAtkRight<!whiteToMove>(myKing);

        // Can be made branch-less (but is slower):
//        flag = - static_cast<BB>((mask & myKing) != 0);  // flag becomes all 1’s if (mask & myKing) != 0, or 0 if (mask & myKing) == 0
//        pd.checkMask &= pawnInvAtkRight<!whiteToMove>(myKing) | ~flag;


        // Knights
        Bitloop(knightBB) {
            int ix = firstBitOf(knightBB);
            mask = PieceSteps::KNIGHT_MOVES[ix];
            pd.attacked |= mask;
            if(mask & myKing) pd.checkMask &= newMask(ix);

            // Can be made branch-less (but is slower):
//            flag = - static_cast<BB>((mask & myKing) != 0);  // flag becomes all 1’s if (mask & myKing) != 0, or 0 if (mask & myKing) == 0
//            pd.checkMask &= newMask(ix) | ~flag;
        }

        // Sliders
        BB occ = board.occ() ^ myKing;

        // Bishops & (diagonal) Queens
        BB pieces = bishopBB | queenBB;
        Bitloop(pieces) {
            int ix = firstBitOf(pieces);
            mask = PieceSteps::slideMask<true>(occ, ix);
            pd.attacked |= mask;
            if(mask & myKing) pd.checkMask &= PieceSteps::FROM_TO[kingSquare][ix];

            // Can be made branch-less (but is slower):
//            flag = - static_cast<BB>((mask & myKing) != 0);  // flag becomes all 1’s if (mask & myKing) != 0, or 0 if (mask & myKing) == 0
//            pd.checkMask &= PieceSteps::FROM_TO[kingSquare][ix] | ~flag;
        }

        // Rooks & (straight) Queens
        pieces = rookBB | queenBB;
        Bitloop(pieces) {
            int ix = firstBitOf(pieces);
            mask = PieceSteps::slideMask<false>(occ, ix);
            pd.attacked |= mask;
            if(mask & myKing) pd.checkMask &= PieceSteps::FROM_TO[kingSquare][ix];

            // Can be made branch-less (but is slower):
//            flag = - static_cast<BB>((mask & myKing) != 0);  // flag becomes all 1’s if (mask & myKing) != 0, or 0 if (mask & myKing) == 0
//            pd.checkMask &= PieceSteps::FROM_TO[kingSquare][ix] | ~flag;
        }

        pd.blockEP = false;
        pd.pinsDiag = addPins<whiteToMove, true>(board, kingSquare, pd.blockEP);
        pd.pinsStr = addPins<whiteToMove, false>(board, kingSquare, pd.blockEP);
        pd.targetSquares = board.enemyOrEmpty<whiteToMove>() & pd.checkMask;
    }

} // namespace Dory

#endif //DORY_CHECKLOGICHANDLER_H
