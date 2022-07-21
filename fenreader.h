//
// Created by robin on 21.07.2022.
//

#include <string_view>
#include <string>
#include "board.h"

#ifndef CHESSENGINE_FENREADER_H
#define CHESSENGINE_FENREADER_H

namespace Utils {

    constexpr Board loadFEN(std::string_view full_fen) {
        int rank{7}, file{0};

        BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{
                0}, bQueens{0}, wKing{0}, bKing{0};
        BB enPassantField{0};
        for (char c: full_fen) {
            if (c == ' ') break;

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

        return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing,enPassantField};
    }

}

#endif //CHESSENGINE_FENREADER_H
