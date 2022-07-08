//
// Created by Robin on 01.07.2022.
//

#include <cstdint>
#include "checklogichandler.h"
#include "utils.h"

#ifndef CHESSENGINE_MOVEGEN_H
#define CHESSENGINE_MOVEGEN_H

class MoveCollector {

    void reset() {

    }

    template<Flag_t flags>
    void registerMove(Piece_t piece, BB from, BB to) {
        Move move{from, to, piece, flags};
        printMove<true>(move);
    }

    template<State state>
    void update(Board& board) {
        std::cout << "Next board: " << std::endl;
        print_board(board);
    }

    friend class MoveGenerator;
};

class MoveGenerator {
    CheckLogicHandler clh{};
    MoveCollector coll;
    BB checkMask{0}, targetSquares{0}, pinsStr{0}, pinsDiag{0};

public:
    explicit MoveGenerator(MoveCollector collector) : coll{collector} {}

    template<State state>
    void generate(Board& board) {
        clh.reload<state>(board);
        coll.reset();

        checkMask = clh.getCheckMask();
        targetSquares = board.enemyOrEmpty<state.whiteToMove>() & checkMask;
        pinsDiag = clh.pinsDiagonal;
        pinsStr = clh.pinsStraight;

        if(!clh.isDoubleCheck) {
            pawnMoves<state>(board);
            knightMoves<state>(board);
            bishopMoves<state>(board);
            rookMoves<state>(board);
            queenMoves<state>(board);
            castles<state>(board);
        }

        kingMoves<state>(board);
    }

private:
    template<State state, Flag_t flags = MoveFlag::Silent, BB epField = 0ull>
    void generateSuccessorBoard(Board& board, Piece_t piece, BB from, BB to) {
        coll.registerMove<flags>(piece, from, to);
        constexpr State nextState = state.next<flags, epField>();
        Board nextBoard = board.next<state, flags>(piece, from, to);
        coll.update<nextState>(nextBoard);
    }

    // - - - - - - Helper Functions - - - - - -

    template<State state, Flag_t flags = MoveFlag::Silent>
    void addToList(Board& board, Piece_t piece, int fromIndex, BB targets) {
        BB fromBB = newMask(fromIndex);
        while(targets != 0) {
            int ix = lastBitOf(targets);
            deleteBitAt(targets, ix);
            BB toBB = newMask(ix);
            generateSuccessorBoard<state, flags>(board, piece, fromBB, toBB);
        }
    }

    template<State state>
    void handlePromotions(Board& board, BB from, BB to) {
        generateSuccessorBoard<state, MoveFlag::PromoteQueen>(board, Piece::Pawn, from, to);
        generateSuccessorBoard<state, MoveFlag::PromoteRook>(board, Piece::Pawn, from, to);
        generateSuccessorBoard<state, MoveFlag::PromoteBishop>(board, Piece::Pawn, from, to);
        generateSuccessorBoard<state, MoveFlag::PromoteKnight>(board, Piece::Pawn, from, to);
    }

    // - - - - - - Individual Piece Moves - - - - - -

