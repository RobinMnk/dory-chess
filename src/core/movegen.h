//
// Created by Robin on 01.07.2022.
//

#include "checklogichandler.h"

#ifndef DORY_MOVEGEN_H
#define DORY_MOVEGEN_H

template<typename MC, bool whiteToMove, Piece_t piece, Flag_t flags>
concept ValidMoveCollector =
    requires(MC mc, const Board& board, BB from, BB to) {
        MC::template registerMove<whiteToMove, piece, flags>(board, from, to);
    };

template<typename MC, bool=false, bool=false>
class MoveGenerator {
public:
    static PDptr pd;
    static std::array<unsigned int, 6> numberOfMovesByPiece;
    static unsigned long numberOfMoves;

    template<bool, bool=true>
    static void generate(const Board& board);

private:
    template<bool whiteToMove, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    requires ValidMoveCollector<MC, whiteToMove, piece, flags>
    static void generateSuccessorBoard(const Board& board, BB from, BB to);

    // - - - - - - Helper Functions - - - - - -

    template<bool, Piece_t, Flag_t = MoveFlag::Silent>
    static void addToList(const Board& board, int fromIndex, BB targets);

    template<bool>
    static void handlePromotions(const Board& board, BB from, BB to);

    // - - - - - - Individual Piece Moves - - - - - -

    template<bool>
    static void pawnMoves(const Board& board);

    template<bool>
    static void knightMoves(const Board& board);

    template<bool>
    static void bishopMoves(const Board& board);

    template<bool>
    static void rookMoves(const Board& board);

    template<bool>
    static void queenMoves(const Board& board);

    template<bool>
    static void kingMoves(const Board& board);

