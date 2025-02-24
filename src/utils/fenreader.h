//
// Created by Robin on 21.07.2022.
//

#ifndef DORY_FENREADER_H
#define DORY_FENREADER_H

#include <vector>
#include "../core/board.h"
#include "../utils/utils.h"

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
        if(fen == "startpos" || fen == "start") return {STARTBOARD, true};
        std::stringstream stream(fen.data());
        std::string segment;
        std::vector<std::string> seglist;
        while(std::getline(stream, segment, ' ')) seglist.push_back(segment);
        return parseFEN(seglist, 0);
    }

} // namespace Dory::Utils

#endif //DORY_FENREADER_H
