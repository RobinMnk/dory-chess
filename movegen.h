//
// Created by Robin on 01.07.2022.
//

#include "checklogichandler.h"
#include "utils.h"

#ifndef CHESSENGINE_MOVEGEN_H
#define CHESSENGINE_MOVEGEN_H

template<typename>
class MoveGenerator {
public:
    template<State, int>
    static void generate(Board& board);

private:
    template<State, int, Piece_t, Flag_t = MoveFlag::Silent>
    static void generateSuccessorBoard(Board& board, BB from, BB to);

    // - - - - - - Helper Functions - - - - - -

    template<State, int, Piece_t, Flag_t = MoveFlag::Silent>
    static void addToList(Board& board, int fromIndex, BB targets);

    template<State, int>
    static void handlePromotions(Board& board, BB from, BB to);

    // - - - - - - Individual Piece Moves - - - - - -

    template<State, int>
    static void pawnMoves(Board& board, PinData& pd);

    template<State, int>
    static void knightMoves(Board& board, PinData& pd);

    template<State, int>
    static void bishopMoves(Board& board, PinData& pd);

    template<State, int>
    static void rookMoves(Board& board, PinData& pd);

    template<State, int>
    static void queenMoves(Board& board, PinData& pd);

    template<State, int>
    static void kingMoves(Board& board, PinData& pd);

    template<State, int>
    static void castles(Board& board, PinData& pd);
};

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::generate(Board& board) {
    PinData pd = CheckLogicHandler::reload<state>(board);

    if(!pd.isDoubleCheck) {
        pawnMoves<state, depth>(board, pd);
        knightMoves<state, depth>(board, pd);
        bishopMoves<state, depth>(board, pd);
        rookMoves<state, depth>(board, pd);
        queenMoves<state, depth>(board, pd);

        if constexpr(canCastle<state>())
            castles<state, depth>(board, pd);
    }

    kingMoves<state, depth>(board, pd);
}

template<typename MoveCollector>
template<State state, int depth, Piece_t piece, Flag_t flags>
void MoveGenerator<MoveCollector>::generateSuccessorBoard(Board& board, BB from, BB to) {
    constexpr State nextState = getNextState<state, flags>();
    Board nextBoard = board.getNextBoard<state, piece, flags>(from, to);

    MoveCollector::template registerMove<state, depth, piece, flags>(board, from, to);
    MoveCollector::template next<nextState, depth>(nextBoard);
}

// - - - - - - Helper Functions - - - - - -

template<typename MoveCollector>
template<State state, int depth, Piece_t piece, Flag_t flags>
void MoveGenerator<MoveCollector>::addToList(Board& board, int fromIndex, BB targets) {
    BB fromBB = newMask(fromIndex);
    Bitloop(targets) {
        BB toBB = isolateLowestBit(targets);
        generateSuccessorBoard<state, depth, piece, flags>(board, fromBB, toBB);
    }
}

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::handlePromotions(Board& board, BB from, BB to) {
    generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteQueen>(board, from, to);
    generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteRook>(board, from, to);
    generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteBishop>(board, from, to);
    generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteKnight>(board, from, to);
}

