//
// Created by Robin on 01.07.2022.
//

#include <cstdint>
#include <vector>
#include "checklogichandler.h"
#include "utils.h"

#ifndef CHESSENGINE_MOVEGEN_H
#define CHESSENGINE_MOVEGEN_H

class MoveCollector {
public:
    std::vector<State> states;
    std::vector<Board> boards;
    int moves{0};

private:
    void reset() {

    }

    template<Flag_t flags, bool whiteMoved>
    void registerMove(Piece_t piece, BB from, BB to, int depth) {
        Move move{from, to, piece, flags};
        for(int i = 0; i < 3 - depth; i++) std::cout << "\t";
        printMove<whiteMoved>(move);
    }

    template<State state>
    void update(Board& board) {
        states.push_back(state);
        boards.push_back(board);
        moves++;
    }

    friend class MoveGenerator;
};

class MoveGenerator {
    CheckLogicHandler clh{};

public:
    MoveCollector coll{};
    unsigned long long nodesAtDepth{0};

    template<State state, int depth>
    constexpr void generate(Board& board) {
        if constexpr (depth > 0) {
            coll.reset();

            PinData pd = clh.reload<state>(board);

            if(!pd.isDoubleCheck) {
                pawnMoves<state, depth>(board, pd);
                knightMoves<state, depth>(board, pd);
                bishopMoves<state, depth>(board, pd);
                rookMoves<state, depth>(board, pd);
                queenMoves<state, depth>(board, pd);
                castles<state, depth>(board, pd);
            }

            kingMoves<state, depth>(board, pd);
        }
    }

private:
    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    void generateSuccessorBoard(Board& board, BB from, BB to) {
//        coll.registerMove<flags, state.whiteToMove>(piece, from, to, depth);
        constexpr State nS = nextState<state, flags>();
        Board nextBoard = board.next<state, piece, flags>(from, to);
//        coll.update<nextState>(nextBoard);

        if(depth == 1) nodesAtDepth++;
        generate<nS, depth-1>(nextBoard);
    }

    // - - - - - - Helper Functions - - - - - -

    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    void addToList(Board& board, int fromIndex, BB targets) {
        BB fromBB = newMask(fromIndex);
        while(targets != 0) {
            int ix = lastBitOf(targets);
            deleteBitAt(targets, ix);
            BB toBB = newMask(ix);
            generateSuccessorBoard<state, depth, piece, flags>(board, fromBB, toBB);
        }
    }

    template<State state, int depth>
    void handlePromotions(Board& board, BB from, BB to) {
        generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteQueen>(board, from, to);
        generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteRook>(board, from, to);
        generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteBishop>(board, from, to);
        generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PromoteKnight>(board, from, to);
    }

    // - - - - - - Individual Piece Moves - - - - - -

