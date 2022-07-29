//
// Created by robin on 21.07.2022.
//

#include <string_view>
#include <string>
#include <sstream>
#include <vector>

#include "utils.h"

#ifndef CHESSENGINE_FENREADER_H
#define CHESSENGINE_FENREADER_H

namespace Utils {
    constexpr Board getBoardFromFEN(std::string_view position, std::string_view ep) {
        int rank{7}, file{0};

        BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{0}, bQueens{0}, wKing{0}, bKing{0};
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
                    setBit(wKing, 8 * rank + file);
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
                    setBit(bKing, 8 * rank + file);
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

        BB enPassantField{0};
        if(ep != "-") enPassantField = newMask(Utils::sqId(ep));

        return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing, enPassantField};
    }

    constexpr State toState(const uint8_t code) {
        return {(code & 0b10000) != 0, (code & 0b01000) != 0, (code & 0b00100) != 0, (code & 0b00010) != 0, (code & 0b00001) != 0};
    }

    template<typename Main>
    void loadFEN(std::string_view full_fen) {
        std::stringstream stream(full_fen.data());
        std::string segment;
        std::vector<std::string> seglist;
        while(std::getline(stream, segment, ' ')) seglist.push_back(segment);

        // first position in FEN is board contents
        Board board = getBoardFromFEN(seglist.at(0), seglist.at(3));

        // second position is side to move
        const bool w = seglist.at(1) == "w";

        // castling rights
        const bool wcs = seglist.at(2).find('K') != std::string::npos;
        const bool wcl = seglist.at(2).find('Q') != std::string::npos;
        const bool bcs = seglist.at(2).find('k') != std::string::npos;
        const bool bcl = seglist.at(2).find('q') != std::string::npos;

        if( w &&  wcs &&  wcl &&  bcs &&  bcl) Main::template main<toState(0b11111)>(board);
        if(!w &&  wcs &&  wcl &&  bcs &&  bcl) Main::template main<toState(0b01111)>(board);
        if( w && !wcs &&  wcl &&  bcs &&  bcl) Main::template main<toState(0b10111)>(board);
        if(!w && !wcs &&  wcl &&  bcs &&  bcl) Main::template main<toState(0b00111)>(board);
        if( w &&  wcs && !wcl &&  bcs &&  bcl) Main::template main<toState(0b11011)>(board);
        if(!w &&  wcs && !wcl &&  bcs &&  bcl) Main::template main<toState(0b01011)>(board);
        if( w && !wcs && !wcl &&  bcs &&  bcl) Main::template main<toState(0b10011)>(board);
        if(!w && !wcs && !wcl &&  bcs &&  bcl) Main::template main<toState(0b00011)>(board);
        if( w &&  wcs &&  wcl && !bcs &&  bcl) Main::template main<toState(0b11101)>(board);
        if(!w &&  wcs &&  wcl && !bcs &&  bcl) Main::template main<toState(0b01101)>(board);
        if( w && !wcs &&  wcl && !bcs &&  bcl) Main::template main<toState(0b10101)>(board);
        if(!w && !wcs &&  wcl && !bcs &&  bcl) Main::template main<toState(0b00101)>(board);
        if( w &&  wcs && !wcl && !bcs &&  bcl) Main::template main<toState(0b11001)>(board);
        if(!w &&  wcs && !wcl && !bcs &&  bcl) Main::template main<toState(0b01001)>(board);
        if( w && !wcs && !wcl && !bcs &&  bcl) Main::template main<toState(0b10001)>(board);
        if(!w && !wcs && !wcl && !bcs &&  bcl) Main::template main<toState(0b00001)>(board);
        if( w &&  wcs &&  wcl &&  bcs && !bcl) Main::template main<toState(0b11110)>(board);
        if(!w &&  wcs &&  wcl &&  bcs && !bcl) Main::template main<toState(0b01110)>(board);
        if( w && !wcs &&  wcl &&  bcs && !bcl) Main::template main<toState(0b10110)>(board);
        if(!w && !wcs &&  wcl &&  bcs && !bcl) Main::template main<toState(0b00110)>(board);
        if( w &&  wcs && !wcl &&  bcs && !bcl) Main::template main<toState(0b11010)>(board);
        if(!w &&  wcs && !wcl &&  bcs && !bcl) Main::template main<toState(0b01010)>(board);
        if( w && !wcs && !wcl &&  bcs && !bcl) Main::template main<toState(0b10010)>(board);
        if(!w && !wcs && !wcl &&  bcs && !bcl) Main::template main<toState(0b00010)>(board);
        if( w &&  wcs &&  wcl && !bcs && !bcl) Main::template main<toState(0b11100)>(board);
        if(!w &&  wcs &&  wcl && !bcs && !bcl) Main::template main<toState(0b01100)>(board);
        if( w && !wcs &&  wcl && !bcs && !bcl) Main::template main<toState(0b10100)>(board);
        if(!w && !wcs &&  wcl && !bcs && !bcl) Main::template main<toState(0b00100)>(board);
        if( w &&  wcs && !wcl && !bcs && !bcl) Main::template main<toState(0b11000)>(board);
        if(!w &&  wcs && !wcl && !bcs && !bcl) Main::template main<toState(0b01000)>(board);
        if( w && !wcs && !wcl && !bcs && !bcl) Main::template main<toState(0b10000)>(board);
        if(!w && !wcs && !wcl && !bcs && !bcl) Main::template main<toState(0b00000)>(board);
    }

}

#endif //CHESSENGINE_FENREADER_H
