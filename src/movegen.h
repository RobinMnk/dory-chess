//
// Created by Robin on 01.07.2022.
//

#include "checklogichandler.h"

#ifndef DORY_MOVEGEN_H
#define DORY_MOVEGEN_H

template<typename MC, State state, Piece_t piece, Flag_t flags>
concept ValidMoveCollector =
    requires(MC mc, const Board& board, BB from, BB to) {
        MC::template registerMove<state, piece, flags>(board, from, to);
    };

template<typename MC>
class MoveGenerator {
public:
    template<State>
    static bool generate(const Board& board);

private:
    static int moves_found;

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    requires ValidMoveCollector<MC, state, piece, flags>
    static void generateSuccessorBoard(const Board& board, BB from, BB to);

    // - - - - - - Helper Functions - - - - - -

    template<State, Piece_t, Flag_t = MoveFlag::Silent>
    static void addToList(const Board& board, int fromIndex, BB targets);

    template<State>
    static void handlePromotions(const Board& board, BB from, BB to);

    // - - - - - - Individual Piece Moves - - - - - -

    template<State>
    static void pawnMoves(const Board& board, PinData& pd);

    template<State>
    static void knightMoves(const Board& board, PinData& pd);

    template<State>
    static void bishopMoves(const Board& board, PinData& pd);

    template<State>
    static void rookMoves(const Board& board, PinData& pd);

    template<State>
    static void queenMoves(const Board& board, PinData& pd);

    template<State>
    static void kingMoves(const Board& board, PinData& pd);

    template<State>
    static void castles(const Board& board, PinData& pd);
};

template<typename MoveCollector>
template<State state>
bool MoveGenerator<MoveCollector>::generate(const Board& board) {
    PinData pd = CheckLogicHandler::reload<state>(board);
    moves_found = 0;

    if(!pd.isDoubleCheck) {
        pawnMoves<state>(board, pd);
        knightMoves<state>(board, pd);
        bishopMoves<state>(board, pd);
        rookMoves<state>(board, pd);
        queenMoves<state>(board, pd);

        if constexpr(canCastle<state>())
            castles<state>(board, pd);
    }

    kingMoves<state>(board, pd);

    // returns true if the current player is checkmated
    return moves_found == 0 && pd.checkMask < FULL_BB;
}

template<typename MoveCollector>
template<State state, Piece_t piece, Flag_t flags>
requires ValidMoveCollector<MoveCollector, state, piece, flags>
void MoveGenerator<MoveCollector>::generateSuccessorBoard(const Board& board, BB from, BB to) {
    MoveCollector::template registerMove<state, piece, flags>(board, from, to);

//    constexpr State nextState = getNextState<state, flags>();
//    Board nextBoard = board.getNextBoard<state, piece, flags>(from, to);
//    MoveCollector::template next<nextState>(nextBoard);
    moves_found++;
}

// - - - - - - Helper Functions - - - - - -

template<typename MoveCollector>
template<State state, Piece_t piece, Flag_t flags>
void MoveGenerator<MoveCollector>::addToList(const Board& board, int fromIndex, BB targets) {
    BB fromBB = newMask(fromIndex);
    Bitloop(targets) {
        BB toBB = isolateLowestBit(targets);
        generateSuccessorBoard<state, piece, flags>(board, fromBB, toBB);
    }
}

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::handlePromotions(const Board& board, BB from, BB to) {
    generateSuccessorBoard<state, Piece::Pawn, MoveFlag::PromoteQueen>(board, from, to);
    generateSuccessorBoard<state, Piece::Pawn, MoveFlag::PromoteRook>(board, from, to);
    generateSuccessorBoard<state, Piece::Pawn, MoveFlag::PromoteBishop>(board, from, to);
    generateSuccessorBoard<state, Piece::Pawn, MoveFlag::PromoteKnight>(board, from, to);
}

