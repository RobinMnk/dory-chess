//
// Created by Robin on 29.06.2022.
//

#include "utils.h"

/*
void add_piece_optimized(ByteBoard byteboard, BB pieces, char letter) {
    BB copied_pieces = pieces;
    Bitloop(copied_pieces) {
        const square sq = SquareOf(copied_pieces);
        byteboard[sq] = letter;
    }
}*/

void add_piece(ByteBoard& byteboard, BB pieces, char letter) {
    BB selector = 1;
    for(int i = 0; i < 64; i++) {
        if(pieces & selector) byteboard[i] = letter;
        selector <<= 1;
    }
}

ByteBoard to_byteboard(Board board){
    ByteBoard byteboard{};
    add_piece(byteboard, board.wPawns, 'P');
    add_piece(byteboard, board.bPawns, 'p');
    add_piece(byteboard, board.wKnights, 'N');
    add_piece(byteboard, board.bKnights, 'n');
    add_piece(byteboard, board.wBishops, 'B');
    add_piece(byteboard, board.bBishops, 'b');
    add_piece(byteboard, board.wRooks, 'R');
    add_piece(byteboard, board.bRooks, 'r');
    add_piece(byteboard, board.wQueens, 'Q');
    add_piece(byteboard, board.bQueens, 'q');
    add_piece(byteboard, board.wKing, 'K');
    add_piece(byteboard, board.bKing, 'k');
    return byteboard;
}

void print_board(ByteBoard&& byteboard) {
    int rank = 7;
    while(rank >= 0) {
        for(int i = 0; i < 8; i++) {
            uint8_t letter = byteboard.at(i + rank * 8);
            if(letter != 0) std::cout << letter << " ";
            else std::cout << "  ";
        }
        std::cout << "\n";
        rank--;
    }
    std::cout << std::endl;
}


void print_board(Board board) {
    print_board(to_byteboard(board));
}