    template<bool>
    static void castles(const Board& board);
};

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove, bool reloadClh>
void MoveGenerator<MoveCollector, quiescence, countOnly>::generate(const Board& board) {
    if constexpr (reloadClh)
        CheckLogicHandler::reload<whiteToMove>(board, pd);

    if constexpr (countOnly) {
//        numberOfMovesByPiece.fill(0);
        numberOfMoves = 0;
    }

    if(!pd->isDoubleCheck) {
        pawnMoves<whiteToMove>(board);
        knightMoves<whiteToMove>(board);
        bishopMoves<whiteToMove>(board);
        rookMoves<whiteToMove>(board);
        queenMoves<whiteToMove>(board);

        if (board.canCastle<whiteToMove>())
            castles<whiteToMove>(board);
    }

    kingMoves<whiteToMove>(board);
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove, Piece_t piece, Flag_t flags>
requires ValidMoveCollector<MoveCollector, whiteToMove, piece, flags>
void MoveGenerator<MoveCollector, quiescence, countOnly>::generateSuccessorBoard(const Board& board, BB from, BB to) {
    if constexpr (quiescence) {
        if ((to & board.enemyPieces<whiteToMove>()) == 0)
            return;
    }

    if constexpr (countOnly) {
//        numberOfMovesByPiece.at(piece)++;
        numberOfMoves++;
        return;
    }

    MoveCollector::template registerMove<whiteToMove, piece, flags>(board, from, to);
}

// - - - - - - Helper Functions - - - - - -

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove, Piece_t piece, Flag_t flags>
void MoveGenerator<MoveCollector, quiescence, countOnly>::addToList(const Board& board, int fromIndex, BB targets) {
    BB fromBB = newMask(fromIndex);
    Bitloop(targets) {
        BB toBB = isolateLowestBit(targets);
        generateSuccessorBoard<whiteToMove, piece, flags>(board, fromBB, toBB);
    }
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::handlePromotions(const Board& board, BB from, BB to) {
    generateSuccessorBoard<whiteToMove, Piece::Pawn, MoveFlag::PromoteQueen>(board, from, to);
    generateSuccessorBoard<whiteToMove, Piece::Pawn, MoveFlag::PromoteRook>(board, from, to);
    generateSuccessorBoard<whiteToMove, Piece::Pawn, MoveFlag::PromoteBishop>(board, from, to);
    generateSuccessorBoard<whiteToMove, Piece::Pawn, MoveFlag::PromoteKnight>(board, from, to);
}

// - - - - - - Individual Piece Moves - - - - - -

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::pawnMoves(const Board& board) {
    constexpr bool white = whiteToMove;
    BB free = board.free();
    BB enemy = board.enemyPieces<white>();
    BB pawnsFwd = board.pawns<white>() & ~pd->pinsDiag;
    BB pawnCapt = board.pawns<white>() & ~pd->pinsStr;

    // pawns that can move 1 or 2 squares
    BB pwnMov = pawnsFwd & backward<white>(free);
    BB pwnMov2 = pwnMov & backward2<white>(free & pd->checkMask) & firstRank<white>();
    pwnMov &= backward<white>(pd->checkMask);

    // pawns that can capture Left or Right
    BB pawnCapL = pawnCapt & pawnInvAtkLeft<white>(enemy & pd->checkMask) & pawnCanGoLeft<white>();
    BB pawnCapR = pawnCapt & pawnInvAtkRight<white>(enemy & pd->checkMask) & pawnCanGoRight<white>();

    // remove pinned pawns
    pwnMov      &= backward<white> (pd->pinsStr) | ~pd->pinsStr;
    pwnMov2     &= backward2<white>(pd->pinsStr) | ~pd->pinsStr;
    pawnCapL    &= pawnInvAtkLeft<white> (pd->pinsDiag & pawnCanGoRight<white>()) | ~pd->pinsDiag;
    pawnCapR    &= pawnInvAtkRight<white>(pd->pinsDiag & pawnCanGoLeft <white>()) | ~pd->pinsDiag;

    // handle en passant pawns
    BB epPawnL{0}, epPawnR{0};
    if(board.hasEnPassant() != 0 && !pd->blockEP) {
        BB enPassant = board.enPassantBB();
        // left capture is ep square and is on checkmask
        epPawnL = pawnCapt & pawnCanGoLeft<white>() & pawnInvAtkLeft<white>(enPassant & forward<white>(pd->checkMask));
        // remove pinned ep pawns
        epPawnL &= pawnInvAtkLeft<white>(pd->pinsDiag & pawnCanGoLeft<white>()) | ~pd->pinsDiag;

        // right capture is ep square and is on checkmask
        epPawnR = pawnCapt & pawnCanGoRight<white>() & pawnInvAtkRight<white>(enPassant & forward<white>(pd->checkMask));
        // remove pinned ep pawns
        epPawnR &= pawnInvAtkRight<white>(pd->pinsDiag & pawnCanGoRight<white>()) | ~pd->pinsDiag;
    }

    // collect all promoting pawns in separate variables
    BB lastRowMask      = pawnOnLastRow<white>();
    BB pwnPromoteFwd    = pwnMov   & lastRowMask;
    BB pwnPromoteL      = pawnCapL & lastRowMask;
    BB pwnPromoteR      = pawnCapR & lastRowMask;

    // remove all promoting pawns from these collections
    pwnMov &= ~lastRowMask;
    pawnCapL &= ~lastRowMask;
    pawnCapR &= ~lastRowMask;

    BB from;
    // non-promoting pawn moves
    Bitloop(pwnMov) {   // straight push, 1 square
        from = isolateLowestBit(pwnMov);
        generateSuccessorBoard<whiteToMove, Piece::Pawn>(board, from, forward<white>(from));
    }
    Bitloop(pawnCapL) { // capture towards left
        from = isolateLowestBit(pawnCapL);
        generateSuccessorBoard<whiteToMove, Piece::Pawn>(board, from, pawnAtkLeft<white>(from));
    }
    Bitloop(pawnCapR) { // capture towards right
        from = isolateLowestBit(pawnCapR);
        generateSuccessorBoard<whiteToMove, Piece::Pawn>(board, from, pawnAtkRight<white>(from));
    }

    // promoting pawn moves
    Bitloop(pwnPromoteFwd) {    // single push + promotion
        from = isolateLowestBit(pwnPromoteFwd);
        handlePromotions<whiteToMove>(board, from, forward<white>(from));
    }
    Bitloop(pwnPromoteL) {    // capture left + promotion
        from = isolateLowestBit(pwnPromoteL);
        handlePromotions<whiteToMove>(board, from, pawnAtkLeft<white>(from));
    }
    Bitloop(pwnPromoteR) {    // capture right + promotion
        from = isolateLowestBit(pwnPromoteR);
        handlePromotions<whiteToMove>(board, from, pawnAtkRight<white>(from));
    }

    // pawn moves that cannot be promotions
    Bitloop(pwnMov2) {    // pawn double push
        from = isolateLowestBit(pwnMov2);
        generateSuccessorBoard<whiteToMove, Piece::Pawn, MoveFlag::PawnDoublePush>(board, from, forward2<white>(from));
    }
    Bitloop(epPawnL) {    // pawn double push
        from = isolateLowestBit(epPawnL);
        generateSuccessorBoard<whiteToMove, Piece::Pawn, MoveFlag::EnPassantCapture>(board, from, pawnAtkLeft<white>(from));
    }
    Bitloop(epPawnR) {    // pawn double push
        from = isolateLowestBit(epPawnR);
        generateSuccessorBoard<whiteToMove, Piece::Pawn, MoveFlag::EnPassantCapture>(board, from, pawnAtkRight<white>(from));
    }
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::knightMoves(const Board& board) {
    BB allPins = pd->pinsStr | pd->pinsDiag;
    BB movKnights = board.knights<whiteToMove>() & ~allPins;

    Bitloop(movKnights) {
        int ix = firstBitOf(movKnights);
        BB targets = PieceSteps::KNIGHT_MOVES[ix] & pd->targetSquares;
        addToList<whiteToMove, Piece::Knight>(board, ix, targets);
    }
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::bishopMoves(const Board& board) {
    BB bishops = board.bishops<whiteToMove>() & ~pd->pinsStr;

    Bitloop(bishops) {
        int ix = firstBitOf(bishops);
        BB targets = PieceSteps::slideMask<true>(board.occ(), ix) & pd->targetSquares;
        if(hasBitAt(pd->pinsDiag, ix)) targets &= pd->pinsDiag;
        addToList<whiteToMove, Piece::Bishop>(board, ix, targets);
    }
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::rookMoves(const Board& board) {
    BB rooks = board.rooks<whiteToMove>() & ~pd->pinsDiag;

    Bitloop(rooks) {
        int ix = firstBitOf(rooks);

        BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd->targetSquares;
        if(hasBitAt(pd->pinsStr, ix)) targets &= pd->pinsStr;

        if (board.canCastleShort<whiteToMove>() && hasBitAt(startingKingsideRook<whiteToMove>(), ix)) {
            addToList<whiteToMove, Piece::Rook, MoveFlag::RemoveShortCastling>(board, ix, targets);
            continue;
        }
        if (board.canCastleLong<whiteToMove>() && hasBitAt(startingQueensideRook<whiteToMove>(), ix)) {
            addToList<whiteToMove, Piece::Rook, MoveFlag::RemoveLongCastling>(board, ix, targets);
            continue;
        }

        addToList<whiteToMove, Piece::Rook>(board, ix, targets);
    }
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::queenMoves(const Board& board) {
    BB queens = board.queens<whiteToMove>();
    BB queensPinStr = queens & pd->pinsStr & ~pd->pinsDiag;
    BB queensPinDiag = queens & pd->pinsDiag & ~pd->pinsStr;
    BB queensNoPin = queens & ~(pd->pinsDiag | pd->pinsStr);

    Bitloop(queensPinStr) {
        int ix = firstBitOf(queensPinStr);
        BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd->targetSquares & pd->pinsStr;
        addToList<whiteToMove, Piece::Queen>(board, ix, targets);
    }

    Bitloop(queensPinDiag) {
        int ix = firstBitOf(queensPinDiag);
        BB targets = PieceSteps::slideMask<true>(board.occ(), ix) & pd->targetSquares & pd->pinsDiag;
        addToList<whiteToMove, Piece::Queen>(board, ix, targets);
    }

    Bitloop(queensNoPin) {
        int ix = firstBitOf(queensNoPin);
        BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd->targetSquares;
        targets |= PieceSteps::slideMask<true>(board.occ(), ix) & pd->targetSquares;
        addToList<whiteToMove, Piece::Queen>(board, ix, targets);
    }
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::kingMoves(const Board& board) {
    int ix = board.kingSquare<whiteToMove>();
    BB targets = PieceSteps::KING_MOVES[ix] & ~pd->attacked & board.enemyOrEmpty<whiteToMove>();
    addToList<whiteToMove, Piece::King, MoveFlag::RemoveAllCastling>(board, ix, targets);
}

template<typename MoveCollector, bool quiescence, bool countOnly>
template<bool whiteToMove>
void MoveGenerator<MoveCollector, quiescence, countOnly>::castles(const Board& board) {
    constexpr bool white = whiteToMove;
    constexpr BB startKing = STARTBOARD.king<white>();
    constexpr BB csMask = castleShortMask<white>();
    constexpr BB clMask = castleLongMask<white>();
    BB kingBB = board.king<white>();

    if (board.canCastleShort<whiteToMove>()
            && kingBB == startKing
            && board.rooks<white>() & startingKingsideRook<white>()
            && (csMask & pd->attacked) == 0
            && (csMask & board.occ()) == kingBB
        ) generateSuccessorBoard<whiteToMove, Piece::King, MoveFlag::ShortCastling>(board, kingBB, kingBB << 2);

    if (board.canCastleLong<whiteToMove>()
            && kingBB == startKing
            && board.rooks<white>() & startingQueensideRook<white>()
            && (clMask & pd->attacked) == 0
            && (clMask & board.occ()) == kingBB
            && board.free() & (startingQueensideRook<white>() << 1)
        ) generateSuccessorBoard<whiteToMove, Piece::King, MoveFlag::LongCastling>(board, kingBB, kingBB >> 2);
}

template<typename MC, bool qc, bool co>
PDptr MoveGenerator<MC, qc, co>::pd{std::make_unique<PinData>()};

template<typename MC, bool qc, bool co>
std::array<unsigned int, 6> MoveGenerator<MC, qc, co>::numberOfMovesByPiece{};
template<typename MC, bool qc, bool co>
unsigned long MoveGenerator<MC, qc, co>::numberOfMoves{0};

#endif //DORY_MOVEGEN_H