    template<State state, int depth>
    void pawnMoves(Board& board, PinData& pd) {
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
        BB epPawnL = 0, epPawnR = 0;
        BB enPassant = board.enPassantField;
        if(enPassant != 0 && !pd.blockEP) {
            // left capture is ep square and is on checkmask
            epPawnL = pawnCapt & pawnCanGoLeft<white>() & pawnInvAtkLeft<white>(enPassant & forward<white>(pd.checkMask));
            // remove pinned ep pawns
            epPawnL &= pawnInvAtkLeft<white>(pd.pinsDiag & pawnCanGoLeft<white>()) | ~pd.pinsDiag;
            // handle very special case of two sideways pinned epPawns
//            epPawnL = clh.pruneEpPin(epPawnL);

            // right capture is ep square and is on checkmask
            epPawnR = pawnCapt & pawnCanGoRight<white>() & pawnInvAtkRight<white>(enPassant & forward<white>(pd.checkMask));
            // remove pinned ep pawns
            epPawnR &= pawnInvAtkRight<white>(pd.pinsDiag & pawnCanGoRight<white>()) | ~pd.pinsDiag;
            // handle very special case of two sideways pinned epPawns
//            epPawnR = clh.pruneEpPin(epPawnR);
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
                generateSuccessorBoard<state, depth, Piece::Pawn>(board, fromBB, forward<white>(fromBB));
            }
            if((pawnCapL & fromBB) != 0) {  // capture towards left
                generateSuccessorBoard<state, depth, Piece::Pawn> (board, fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((pawnCapR & fromBB) != 0) {  // capture towards right
                generateSuccessorBoard<state, depth, Piece::Pawn>(board, fromBB, pawnAtkRight<white>(fromBB));
            }

            // promoting pawn moves
            if((pwnPromoteFwd & fromBB) != 0) {  // single push + promotion
                handlePromotions<state, depth>(board, fromBB, forward<white>(fromBB));
            }
            if((pwnPromoteL & fromBB) != 0) {  // capture left + promotion
                handlePromotions<state, depth>(board, fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((pwnPromoteR & fromBB) != 0) {  // capture right + promotion
                handlePromotions<state, depth>(board, fromBB, pawnAtkRight<white>(fromBB));
            }

            // pawn moves that cannot be promotions
            if((pwnMov2 & fromBB) != 0) {   // pawn double push
                generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PawnDoublePush>(board, fromBB, forward2<white>(fromBB));
            }
            if((epPawnL & fromBB) != 0) {   // pawn ep capture towards left
                generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::EnPassantCapture>(board, fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((epPawnR & fromBB) != 0) {   // pawn ep capture towards right
                generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::EnPassantCapture>(board, fromBB, pawnAtkRight<white>(fromBB));
            }

            fromBB <<= 1;
        }
    }

    template<State state, int depth>
    void knightMoves(Board& board, PinData& pd) {
        BB allPins = pd.pinsStr | pd.pinsDiag;
        BB movKnights = board.knights<state.whiteToMove>() & ~allPins;

        while(movKnights != 0) {
            int ix = lastBitOf(movKnights);
            deleteBitAt(movKnights, ix);

            BB targets = PieceSteps::KNIGHT_MOVES[ix] & pd.targetSquares;
            addToList<state, depth, Piece::Knight>(board, ix, targets);
        }
    }

    template<State state, int depth>
    void bishopMoves(Board& board, PinData& pd) {
        BB bishops = board.bishops<state.whiteToMove>() & ~pd.pinsStr;

        while(bishops != 0) {
            int ix = lastBitOf(bishops);
            deleteBitAt(bishops, ix);

            BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & pd.targetSquares;
            if(hasBitAt(pd.pinsDiag, ix)) targets &= pd.pinsDiag;
            addToList<state, depth, Piece::Bishop>(board, ix, targets);
        }
    }

    template<State state, int depth>
    void rookMoves(Board& board, PinData& pd) {
        BB rooks = board.rooks<state.whiteToMove>() & ~pd.pinsDiag;

        while(rooks != 0) {
            int ix = lastBitOf(rooks);
            deleteBitAt(rooks, ix);

            BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & pd.targetSquares;
            if(hasBitAt(pd.pinsStr, ix)) targets &= pd.pinsStr;

            if(hasBitAt(startingKingsideRook<state.whiteToMove>(), ix))
                addToList<state, depth, Piece::Rook, MoveFlag::RemoveShortCastling>(board, ix, targets);
            else if (hasBitAt(startingQueensideRook<state.whiteToMove>(), ix))
                addToList<state, depth, Piece::Rook, MoveFlag::RemoveLongCastling>(board, ix, targets);
            else
                addToList<state, depth, Piece::Rook>(board, ix, targets);
        }
    }

    template<State state, int depth>
    void queenMoves(Board& board, PinData& pd) {
        BB queens = board.queens<state.whiteToMove>();
        BB queensPinStr = queens & pd.pinsStr & ~pd.pinsDiag;
        BB queensPinDiag = queens & pd.pinsDiag & ~pd.pinsStr;
        BB queensNoPin = queens & ~(pd.pinsDiag | pd.pinsStr);

        BB selector = 1;
        for(int i = 0; i < 64; i++) {
            if((queensPinStr & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, i) & pd.targetSquares & pd.pinsStr;
                addToList<state, depth, Piece::Queen>(board, i, targets);
            }
            else if((queensPinDiag & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, i) & pd.targetSquares & pd.pinsDiag;
                addToList<state, depth, Piece::Queen>(board, i, targets);
            }
            else if((queensNoPin & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, i) & pd.targetSquares;
                targets |= PieceSteps::slideMask<state.whiteToMove, true, false>(board, i) & pd.targetSquares;
                addToList<state, depth, Piece::Queen>(board, i, targets);
            }
            selector <<= 1;
        }
    }

    template<State state, int depth>
    void kingMoves(Board& board, PinData& pd) {
        BB king = board.king<state.whiteToMove>();
        int ix = singleBitOf(king);
        BB targets = PieceSteps::KING_MOVES[ix] & ~pd.attacked & board.enemyOrEmpty<state.whiteToMove>();
        addToList<state, depth, Piece::King, MoveFlag::RemoveAllCastling>(board, ix, targets);
    }

    template<State state, int depth>
    void castles(Board& board, PinData& pd) {
        constexpr bool white = state.whiteToMove;
        BB kingBB = board.king<white>();
        BB startKing = white ? STARTBOARD.wKing : STARTBOARD.bKing;
        BB csMask = castleShortMask<white>();
        BB clMask = castleLongMask<white>();

        if(kingBB == startKing) {
            if(canCastleShort<state>()
               && board.rooks<white>() & startingKingsideRook<white>()
               && csMask & ~pd.attacked
               && (csMask & board.occ()) == kingBB
            ) generateSuccessorBoard<state, depth, Piece::King, MoveFlag::ShortCastling>(board, kingBB, kingBB << 2);

            if(canCastleLong<state>()
                && board.rooks<white>() & startingQueensideRook<white>()
                && clMask & ~pd.attacked
                && (clMask & board.occ()) == kingBB
                && board.free() & (startingQueensideRook<white>() << 1)
            ) generateSuccessorBoard<state, depth, Piece::King, MoveFlag::LongCastling>(board, kingBB, kingBB >> 2);
        }
    }
};

#endif //CHESSENGINE_MOVEGEN_H
