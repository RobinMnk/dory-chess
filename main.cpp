#include <iostream>
#include <bitset>
#include <chrono>
#include "movegen.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

template<State state, int depth>
void time_movegen(MoveGenerator& gen, Board& board) {
    auto t1 = high_resolution_clock::now();
    gen.generate<state, depth>(board);
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;

    std::cout << "Generated " << gen.nodesAtDepth << " nodes in " << ms_double.count() << "ms\n";
}

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    constexpr State state = STARTSTATE;
    Board board = STARTBOARD;

    MoveGenerator gen{};
    time_movegen<state, 3>(gen, board);


//    Board second = board.next<state, MoveFlag::Silent>(Piece::Pawn, newMask(12), newMask(20));
//
//    print_board(second);
//
//    print_board(board);

//
//    gen.generate<state>(board);

//    auto lst = gen.generate<state>(board);
//
//    std::cout << lst->size() << " legal moves:" << std::endl;
//    for(Move m: lst->moves) {
//        if(m.from == 0 && m.to == 0) break;
//        printMove<true>(m);
//    }
//
//
//    std::cout << "Applying the 10th move" << std::endl;
//    Move move = lst->moves[9];

    return 0;
}
