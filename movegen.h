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
    std::vector<Move> moves;
    std::vector<long> follow_positions{};
    unsigned long long nodes{0}, captures{0}, checks{0};
private:
    CheckLogicHandler clh{};

    template<Flag_t flags, bool whiteMoved>
    void registerMove(Piece_t piece, BB from, BB to, int depth) {
        if(depth == 1) {
            moves.push_back(Move{from, to, piece, flags});
            follow_positions.push_back(0);
        }
        if(depth == 1) {
            follow_positions.at(follow_positions.size() - 1) += 1;
        }
//        if(depth > 0) {
//            Move move{from, to, piece, flags};
//            for(int i = 0; i < 3 - depth; i++) std::cout << "\t";
//            printMove<whiteMoved>(move);
//        }
    }

    template<State state, State nextState>
    void countMoves(Board& board, Board& nextBoard, BB from, BB to) {
        nodes++;
        if(board.enemyPieces<state.whiteToMove>() & to) {
            captures++;
        }
//        if(clh.reload<nextState>(nextBoard).isCheck()) checks++;
    }

    template<State state>
    void update(Board& board) {
    }

    friend class MoveGenerator;
};

class MoveGenerator {
    CheckLogicHandler clh{};

public:
    MoveCollector coll{};

    template<State state, int depth>
    constexpr void generate(Board& board) {
        if constexpr (depth > 0) {
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
//        if(hasBitAt(board.wPawns, 28) && hasBitAt(board.bPawns, 37)) coll.registerMove<flags, state.whiteToMove>(piece, from, to, depth);
        constexpr State nS = nextState<state, flags>();
        Board nextBoard = board.next<state, piece, flags>(from, to);
//        coll.update<nextState>(nextBoard);

        if(depth == 1) coll.countMoves<state, nS>(board, nextBoard, from, to);
        generate<nS, depth-1>(nextBoard);
    }

    // - - - - - - Helper Functions - - - - - -

    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    void addToList(Board& board, int fromIndex, BB targets) {
        BB fromBB = newMask(fromIndex);
        Bitloop(targets) {
            int ix = lastBitOf(targets);
            generateSuccessorBoard<state, depth, piece, flags>(board, fromBB, newMask(ix));
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


        // non-promoting pawn moves
        Bitloop(pwnMov) {   // straight push, 1 square
            int fromIx = lastBitOf(pwnMov);
            generateSuccessorBoard<state, depth, Piece::Pawn>(board, newMask(fromIx), forward<white>(newMask(fromIx)));
        }
        Bitloop(pawnCapL) { // capture towards left
            int fromIx = lastBitOf(pawnCapL);
            generateSuccessorBoard<state, depth, Piece::Pawn>(board, newMask(fromIx), pawnAtkLeft<white>(newMask(fromIx)));
        }
        Bitloop(pawnCapR) { // capture towards right
            int fromIx = lastBitOf(pawnCapR);
            generateSuccessorBoard<state, depth, Piece::Pawn>(board, newMask(fromIx), pawnAtkRight<white>(newMask(fromIx)));
        }

        // promoting pawn moves
        Bitloop(pwnPromoteFwd) {    // single push + promotion
            int fromIx = lastBitOf(pwnPromoteFwd);
            handlePromotions<state, depth>(board, newMask(fromIx), forward<white>(newMask(fromIx)));
        }
        Bitloop(pwnPromoteL) {    // capture left + promotion
            int fromIx = lastBitOf(pwnPromoteL);
            handlePromotions<state, depth>(board, newMask(fromIx), pawnAtkLeft<white>(newMask(fromIx)));
        }
        Bitloop(pwnPromoteR) {    // capture right + promotion
            int fromIx = lastBitOf(pwnPromoteR);
            handlePromotions<state, depth>(board, newMask(fromIx), pawnAtkRight<white>(newMask(fromIx)));
        }

        // pawn moves that cannot be promotions
        Bitloop(pwnMov2) {    // pawn double push
            int fromIx = lastBitOf(pwnMov2);
            generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::PawnDoublePush>(board, newMask(fromIx), forward2<white>(newMask(fromIx)));
        }
        Bitloop(epPawnL) {    // pawn double push
            int fromIx = lastBitOf(epPawnL);
            generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::EnPassantCapture>(board, newMask(fromIx), pawnAtkLeft<white>(newMask(fromIx)));
        }
        Bitloop(epPawnR) {    // pawn double push
            int fromIx = lastBitOf(epPawnR);
            generateSuccessorBoard<state, depth, Piece::Pawn, MoveFlag::EnPassantCapture>(board, newMask(fromIx), pawnAtkRight<white>(newMask(fromIx)));
        }
    }

    template<State state, int depth>
    void knightMoves(Board& board, PinData& pd) {
        BB allPins = pd.pinsStr | pd.pinsDiag;
        BB movKnights = board.knights<state.whiteToMove>() & ~allPins;

        Bitloop(movKnights) {
            int ix = lastBitOf(movKnights);
            BB targets = PieceSteps::KNIGHT_MOVES[ix] & pd.targetSquares;
            addToList<state, depth, Piece::Knight>(board, ix, targets);
        }
    }

    template<State state, int depth>
    void bishopMoves(Board& board, PinData& pd) {
        BB bishops = board.bishops<state.whiteToMove>() & ~pd.pinsStr;

        Bitloop(bishops) {
            int ix = lastBitOf(bishops);

            BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & pd.targetSquares;
            if(hasBitAt(pd.pinsDiag, ix)) targets &= pd.pinsDiag;
            addToList<state, depth, Piece::Bishop>(board, ix, targets);
        }
    }

    template<State state, int depth>
    void rookMoves(Board& board, PinData& pd) {
        BB rooks = board.rooks<state.whiteToMove>() & ~pd.pinsDiag;

        Bitloop(rooks) {
            int ix = lastBitOf(rooks);

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

        Bitloop(queensPinStr) {
            int ix = lastBitOf(queensPinStr);
            BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & pd.targetSquares & pd.pinsStr;
            addToList<state, depth, Piece::Queen>(board, ix, targets);
        }

        Bitloop(queensPinDiag) {
            int ix = lastBitOf(queensPinDiag);
            BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & pd.targetSquares & pd.pinsDiag;
            addToList<state, depth, Piece::Queen>(board, ix, targets);
        }

        Bitloop(queensNoPin) {
            int ix = lastBitOf(queensNoPin);
            BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & pd.targetSquares;
            targets |= PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & pd.targetSquares;
            addToList<state, depth, Piece::Queen>(board, ix, targets);
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
};

#endif //CHESSENGINE_MOVEGEN_H
