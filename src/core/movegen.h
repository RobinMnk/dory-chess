//
// Created by Robin on 01.07.2022.
//

#ifndef DORY_MOVEGEN_H
#define DORY_MOVEGEN_H

#include "checklogichandler.h"

namespace Dory {

    template<typename MC, bool whiteToMove, Piece_t piece, Flag_t flags>
    concept ValidMoveCollector = requires(MC mc, const Board &board, BB from, BB to) {
        MC::template nextMove<whiteToMove, piece, flags>(board, from, to);
    };

    template<typename MC, bool whiteToMove, Piece_t piece, Flag_t flags>
    concept ValidMoveCollectorObj = requires(MC* mc, const Board &board, BB from, BB to) {
        mc->template nextMove<whiteToMove, piece, flags>(board, from, to);
    };

    struct GenerationConfig {
        bool quiescence{false}, countOnly{false}, reloadClh{true};
    };

    constexpr GenerationConfig GC_DEFAULT = {false, false, true};
    constexpr GenerationConfig GC_DEFAULT_NO_CLH = {false, false, false};
    constexpr GenerationConfig GC_QUIESCENCE = {true, false, true};
    constexpr GenerationConfig GC_QUIESCENCE_NO_CLH = {true, false, false};
    constexpr GenerationConfig GC_COUNT_ONLY = {false, true, true};

    template<typename Collector>
    class MoveGenerator {
    public:
        static PinData pd;
        static unsigned long numberOfMoves;

        MoveGenerator() = delete;

        template<bool whiteToMove, GenerationConfig config=GC_DEFAULT>
        static void generate(const Board &board);

    private:
        template<bool whiteToMove, GenerationConfig config, Piece_t piece, Flag_t flags = MOVEFLAG_Silent>
        static void generateSuccessorBoard(const Board &board, BB from, BB to)
        requires ValidMoveCollector<Collector, whiteToMove, piece, flags>;

        // - - - - - - Helper Functions - - - - - -

        template<bool whiteToMove, GenerationConfig config, Piece_t, Flag_t=MOVEFLAG_Silent>
        static void addToList(const Board &board, int fromIndex, BB targets);

        template<bool whiteToMove, GenerationConfig config>
        static void handlePromotions(const Board &board, BB from, BB to);

        // - - - - - - Individual Piece Moves - - - - - -

        template<bool whiteToMove, GenerationConfig config>
        static void pawnMoves(const Board &board);

        template<bool whiteToMove, GenerationConfig config>
        static void knightMoves(const Board &board);

        template<bool whiteToMove, GenerationConfig config>
        static void bishopMoves(const Board &board);

        template<bool whiteToMove, GenerationConfig config>
        static void rookMoves(const Board &board);

        template<bool whiteToMove, GenerationConfig config>
        static void queenMoves(const Board &board);

        template<bool whiteToMove, GenerationConfig config>
        static void kingMoves(const Board &board);

