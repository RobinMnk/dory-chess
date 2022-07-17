#include <iostream>
#include <bitset>
#include <chrono>

#include "movecollectors.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

template<typename Collector, State state, int depth>
void time_movegen(Board& board) {
    auto t1 = high_resolution_clock::now();
    Collector::template generateGameTree<state, depth>(board);
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;

    /* Getting number of milliseconds as a double. */
    duration<double> seconds = t2 - t1;
    double mnps = (static_cast<double>(Collector::nodes) / 1000000) / seconds.count();

    std::cout << "Generated " << Collector::nodes << " nodes in " << ms_int.count() << "ms\n";
    std::cout << mnps << " M nps\n\n";
//    std::cout << gen.coll.captures << " captures\n" << gen.coll.checks << " checks" << std::endl;
}

using Collector = MoveCollectorStandard<false>;

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    constexpr State state = STARTSTATE;
    Board board = STARTBOARD;

    time_movegen<Collector, state, 3>(board);


//    for(int i = 0; i < gen.coll.follow_positions.size(); i++) {
//        printMove<true>(gen.coll.moves.at(i));
//        std::cout << "\t\t" << gen.coll.follow_positions.at(i) << std::endl;
//    }


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