// - - - - - - Individual Piece Moves - - - - - -

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::pawnMoves(Board& board, PinData& pd) {
    constexpr bool white = state.whiteToMove;
    BB free = board.free();
    BB enemy = board.enemyPieces<white>();
    BB pawnsFwd = board.pawns<white>() & ~pd.pinsDiag;
    BB pawnCapt = board.pawns<white>() & ~pd.pinsStr;

    // pawns that can move 1 or 2 squares
    BB pwnMov = pawnsFwd & backward<white>(free);
    BB pwnMov2 = pwnMov & backward2<white>(free & pd.checkMask) & firstRank<white>();
    pwnMov &= backward<white>(pd.checkMask);

    // pawns that can capture Left or Right
    BB pawnCapL = pawnCapt & pawnInvAtkLeft<white>(enemy & pd.checkMask) & pawnCanGoLeft<white>();
    BB pawnCapR = pawnCapt & pawnInvAtkRight<white>(enemy & pd.checkMask) & pawnCanGoRight<white>();

    // remove pinned pawns
    pwnMov      &= backward<white> (pd.pinsStr) | ~pd.pinsStr;
    pwnMov2     &= backward2<white>(pd.pinsStr) | ~pd.pinsStr;
    pawnCapL    &= pawnInvAtkLeft<white> (pd.pinsDiag & pawnCanGoRight<white>()) | ~pd.pinsDiag;
    pawnCapR    &= pawnInvAtkRight<white>(pd.pinsDiag & pawnCanGoLeft <white>()) | ~pd.pinsDiag;

    // handle en passant pawns
    BB epPawnL{0}, epPawnR{0};
    BB enPassant = board.enPassantField;
    if(enPassant != 0 && !pd.blockEP) {
        // left capture is ep square and is on checkmask
        epPawnL = pawnCapt & pawnCanGoLeft<white>() & pawnInvAtkLeft<white>(enPassant & forward<white>(pd.checkMask));
        // remove pinned ep pawns
        epPawnL &= pawnInvAtkLeft<white>(pd.pinsDiag & pawnCanGoLeft<white>()) | ~pd.pinsDiag;

        // right capture is ep square and is on checkmask
        epPawnR = pawnCapt & pawnCanGoRight<white>() & pawnInvAtkRight<white>(enPassant & forward<white>(pd.checkMask));
        // remove pinned ep pawns
        epPawnR &= pawnInvAtkRight<white>(pd.pinsDiag & pawnCanGoRight<white>()) | ~pd.pinsDiag;
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


    // non-promoting pawn moves
    Bitloop(pwnMov) {   // straight push, 1 square
        int fromIx = firstBitOf(pwnMov);
        generateSuccessorBoard<state, depth, Piece::Pawn>(board, newMask(fromIx), forward<white>(newMask(fromIx)));
    }
    Bitloop(pawnCapL) { // capture towards left
        int fromIx = firstBitOf(pawnCapL);
        generateSuccessorBoard<state, depth, Piece::Pawn>(board, newMask(fromIx), pawnAtkLeft<white>(newMask(fromIx)));
    }
    Bitloop(pawnCapR) { // capture towards right
        int fromIx = firstBitOf(pawnCapR);
        generateSuccessorBoard<state, depth, Piece::Pawn>(board, newMask(fromIx), pawnAtkRight<white>(newMask(fromIx)));
    }

    // promoting pawn moves
    Bitloop(pwnPromoteFwd) {    // single push + promotion
        int fromIx = firstBitOf(pwnPromoteFwd);
        handlePromotions<state, depth>(board, newMask(fromIx), forward<white>(newMask(fromIx)));
    }
    Bitloop(pwnPromoteL) {    // capture left + promotion
        int fromIx = firstBitOf(pwnPromoteL);
        handlePromotions<state, depth>(board, newMask(fromIx), pawnAtkLeft<white>(newMask(fromIx)));
    }
    Bitloop(pwnPromoteR) {    // capture right + promotion
        int fromIx = firstBitOf(pwnPromoteR);
        handlePromotions<state, depth>(board, newMask(fromIx), pawnAtkRight<white>(newMask(fromIx)));
    }

    // pawn moves that cannot be promotions
    Bitloop(pwnMov2) {    // pawn double push
        int fromIx = firstBitOf(pwnMov2);
        generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PawnDoublePush>(board, newMask(fromIx), forward2<white>(newMask(fromIx)));
    }
    Bitloop(epPawnL) {    // pawn double push
        int fromIx = firstBitOf(epPawnL);
        generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::EnPassantCapture>(board, newMask(fromIx), pawnAtkLeft<white>(newMask(fromIx)));
    }
    Bitloop(epPawnR) {    // pawn double push
        int fromIx = firstBitOf(epPawnR);
        generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::EnPassantCapture>(board, newMask(fromIx), pawnAtkRight<white>(newMask(fromIx)));
    }
}

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::knightMoves(Board& board, PinData& pd) {
    BB allPins = pd.pinsStr | pd.pinsDiag;
    BB movKnights = board.knights<state.whiteToMove>() & ~allPins;

    Bitloop(movKnights) {
        int ix = firstBitOf(movKnights);
        BB targets = PieceSteps::KNIGHT_MOVES[ix] & pd.targetSquares;
        addToList<state, depth, Piece::Knight>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::bishopMoves(Board& board, PinData& pd) {
    BB bishops = board.bishops<state.whiteToMove>() & ~pd.pinsStr;

    Bitloop(bishops) {
        int ix = firstBitOf(bishops);
        BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & pd.targetSquares;
        if(hasBitAt(pd.pinsDiag, ix)) targets &= pd.pinsDiag;
        addToList<state, depth, Piece::Bishop>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::rookMoves(Board& board, PinData& pd) {
    BB rooks = board.rooks<state.whiteToMove>() & ~pd.pinsDiag;

    Bitloop(rooks) {
        int ix = firstBitOf(rooks);

        BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & pd.targetSquares;
        if(hasBitAt(pd.pinsStr, ix)) targets &= pd.pinsStr;

        if constexpr(canCastleShort<state>()) {
            if (hasBitAt(startingKingsideRook<state.whiteToMove>(), ix)) {
                addToList<state, depth, Piece::Rook, MoveFlag::RemoveShortCastling>(board, ix, targets);
                continue;
            }
        } else if constexpr(canCastleLong<state>()) {
            if (hasBitAt(startingQueensideRook<state.whiteToMove>(), ix)) {
                addToList<state, depth, Piece::Rook, MoveFlag::RemoveLongCastling>(board, ix, targets);
                continue;
            }
        }

        addToList<state, depth, Piece::Rook>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::queenMoves(Board& board, PinData& pd) {
    BB queens = board.queens<state.whiteToMove>();
    BB queensPinStr = queens & pd.pinsStr & ~pd.pinsDiag;
    BB queensPinDiag = queens & pd.pinsDiag & ~pd.pinsStr;
    BB queensNoPin = queens & ~(pd.pinsDiag | pd.pinsStr);

    Bitloop(queensPinStr) {
        int ix = firstBitOf(queensPinStr);
        BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & pd.targetSquares & pd.pinsStr;
        addToList<state, depth, Piece::Queen>(board, ix, targets);
    }

    Bitloop(queensPinDiag) {
        int ix = firstBitOf(queensPinDiag);
        BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & pd.targetSquares & pd.pinsDiag;
        addToList<state, depth, Piece::Queen>(board, ix, targets);
    }

    Bitloop(queensNoPin) {
        int ix = firstBitOf(queensNoPin);
        BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & pd.targetSquares;
        targets |= PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & pd.targetSquares;
        addToList<state, depth, Piece::Queen>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::kingMoves(Board& board, PinData& pd) {
    BB king = board.king<state.whiteToMove>();
    int ix = singleBitOf(king);
    BB targets = PieceSteps::KING_MOVES[ix] & ~pd.attacked & board.enemyOrEmpty<state.whiteToMove>();
    addToList<state, depth, Piece::King, MoveFlag::RemoveAllCastling>(board, ix, targets);
}

template<typename MoveCollector>
template<State state, int depth>
void MoveGenerator<MoveCollector>::castles(Board& board, PinData& pd) {
    constexpr bool white = state.whiteToMove;
    constexpr BB startKing = white ? STARTBOARD.wKing : STARTBOARD.bKing;
    constexpr BB csMask = castleShortMask<white>();
    constexpr BB clMask = castleLongMask<white>();
    BB kingBB = board.king<white>();

    if constexpr (canCastleShort<state>())
        if(kingBB == startKing
               && board.rooks<white>() & startingKingsideRook<white>()
               && csMask & ~pd.attacked
               && (csMask & board.occ()) == kingBB
        ) generateSuccessorBoard<state, depth, Piece::King, MoveFlag::ShortCastling>(board, kingBB, kingBB << 2);

    if constexpr (canCastleLong<state>())
        if(kingBB == startKing
           && board.rooks<white>() & startingQueensideRook<white>()
           && clMask & ~pd.attacked
           && (clMask & board.occ()) == kingBB
           && board.free() & (startingQueensideRook<white>() << 1)
        ) generateSuccessorBoard<state, depth, Piece::King, MoveFlag::LongCastling>(board, kingBB, kingBB >> 2);
}

#endif //CHESSENGINE_MOVEGEN_H