        template<bool whiteToMove, GenerationConfig config>
        static void castles(const Board &board);
    };

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::generate(const Board &board) {
        if constexpr (config.reloadClh)
            CheckLogicHandler::reload<whiteToMove>(board, pd);

        if constexpr (config.countOnly) {
            numberOfMoves = 0;
        }

        if (!pd.isDoubleCheck) {
            pawnMoves<whiteToMove, config>(board);
            knightMoves<whiteToMove, config>(board);
            bishopMoves<whiteToMove, config>(board);
            rookMoves<whiteToMove, config>(board);
            queenMoves<whiteToMove, config>(board);

            if (board.canCastle<whiteToMove>())
                castles<whiteToMove, config>(board);
        }

        kingMoves<whiteToMove, config>(board);
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config, Piece_t piece, Flag_t flags>
    void MoveGenerator<Collector>::generateSuccessorBoard(const Board &board, BB from, BB to)
    requires ValidMoveCollector<Collector, whiteToMove, piece, flags>
    {
        if constexpr (config.quiescence) {
            if ((to & board.enemyPieces<whiteToMove>()) == 0)
                return;
        }

        if constexpr (config.countOnly) {
            numberOfMoves++;
            return;
        }

        Collector::template nextMove<whiteToMove, piece, flags>(board, from, to);
    }

// - - - - - - Helper Functions - - - - - -

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config, Piece_t piece, Flag_t flags>
    void MoveGenerator<Collector>::addToList(const Board &board, int fromIndex, BB targets) {
        BB fromBB = newMask(fromIndex);
        Bitloop(targets) {
            BB toBB = isolateLowestBit(targets);
            generateSuccessorBoard<whiteToMove, config, piece, flags>(board, fromBB, toBB);
        }
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::handlePromotions(const Board &board, BB from, BB to) {
        generateSuccessorBoard<whiteToMove, config, PIECE_Pawn, MOVEFLAG_PromoteQueen>(board, from, to);
        generateSuccessorBoard<whiteToMove, config, PIECE_Pawn, MOVEFLAG_PromoteRook>(board, from, to);
        generateSuccessorBoard<whiteToMove, config, PIECE_Pawn, MOVEFLAG_PromoteBishop>(board, from, to);
        generateSuccessorBoard<whiteToMove, config, PIECE_Pawn, MOVEFLAG_PromoteKnight>(board, from, to);
    }

// - - - - - - Individual Piece Moves - - - - - -

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::pawnMoves(const Board &board) {
        constexpr bool white = whiteToMove;
        BB free = board.free();
        BB enemy = board.enemyPieces<white>();
        BB pawnsFwd = board.pawns<white>() & ~pd.pinsDiag;
        BB pawnCapt = board.pawns<white>() & ~pd.pinsStr;

        // pawns that can move 1 or 2 squares
        BB pwnMov = pawnsFwd & backward<white>(free);
        BB pwnMov2 = pwnMov & backward2<white>(free & pd.checkMask) & firstPawnRank<white>();
        pwnMov &= backward<white>(pd.checkMask);

        // pawns that can capture Left or Right
        BB pawnCapL = pawnCapt & pawnInvAtkLeft<white>(enemy & pd.checkMask) & pawnCanGoLeft<white>();
        BB pawnCapR = pawnCapt & pawnInvAtkRight<white>(enemy & pd.checkMask) & pawnCanGoRight<white>();

        // remove pinned pawns
        pwnMov &= backward<white>(pd.pinsStr) | ~pd.pinsStr;
        pwnMov2 &= backward2<white>(pd.pinsStr) | ~pd.pinsStr;
        pawnCapL &= pawnInvAtkLeft<white>(pd.pinsDiag & pawnCanGoRight<white>()) | ~pd.pinsDiag;
        pawnCapR &= pawnInvAtkRight<white>(pd.pinsDiag & pawnCanGoLeft<white>()) | ~pd.pinsDiag;

        // handle en passant pawns
        BB epPawnL{0}, epPawnR{0};
        if (board.hasEnPassant() != 0 && !pd.blockEP) {
            BB enPassant = board.enPassantBB();
            // left capture is ep square and is on checkmask
            epPawnL = pawnCapt & pawnCanGoLeft<white>() &
                      pawnInvAtkLeft<white>(enPassant & forward<white>(pd.checkMask));
            // remove pinned ep pawns
            epPawnL &= pawnInvAtkLeft<white>(pd.pinsDiag & pawnCanGoLeft<white>()) | ~pd.pinsDiag;

            // right capture is ep square and is on checkmask
            epPawnR = pawnCapt & pawnCanGoRight<white>() &
                      pawnInvAtkRight<white>(enPassant & forward<white>(pd.checkMask));
            // remove pinned ep pawns
            epPawnR &= pawnInvAtkRight<white>(pd.pinsDiag & pawnCanGoRight<white>()) | ~pd.pinsDiag;
        }

        // collect all promoting pawns in separate variables
        BB lastRowMask = pawnOnLastRow<white>();
        BB pwnPromoteFwd = pwnMov & lastRowMask;
        BB pwnPromoteL = pawnCapL & lastRowMask;
        BB pwnPromoteR = pawnCapR & lastRowMask;

        // remove all promoting pawns from these collections
        pwnMov &= ~lastRowMask;
        pawnCapL &= ~lastRowMask;
        pawnCapR &= ~lastRowMask;

        BB from;
        // non-promoting pawn moves
        Bitloop(pwnMov) {   // straight push, 1 square
            from = isolateLowestBit(pwnMov);
            generateSuccessorBoard<whiteToMove, config, PIECE_Pawn>(board, from, forward<white>(from));
        }
        Bitloop(pawnCapL) { // capture towards left
            from = isolateLowestBit(pawnCapL);
            generateSuccessorBoard<whiteToMove, config, PIECE_Pawn>(board, from, pawnAtkLeft<white>(from));
        }
        Bitloop(pawnCapR) { // capture towards right
            from = isolateLowestBit(pawnCapR);
            generateSuccessorBoard<whiteToMove, config, PIECE_Pawn>(board, from, pawnAtkRight<white>(from));
        }

        // promoting pawn moves
        Bitloop(pwnPromoteFwd) {    // single push + promotion
            from = isolateLowestBit(pwnPromoteFwd);
            handlePromotions<whiteToMove, config>(board, from, forward<white>(from));
        }
        Bitloop(pwnPromoteL) {    // capture left + promotion
            from = isolateLowestBit(pwnPromoteL);
            handlePromotions<whiteToMove, config>(board, from, pawnAtkLeft<white>(from));
        }
        Bitloop(pwnPromoteR) {    // capture right + promotion
            from = isolateLowestBit(pwnPromoteR);
            handlePromotions<whiteToMove, config>(board, from, pawnAtkRight<white>(from));
        }

        // pawn moves that cannot be promotions
        Bitloop(pwnMov2) {    // pawn double push
            from = isolateLowestBit(pwnMov2);
            generateSuccessorBoard<whiteToMove, config, PIECE_Pawn, MOVEFLAG_PawnDoublePush>(board, from,  forward2<white>(from));
        }
        Bitloop(epPawnL) {    // en passant left
            from = isolateLowestBit(epPawnL);
            generateSuccessorBoard<whiteToMove, config, PIECE_Pawn, MOVEFLAG_EnPassantCapture>(board, from, pawnAtkLeft<white>(from));
        }
        Bitloop(epPawnR) {    // en passant right
            from = isolateLowestBit(epPawnR);
            generateSuccessorBoard<whiteToMove, config, PIECE_Pawn, MOVEFLAG_EnPassantCapture>(board, from, pawnAtkRight<white>(from));
        }
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::knightMoves(const Board &board) {
        BB allPins = pd.pinsStr | pd.pinsDiag;
        BB movKnights = board.knights<whiteToMove>() & ~allPins;

        Bitloop(movKnights) {
            int ix = firstBitOf(movKnights);
            BB targets = PieceSteps::KNIGHT_MOVES[ix] & pd.targetSquares;
            addToList<whiteToMove, config, PIECE_Knight>(board, ix, targets);
        }
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::bishopMoves(const Board &board) {
        BB bishops = board.bishops<whiteToMove>() & ~pd.pinsStr;

        Bitloop(bishops) {
            int ix = firstBitOf(bishops);
            BB targets = PieceSteps::slideMask<true>(board.occ(), ix) & pd.targetSquares;
            if (hasBitAt(pd.pinsDiag, ix)) targets &= pd.pinsDiag;
            addToList<whiteToMove, config, PIECE_Bishop>(board, ix, targets);
        }
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::rookMoves(const Board &board) {
        BB rooks = board.rooks<whiteToMove>() & ~pd.pinsDiag;

        Bitloop(rooks) {
            int ix = firstBitOf(rooks);

            BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd.targetSquares;
            if (hasBitAt(pd.pinsStr, ix)) targets &= pd.pinsStr;

            if (board.canCastleShort<whiteToMove>() && hasBitAt(startingKingsideRook<whiteToMove>(), ix)) {
                addToList<whiteToMove, config, PIECE_Rook, MOVEFLAG_RemoveShortCastling>(board, ix, targets);
                continue;
            }
            if (board.canCastleLong<whiteToMove>() && hasBitAt(startingQueensideRook<whiteToMove>(), ix)) {
                addToList<whiteToMove, config, PIECE_Rook, MOVEFLAG_RemoveLongCastling>(board, ix, targets);
                continue;
            }

            addToList<whiteToMove, config, PIECE_Rook>(board, ix, targets);
        }
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::queenMoves(const Board &board) {
        BB queens = board.queens<whiteToMove>();
        BB queensPinStr = queens & pd.pinsStr & ~pd.pinsDiag;
        BB queensPinDiag = queens & pd.pinsDiag & ~pd.pinsStr;
        BB queensNoPin = queens & ~(pd.pinsDiag | pd.pinsStr);

        Bitloop(queensPinStr) {
            int ix = firstBitOf(queensPinStr);
            BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd.targetSquares & pd.pinsStr;
            addToList<whiteToMove, config, PIECE_Queen>(board, ix, targets);
        }

        Bitloop(queensPinDiag) {
            int ix = firstBitOf(queensPinDiag);
            BB targets = PieceSteps::slideMask<true>(board.occ(), ix) & pd.targetSquares & pd.pinsDiag;
            addToList<whiteToMove, config, PIECE_Queen>(board, ix, targets);
        }

        Bitloop(queensNoPin) {
            int ix = firstBitOf(queensNoPin);
            BB targets = PieceSteps::slideMask<false>(board.occ(), ix) & pd.targetSquares;
            targets |= PieceSteps::slideMask<true>(board.occ(), ix) & pd.targetSquares;
            addToList<whiteToMove, config, PIECE_Queen>(board, ix, targets);
        }
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::kingMoves(const Board &board) {
        int ix = board.kingSquare<whiteToMove>();
        BB targets = PieceSteps::KING_MOVES[ix] & ~pd.attacked & board.enemyOrEmpty<whiteToMove>();
        addToList<whiteToMove, config, PIECE_King, MOVEFLAG_RemoveAllCastling>(board, ix, targets);
    }

    template<typename Collector>
    template<bool whiteToMove, GenerationConfig config>
    void MoveGenerator<Collector>::castles(const Board &board) {
        constexpr bool white = whiteToMove;
        constexpr BB startKing = STARTBOARD.king<white>();
        constexpr BB csMask = castleShortMask<white>();
        constexpr BB clMask = castleLongMask<white>();
        BB kingBB = board.king<white>();

        if (board.canCastleShort<whiteToMove>()
            && kingBB == startKing
            && board.rooks<white>() & startingKingsideRook<white>()
            && (csMask & pd.attacked) == 0
            && (csMask & board.occ()) == kingBB
        )
            generateSuccessorBoard<whiteToMove, config, PIECE_King, MOVEFLAG_ShortCastling>(board, kingBB, startKing << 2);

        if (board.canCastleLong<whiteToMove>()
            && kingBB == startKing
            && board.rooks<white>() & startingQueensideRook<white>()
            && (clMask & pd.attacked) == 0
            && (clMask & board.occ()) == kingBB
            && board.free() & (startingQueensideRook<white>() << 1)
        )
            generateSuccessorBoard<whiteToMove, config, PIECE_King, MOVEFLAG_LongCastling>(board, kingBB, startKing >> 2);
    }

    template<typename MC>
    PinData MoveGenerator<MC>::pd{};

    template<typename MC>
    unsigned long MoveGenerator<MC>::numberOfMoves{0};

} // namespace Dory

#endif //DORY_MOVEGEN_H
