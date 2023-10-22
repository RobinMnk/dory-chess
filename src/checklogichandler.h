//
// Created by Robin on 29.06.2022.
//

#include "board.h"
#include "piecesteps.h"

#ifndef DORY_CHECKLOGICHANDLER_H
#define DORY_CHECKLOGICHANDLER_H

struct PinData;

using PDptr = std::unique_ptr<PinData>;

struct PinData {
    bool isDoubleCheck{false}, blockEP{false};
    BB attacked{0}, checkMask{0}, targetSquares{0}, pinsStr{0}, pinsDiag{0};

    [[nodiscard]] bool inCheck() const {
        return checkMask != FULL_BB;
    }
};

class CheckLogicHandler {
    template<bool, bool>
    static BB addPins(const Board& board, int kingSquare, bool& blockEP);

public:
    template<bool>
    static void reload(const Board& board, const PDptr& pd);
};

template<bool whiteToMove, int dir>
void handlePin(const Board& board, BB line, BB pieces, int kingSquare, BB& mask, bool& blockEP) {
    constexpr int dir_off = PieceSteps::directions[dir];
    BB sol = line & pieces;
    if(sol) {
        int ix;
        if constexpr (dir_off > 0) ix = firstBitOf(sol);
        else ix = lastBitOf(sol);

        BB kl = line & PieceSteps::FROM_TO[kingSquare][ix];
        if(bitCount(kl & board.enemyPieces<whiteToMove>()) == 1             // only enemyPieces piece on line is the slider
            && bitCount(kl & board.myPieces<whiteToMove>()) == 1            // I only have one piece on line (excluding king)
        ) mask |= kl;

        // handle very special case of two sideways pinned epPawns
        // add pin line through two pawns to prevent pinned en passant
        else if(board.hasEnPassant()
                && (dir == PieceSteps::DIR_LEFT || dir == PieceSteps::DIR_RIGHT)
                && rankOf(kingSquare) == epRankNr<whiteToMove>()
                && bitCount(kl & board.pawns<whiteToMove>()) == 1         // one own pawn
                && bitCount(kl & board.enemyPawns<whiteToMove>()) == 1    // one enemy pawn
                && bitCount(kl & board.occ()) == 3  // 2 pawns + 1 king = 3 total pieces on line
                ) blockEP = true;
    }
}


template<bool whiteToMove, bool diag>
BB CheckLogicHandler::addPins(const Board& board, int kingSquare, bool& blockEP){
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

template<bool white>
void CheckLogicHandler::reload(const Board& board, const PDptr& pd){
    BB attacked{0}, checkMask{0}, mask;
    int numChecks = 0;
    int kingSquare = board.kingSquare<white>();

    BB pawnBB = board.enemyPawns<white>();
    BB knightBB = board.enemyKnights<white>();
    BB bishopBB = board.enemyBishops<white>();
    BB rookBB = board.enemyRooks<white>();
    BB queenBB = board.enemyQueens<white>();

    BB myKing = board.king<white>();

    // IS THE KING IN CHECK

    attacked |= PieceSteps::KING_MOVES[board.kingSquare<!white>()];

    // Pawns
    mask = pawnAtkLeft<!white>(pawnBB & pawnCanGoLeft<!white>());     // pawn attack to the left
    attacked |= mask;
    if(mask & myKing) {
        numChecks++;
        checkMask |= pawnInvAtkLeft<!white>(myKing);
    }

    mask = pawnAtkRight<!white>(pawnBB & pawnCanGoRight<!white>());    // pawn attack to the right
    attacked |= mask;
    if(mask & myKing) {
        numChecks++;
        checkMask |= pawnInvAtkRight<!white>(myKing);
    }

    // Knights
    Bitloop(knightBB) {
        int ix = firstBitOf(knightBB);
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
        int ix = firstBitOf(pieces);
        mask = PieceSteps::slideMask<true>(board.occ() ^ myKing, ix);
        attacked |= mask;
        if(mask & myKing) {
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
        if(mask & myKing) {
            numChecks++;
            checkMask |= PieceSteps::FROM_TO[kingSquare][ix];
        }
    }

    pd->isDoubleCheck = numChecks > 1;
    if(pd->isDoubleCheck) checkMask = 0;
    if(numChecks == 0) checkMask = FULL_BB;

    pd->pinsDiag = addPins<white, true>(board, kingSquare, pd->blockEP);
    pd->pinsStr  = addPins<white, false>(board, kingSquare, pd->blockEP);
    pd->targetSquares = board.enemyOrEmpty<white>() & checkMask;
    pd->attacked = attacked;
    pd->checkMask = checkMask;
}

#endif //DORY_CHECKLOGICHANDLER_H