    template<State state>
    void pawnMoves(Board& board) {
        constexpr bool white = state.whiteToMove;
        BB free = board.free();
        BB enemy = board.enemyPieces<white>();
        BB pawnsFwd = board.pawns<white>() & ~pinsDiag;
        BB pawnCapt = board.pawns<white>() & ~pinsStr;

        // pawns that can move 1 or 2 squares
        BB pwnMov = pawnsFwd & backward<white>(free);
        BB pwnMov2 = pwnMov & backward2<white>(free & checkMask) & firstRank<white>();
        pwnMov &= backward<white>(checkMask);

        // pawns that can capture Left or Right
        BB pawnCapL = pawnCapt & pawnInvAtkLeft<white>(enemy & checkMask) & pawnCanGoLeft<white>();
        BB pawnCapR = pawnCapt & pawnInvAtkRight<white>(enemy & checkMask) & pawnCanGoRight<white>();

        // remove pinned pawns
        pwnMov      &= backward<white> (pinsStr) | ~pinsStr;
        pwnMov2     &= backward2<white>(pinsStr) | ~pinsStr;
        pawnCapL    &= pawnInvAtkLeft<white> (pinsDiag & pawnCanGoRight<white>()) | ~pinsDiag;
        pawnCapR    &= pawnInvAtkRight<white>(pinsDiag & pawnCanGoLeft <white>()) | ~pinsDiag;

        // handle en passant pawns
        BB epPawnL = 0, epPawnR = 0;
        constexpr BB enPassant = state.enPassantField;
        if(enPassant != 0) {
            // left capture is ep square and is on checkmask
            epPawnL = pawnCapt & pawnCanGoLeft<white>() & pawnInvAtkLeft<white>(enPassant & forward<white>(checkMask));
            // remove pinned ep pawns
            epPawnL &= pawnInvAtkLeft<white>(pinsDiag & pawnCanGoLeft<white>()) | ~pinsDiag;
            // handle very special case of two sideways pinned epPawns
            epPawnL = clh.pruneEpPin(epPawnL);

            // right capture is ep square and is on checkmask
            epPawnR = pawnCapt & pawnCanGoRight<white>() & pawnInvAtkRight<white>(enPassant & forward<white>(checkMask));
            // remove pinned ep pawns
            epPawnR &= pawnInvAtkRight<white>(pinsDiag & pawnCanGoRight<white>()) | ~pinsDiag;
            // handle very special case of two sideways pinned epPawns
            epPawnR = clh.pruneEpPin(epPawnR);
        }

        // collect all promoting pawns in separate variables
        BB lastRowMask = pawnOnLastRow<white>();
        BB pwnPromoteFwd  = pwnMov   & lastRowMask;
        BB pwnPromoteL    = pawnCapL & lastRowMask;
        BB pwnPromoteR    = pawnCapR & lastRowMask;

        // remove all promoting pawns from these collections
        pwnMov &= ~lastRowMask;
        pawnCapL &= ~lastRowMask;
        pawnCapR &= ~lastRowMask;

        BB fromBB = 1;
        for(int i = 0; i < 64; i++) {
            // non-promoting pawn moves
            if((pwnMov & fromBB) != 0) {    // straight push, 1 square
                generateSuccessorBoard<state>(board, Piece::Pawn, fromBB, forward<white>(fromBB));
            }
            if((pawnCapL & fromBB) != 0) {  // capture towards left
                generateSuccessorBoard<state> (board, Piece::Pawn, fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((pawnCapR & fromBB) != 0) {  // capture towards right
                generateSuccessorBoard<state>(board, Piece::Pawn, fromBB, pawnAtkRight<white>(fromBB));
            }

            // promoting pawn moves
            if((pwnPromoteFwd & fromBB) != 0) {  // single push + promotion
                handlePromotions<state>(board, fromBB, forward<white>(fromBB));
            }
            if((pwnPromoteL & fromBB) != 0) {  // capture left + promotion
                handlePromotions<state>(board, fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((pwnPromoteR & fromBB) != 0) {  // capture right + promotion
                handlePromotions<state>(board, fromBB, pawnAtkRight<white>(fromBB));
            }

            // pawn moves that cannot be promotions
            if((pwnMov2 & fromBB) != 0) {   // pawn double push
                helpPawnDoublePush<state>(board, i);
            }
            if((epPawnL & fromBB) != 0) {   // pawn ep capture towards left
                generateSuccessorBoard<state, MoveFlag::EnPassantCapture>(board, Piece::Pawn, fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((epPawnR & fromBB) != 0) {   // pawn ep capture towards right
                generateSuccessorBoard<state, MoveFlag::EnPassantCapture>(board, Piece::Pawn, fromBB, pawnAtkRight<white>(fromBB));
            }

            fromBB <<= 1;
        }
    }

    template<State state>
    constexpr void helpPawnDoublePush(Board& board, int fromIndex){
        // TODO: This is really ugly, but we need to call generateSuccessorBoard with a constexpr as epField
        if(state.whiteToMove) {
            if(fromIndex == 8) addPawnDoublePush<state, 8>(board);
            if(fromIndex == 9) addPawnDoublePush<state, 9>(board);
            if(fromIndex == 10) addPawnDoublePush<state, 10>(board);
            if(fromIndex == 11) addPawnDoublePush<state, 11>(board);
            if(fromIndex == 12) addPawnDoublePush<state, 12>(board);
            if(fromIndex == 13) addPawnDoublePush<state, 13>(board);
            if(fromIndex == 14) addPawnDoublePush<state, 14>(board);
            if(fromIndex == 15) addPawnDoublePush<state, 15>(board);
        } else {
            if(fromIndex == 48) addPawnDoublePush<state, 48>(board);
            if(fromIndex == 49) addPawnDoublePush<state, 59>(board);
            if(fromIndex == 50) addPawnDoublePush<state, 50>(board);
            if(fromIndex == 51) addPawnDoublePush<state, 51>(board);
            if(fromIndex == 52) addPawnDoublePush<state, 52>(board);
            if(fromIndex == 53) addPawnDoublePush<state, 53>(board);
            if(fromIndex == 54) addPawnDoublePush<state, 54>(board);
            if(fromIndex == 55) addPawnDoublePush<state, 55>(board);
        }
    }

    template<State state, int fromIndex>
    constexpr void addPawnDoublePush(Board& board) {
        constexpr BB fromBB = newMask(fromIndex);
        generateSuccessorBoard<state, MoveFlag::PawnDoublePush, forward<state.whiteToMove>(fromBB)>(
            board, Piece::Pawn, fromBB, forward2<state.whiteToMove>(fromBB)
        );
    }

    template<State state>
    void knightMoves(Board& board) {
        BB movKnights = board.knights<state.whiteToMove>() & ~clh.allPins();

        while(movKnights != 0) {
            int ix = lastBitOf(movKnights);
            deleteBitAt(movKnights, ix);

            BB targets = PieceSteps::KNIGHT_MOVES[ix] & targetSquares;
            addToList<state>(board, Piece::Knight, ix, targets);
        }
    }

    template<State state>
    void bishopMoves(Board& board) {
        BB bishops = board.bishops<state.whiteToMove>() & ~pinsStr;

        while(bishops != 0) {
            int ix = lastBitOf(bishops);
            deleteBitAt(bishops, ix);

            BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & targetSquares;
            if(hasBitAt(pinsDiag, ix)) targets &= pinsDiag;
            addToList<state>(board, Piece::Bishop, ix, targets);
        }
    }

    template<State state>
    void rookMoves(Board& board) {
        BB rooks = board.rooks<state.whiteToMove>() & ~pinsDiag;

        while(rooks != 0) {
            int ix = lastBitOf(rooks);
            deleteBitAt(rooks, ix);

            BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & targetSquares;
            if(hasBitAt(pinsStr, ix)) targets &= pinsStr;

            if(hasBitAt(startingKingsideRook<state.whiteToMove>(), ix))
                addToList<state, MoveFlag::RemoveShortCastling>(board, Piece::Rook, ix, targets);
            else if (hasBitAt(startingQueensideRook<state.whiteToMove>(), ix))
                addToList<state, MoveFlag::RemoveLongCastling>(board, Piece::Rook, ix, targets);
            else
                addToList<state>(board, Piece::Rook, ix, targets);
        }
    }

    template<State state>
    void queenMoves(Board& board) {
        BB queens = board.queens<state.whiteToMove>();
        BB queensPinStr = queens & pinsStr & ~pinsDiag;
        BB queensPinDiag = queens & pinsDiag & ~pinsStr;
        BB queensNoPin = queens & ~(pinsDiag | pinsStr);

        BB selector = 1;
        for(int i = 0; i < 64; i++) {
            if((queensPinStr & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, i) & targetSquares & pinsStr;
                addToList<state>(board, Piece::Queen, i, targets);
            }
            else if((queensPinDiag & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, i) & targetSquares & pinsDiag;
                addToList<state>(board, Piece::Queen, i, targets);
            }
            else if((queensNoPin & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, i) & targetSquares;
                targets |= PieceSteps::slideMask<state.whiteToMove, true, false>(board, i) & targetSquares;
                addToList<state>(board, Piece::Queen, i, targets);
            }
            selector <<= 1;
        }
    }

    template<State state>
    void kingMoves(Board& board) {
        BB king = board.king<state.whiteToMove>();
        int ix = singleBitOf(king);
        BB targets = PieceSteps::KING_MOVES[ix] & ~clh.attacked & board.enemyOrEmpty<state.whiteToMove>();
        addToList<state, MoveFlag::RemoveAllCastling>(board, Piece::King, ix, targets);
    }

    template<State state>
    void castles(Board& board) {
        constexpr bool white = state.whiteToMove;
        BB kingBB = board.king<white>();
        BB startKing = white ? STARTBOARD.wKing : STARTBOARD.bKing;
        BB csMask = castleShortMask<white>();
        BB clMask = castleLongMask<white>();

        if(kingBB == startKing) {
            if(state.canCastleShort()
               && board.rooks<white>() & startingKingsideRook<white>()
               && csMask & ~clh.attacked
               && (csMask & board.occ()) == kingBB
            ) generateSuccessorBoard<state, MoveFlag::ShortCastling>(board, Piece::King, kingBB, kingBB << 2);

            if(state.canCastleLong()
                && board.rooks<white>() & startingQueensideRook<white>()
                && clMask & ~clh.attacked
                && (clMask & board.occ()) == kingBB
                && board.free() & (startingQueensideRook<white>() << 1)
            ) generateSuccessorBoard<state, MoveFlag::LongCastling>(board, Piece::King, kingBB, kingBB >> 2);
        }
    }
};

#endif //CHESSENGINE_MOVEGEN_H
