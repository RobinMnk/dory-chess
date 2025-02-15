//
// Created by Robin on 21.07.2022.
//

#include <vector>
#include "../core/board.h"
#include "../utils/utils.h"

#ifndef DORY_FENREADER_H
#define DORY_FENREADER_H

namespace Dory::Utils {

    std::pair<Board, bool> parseFEN(std::vector<std::string>& fenParts, int ix) {

        std::string position = fenParts.at(ix), currentPlayer = fenParts.at(ix+1), castling = fenParts.at(ix+2), ep = fenParts.at(ix+3);

        int rank{7}, file{0};

        BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{0}, bQueens{0};
        uint8_t wKing{0}, bKing{1};
        for (char c: position) {
            switch (c) {
                case 'P':
                    setBit(wPawns, 8 * rank + file);
                    break;
                case 'N':
                    setBit(wKnights, 8 * rank + file);
                    break;
                case 'B':
                    setBit(wBishops, 8 * rank + file);
                    break;
                case 'R':
                    setBit(wRooks, 8 * rank + file);
                    break;
                case 'Q':
                    setBit(wQueens, 8 * rank + file);
                    break;
                case 'K':
                    wKing = 8 * rank + file;
                    break;
                case 'p':
                    setBit(bPawns, 8 * rank + file);
                    break;
                case 'n':
                    setBit(bKnights, 8 * rank + file);
                    break;
                case 'b':
                    setBit(bBishops, 8 * rank + file);
                    break;
                case 'r':
                    setBit(bRooks, 8 * rank + file);
                    break;
                case 'q':
                    setBit(bQueens, 8 * rank + file);
                    break;
                case 'k':
                    bKing = 8 * rank + file;
                    break;
                case '/':
                    rank--;
                    file = 0;
                    continue;
                default:
                    if (isdigit(c)) file += c - '1';
                    break;
            }

            file++;
        }

        uint8_t enPassantField{0};
        if(ep != "-") enPassantField = sqId(ep);

        // second position is side to move
        const bool w = fenParts.at(ix+1) == "w";

        // castling rights
        const bool wcs = fenParts.at(ix+2).find('K') != std::string::npos;
        const bool wcl = fenParts.at(ix+2).find('Q') != std::string::npos;
        const bool bcs = fenParts.at(ix+2).find('k') != std::string::npos;
        const bool bcl = fenParts.at(ix+2).find('q') != std::string::npos;

        uint8_t castlingRights{0};
        if(wcs) castlingRights |= wCastleShortMask;
        if(wcl) castlingRights |= wCastleLongMask;
        if(bcs) castlingRights |= bCastleShortMask;
        if(bcl) castlingRights |= bCastleLongMask;

        return {{ wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing, enPassantField, castlingRights }, w};
    }