// - - - - - - Individual Piece Moves - - - - - -

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::pawnMoves(const Board& board, PinData& pd) {
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
    if(board.hasEnPassant() != 0 && !pd.blockEP) {
        BB enPassant = board.enPassantBB();
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

    BB from;
    // non-promoting pawn moves
    Bitloop(pwnMov) {   // straight push, 1 square
        from = isolateLowestBit(pwnMov);
        generateSuccessorBoard<state, Piece::Pawn>(board, from, forward<white>(from));
    }
    Bitloop(pawnCapL) { // capture towards left
        from = isolateLowestBit(pawnCapL);
        generateSuccessorBoard<state, Piece::Pawn>(board, from, pawnAtkLeft<white>(from));
    }
    Bitloop(pawnCapR) { // capture towards right
        from = isolateLowestBit(pawnCapR);
        generateSuccessorBoard<state, Piece::Pawn>(board, from, pawnAtkRight<white>(from));
    }

    // promoting pawn moves
    Bitloop(pwnPromoteFwd) {    // single push + promotion
        from = isolateLowestBit(pwnPromoteFwd);
        handlePromotions<state>(board, from, forward<white>(from));
    }
    Bitloop(pwnPromoteL) {    // capture left + promotion
        from = isolateLowestBit(pwnPromoteL);
        handlePromotions<state>(board, from, pawnAtkLeft<white>(from));
    }
    Bitloop(pwnPromoteR) {    // capture right + promotion
        from = isolateLowestBit(pwnPromoteR);
        handlePromotions<state>(board, from, pawnAtkRight<white>(from));
    }

    // pawn moves that cannot be promotions
    Bitloop(pwnMov2) {    // pawn double push
        from = isolateLowestBit(pwnMov2);
        generateSuccessorBoard<state, Piece::Pawn, MoveFlag::PawnDoublePush>(board, from, forward2<white>(from));
    }
    Bitloop(epPawnL) {    // pawn double push
        from = isolateLowestBit(epPawnL);
        generateSuccessorBoard<state, Piece::Pawn, MoveFlag::EnPassantCapture>(board, from, pawnAtkLeft<white>(from));
    }
    Bitloop(epPawnR) {    // pawn double push
        from = isolateLowestBit(epPawnR);
        generateSuccessorBoard<state, Piece::Pawn, MoveFlag::EnPassantCapture>(board, from, pawnAtkRight<white>(from));
    }
}

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::knightMoves(const Board& board, PinData& pd) {
    BB allPins = pd.pinsStr | pd.pinsDiag;
    BB movKnights = board.knights<state.whiteToMove>() & ~allPins;

    Bitloop(movKnights) {
        int ix = firstBitOf(movKnights);
        BB targets = PieceSteps::KNIGHT_MOVES[ix] & pd.targetSquares;
        addToList<state, Piece::Knight>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::bishopMoves(const Board& board, PinData& pd) {
    BB bishops = board.bishops<state.whiteToMove>() & ~pd.pinsStr;

    Bitloop(bishops) {
        int ix = firstBitOf(bishops);
        BB targets = PieceSteps::slideMask<true>(board.occ(), ix) & pd.targetSquares;
        if(hasBitAt(pd.pinsDiag, ix)) targets &= pd.pinsDiag;
        addToList<state, Piece::Bishop>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::rookMoves(const Board& board, PinData& pd) {
    BB rooks = board.rooks<state.whiteToMove>() & ~pd.pinsDiag;

    Bitloop(rooks) {
        int ix = firstBitOf(rooks);

        BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd.targetSquares;
        if(hasBitAt(pd.pinsStr, ix)) targets &= pd.pinsStr;

        if constexpr(canCastleShort<state>()) {
            if (hasBitAt(startingKingsideRook<state.whiteToMove>(), ix)) {
                addToList<state, Piece::Rook, MoveFlag::RemoveShortCastling>(board, ix, targets);
                continue;
            }
        }
        if constexpr(canCastleLong<state>()) {
            if (hasBitAt(startingQueensideRook<state.whiteToMove>(), ix)) {
                addToList<state, Piece::Rook, MoveFlag::RemoveLongCastling>(board, ix, targets);
                continue;
            }
        }

        addToList<state, Piece::Rook>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::queenMoves(const Board& board, PinData& pd) {
    BB queens = board.queens<state.whiteToMove>();
    BB queensPinStr = queens & pd.pinsStr & ~pd.pinsDiag;
    BB queensPinDiag = queens & pd.pinsDiag & ~pd.pinsStr;
    BB queensNoPin = queens & ~(pd.pinsDiag | pd.pinsStr);

    Bitloop(queensPinStr) {
        int ix = firstBitOf(queensPinStr);
        BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd.targetSquares & pd.pinsStr;
        addToList<state, Piece::Queen>(board, ix, targets);
    }

    Bitloop(queensPinDiag) {
        int ix = firstBitOf(queensPinDiag);
        BB targets = PieceSteps::slideMask<true>(board.occ(), ix) & pd.targetSquares & pd.pinsDiag;
        addToList<state, Piece::Queen>(board, ix, targets);
    }

    Bitloop(queensNoPin) {
        int ix = firstBitOf(queensNoPin);
        BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd.targetSquares;
        targets |= PieceSteps::slideMask<true>(board.occ(), ix) & pd.targetSquares;
        addToList<state, Piece::Queen>(board, ix, targets);
    }
}

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::kingMoves(const Board& board, PinData& pd) {
    int ix = board.kingSquare<state.whiteToMove>();
    BB targets = PieceSteps::KING_MOVES[ix] & ~pd.attacked & board.enemyOrEmpty<state.whiteToMove>();
    addToList<state, Piece::King, MoveFlag::RemoveAllCastling>(board, ix, targets);
}

template<typename MoveCollector>
template<State state>
void MoveGenerator<MoveCollector>::castles(const Board& board, PinData& pd) {
    constexpr bool white = state.whiteToMove;
    constexpr BB startKing = STARTBOARD.king<white>();
    constexpr BB csMask = castleShortMask<white>();
    constexpr BB clMask = castleLongMask<white>();
    BB kingBB = board.king<white>();

    if constexpr (canCastleShort<state>())
        if(kingBB == startKing
               && board.rooks<white>() & startingKingsideRook<white>()
               && (csMask & pd.attacked) == 0
               && (csMask & board.occ()) == kingBB
        ) generateSuccessorBoard<state, Piece::King, MoveFlag::ShortCastling>(board, kingBB, kingBB << 2);

    if constexpr (canCastleLong<state>())
        if(kingBB == startKing
           && board.rooks<white>() & startingQueensideRook<white>()
           && (clMask & pd.attacked) == 0
           && (clMask & board.occ()) == kingBB
           && board.free() & (startingQueensideRook<white>() << 1)
        ) generateSuccessorBoard<state, Piece::King, MoveFlag::LongCastling>(board, kingBB, kingBB >> 2);
}

template<typename MC>
int MoveGenerator<MC>::moves_found{0};

template<typename MC>
static bool generate(const Board& board, State state) {
    unsigned int state_code = state.code();
    switch (state_code) {
        case 0: return MoveGenerator<MC>::template generate<toState(0)>(board);
        case 1: return MoveGenerator<MC>::template generate<toState(1)>(board);
        case 2: return MoveGenerator<MC>::template generate<toState(2)>(board);
        case 3: return MoveGenerator<MC>::template generate<toState(3)>(board);
        case 4: return MoveGenerator<MC>::template generate<toState(4)>(board);
        case 5: return MoveGenerator<MC>::template generate<toState(5)>(board);
        case 6: return MoveGenerator<MC>::template generate<toState(6)>(board);
        case 7: return MoveGenerator<MC>::template generate<toState(7)>(board);
        case 8: return MoveGenerator<MC>::template generate<toState(8)>(board);
        case 9: return MoveGenerator<MC>::template generate<toState(9)>(board);
        case 10: return MoveGenerator<MC>::template generate<toState(10)>(board);
        case 11: return MoveGenerator<MC>::template generate<toState(11)>(board);
        case 12: return MoveGenerator<MC>::template generate<toState(12)>(board);
        case 13: return MoveGenerator<MC>::template generate<toState(13)>(board);
        case 14: return MoveGenerator<MC>::template generate<toState(14)>(board);
        case 15: return MoveGenerator<MC>::template generate<toState(15)>(board);
        case 16: return MoveGenerator<MC>::template generate<toState(16)>(board);
        case 17: return MoveGenerator<MC>::template generate<toState(17)>(board);
        case 18: return MoveGenerator<MC>::template generate<toState(18)>(board);
        case 19: return MoveGenerator<MC>::template generate<toState(19)>(board);
        case 20: return MoveGenerator<MC>::template generate<toState(20)>(board);
        case 21: return MoveGenerator<MC>::template generate<toState(21)>(board);
        case 22: return MoveGenerator<MC>::template generate<toState(22)>(board);
        case 23: return MoveGenerator<MC>::template generate<toState(23)>(board);
        case 24: return MoveGenerator<MC>::template generate<toState(24)>(board);
        case 25: return MoveGenerator<MC>::template generate<toState(25)>(board);
        case 26: return MoveGenerator<MC>::template generate<toState(26)>(board);
        case 27: return MoveGenerator<MC>::template generate<toState(27)>(board);
        case 28: return MoveGenerator<MC>::template generate<toState(28)>(board);
        case 29: return MoveGenerator<MC>::template generate<toState(29)>(board);
        case 30: return MoveGenerator<MC>::template generate<toState(30)>(board);
        case 31: return MoveGenerator<MC>::template generate<toState(31)>(board);
        default: return false;
    }
}

#endif //DORY_MOVEGEN_H