    std::pair<Board, bool> parseFEN(const std::string_view& fen) {
        std::stringstream stream(fen.data());
        std::string segment;
        std::vector<std::string> seglist;
        while(std::getline(stream, segment, ' ')) seglist.push_back(segment);
        return parseFEN(seglist, 0);
    }


//    template<typename Main, int depth>
//    void run(uint8_t state_code, const Board& board) {
//        switch (state_code) {
//            case 0:  Main::template main<toState( 0), depth>(board); break;
//            case 1:  Main::template main<toState( 1), depth>(board); break;
//            case 2:  Main::template main<toState( 2), depth>(board); break;
//            case 3:  Main::template main<toState( 3), depth>(board); break;
//            case 4:  Main::template main<toState( 4), depth>(board); break;
//            case 5:  Main::template main<toState( 5), depth>(board); break;
//            case 6:  Main::template main<toState( 6), depth>(board); break;
//            case 7:  Main::template main<toState( 7), depth>(board); break;
//            case 8:  Main::template main<toState( 8), depth>(board); break;
//            case 9:  Main::template main<toState( 9), depth>(board); break;
//            case 10: Main::template main<toState(10), depth>(board); break;
//            case 11: Main::template main<toState(11), depth>(board); break;
//            case 12: Main::template main<toState(12), depth>(board); break;
//            case 13: Main::template main<toState(13), depth>(board); break;
//            case 14: Main::template main<toState(14), depth>(board); break;
//            case 15: Main::template main<toState(15), depth>(board); break;
//            case 16: Main::template main<toState(16), depth>(board); break;
//            case 17: Main::template main<toState(17), depth>(board); break;
//            case 18: Main::template main<toState(18), depth>(board); break;
//            case 19: Main::template main<toState(19), depth>(board); break;
//            case 20: Main::template main<toState(20), depth>(board); break;
//            case 21: Main::template main<toState(21), depth>(board); break;
//            case 22: Main::template main<toState(22), depth>(board); break;
//            case 23: Main::template main<toState(23), depth>(board); break;
//            case 24: Main::template main<toState(24), depth>(board); break;
//            case 25: Main::template main<toState(25), depth>(board); break;
//            case 26: Main::template main<toState(26), depth>(board); break;
//            case 27: Main::template main<toState(27), depth>(board); break;
//            case 28: Main::template main<toState(28), depth>(board); break;
//            case 29: Main::template main<toState(29), depth>(board); break;
//            case 30: Main::template main<toState(30), depth>(board); break;
//            case 31: Main::template main<toState(31), depth>(board); break;
//            default: break;
//        }
//    }

//    template<typename Main, int depth>
//    void loadFEN(std::string_view full_fen) {
//        std::stringstream stream(full_fen.data());
//        std::string segment;
//        std::vector<std::string> seglist;
////        try {
//            while(std::getline(stream, segment, ' ')) seglist.push_back(segment);
//
//            // first position in FEN is board contents
//            Board board = getBoardFromFEN(seglist.at(0), seglist.at(3));
//
//            // second position is side to move
//            const bool w = seglist.at(1) == "w";
//
//            // castling rights
//            const bool wcs = seglist.at(2).find('K') != std::string::npos;
//            const bool wcl = seglist.at(2).find('Q') != std::string::npos;
//            const bool bcs = seglist.at(2).find('k') != std::string::npos;
//            const bool bcl = seglist.at(2).find('q') != std::string::npos;
//
//            uint8_t state_code = 0;
//            if(w)   state_code |= 0b10000;
//
//            run<Main, depth>(state_code, board);
//
////        } catch (std::exception& ex) {
////            std::cerr << "Invalid FEN string!" << std::endl;
////        }
//    }

//    std::pair<Board, State> loadFEN(std::string_view fen) {
//        if (fen == "startpos" || fen == "start") {
//            return { STARTBOARD, STARTSTATE };
//        }
//
//        std::stringstream stream(fen.data());
//        std::string segment;
//        std::vector<std::string> seglist;
////        try {
//        while(std::getline(stream, segment, ' ')) seglist.push_back(segment);
//
//        // first position in FEN is board contents
//        Board board = getBoardFromFEN(seglist.at(0), seglist.at(3));
//
//        // second position is side to move
//        const bool w = seglist.at(1) == "w";
//
//        // castling rights
//        const bool wcs = seglist.at(2).find('K') != std::string::npos;
//        const bool wcl = seglist.at(2).find('Q') != std::string::npos;
//        const bool bcs = seglist.at(2).find('k') != std::string::npos;
//        const bool bcl = seglist.at(2).find('q') != std::string::npos;
//
//        return { board, State(w, wcs, wcl, bcs, bcl)};
//    }

//    template<typename Main>
//    void loadFEN(std::string_view full_fen, int depth) {
//        switch(depth) {
//            case 1: loadFEN<Main, 1>(full_fen); break;
//            case 2: loadFEN<Main, 2>(full_fen); break;
//            case 3: loadFEN<Main, 3>(full_fen); break;
//            case 4: loadFEN<Main, 4>(full_fen); break;
//            case 5: loadFEN<Main, 5>(full_fen); break;
//            case 6: loadFEN<Main, 6>(full_fen); break;
//            case 7: loadFEN<Main, 7>(full_fen); break;
////            case 8: loadFEN<Main, 8>(full_fen); break;
////            case 9: loadFEN<Main, 9>(full_fen); break;
////            case 10: loadFEN<Main,10>(full_fen); break;
////            case 11: loadFEN<Main, 11>(full_fen); break;
////            case 12: loadFEN<Main, 12>(full_fen); break;
////            case 13: loadFEN<Main, 13>(full_fen); break;
////            case 14: loadFEN<Main, 14>(full_fen); break;
////            case 15: loadFEN<Main, 15>(full_fen); break;
//            default: std::cerr << "Depth not implemented!" << std::endl;
//        }
//    }

//    template<typename Main>
//    void startingPositionAtDepth(int depth) {
////        BoardPtr board = std::make_unique<Board>(STARTBOARD);
//        Board board = STARTBOARD;
//        switch(depth) {
//            case 1: Main::template main<STARTSTATE, 1>(board); break;
//            case 2: Main::template main<STARTSTATE, 2>(board); break;
//            case 3: Main::template main<STARTSTATE, 3>(board); break;
//            case 4: Main::template main<STARTSTATE, 4>(board); break;
//            case 5: Main::template main<STARTSTATE, 5>(board); break;
//            case 6: Main::template main<STARTSTATE, 6>(board); break;
//            case 7: Main::template main<STARTSTATE, 7>(board); break;
//            case 8: Main::template main<STARTSTATE, 8>(board); break;
//            case 9: Main::template main<STARTSTATE, 9>(board); break;
//            default: std::cerr << "Depth not implemented!" << std::endl;
//        }
//    }


} // namespace Dory::Utils

#endif //DORY_FENREADER_H
