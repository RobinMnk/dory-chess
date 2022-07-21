#include <iostream>
#include <chrono>

#include "movecollectors.h"
#include "fenreader.h"

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
    double mnps = (static_cast<double>(Collector::totalNodes) / 1000000) / seconds.count();

    std::cout << "Generated " << Collector::totalNodes << " totalNodes in " << ms_int.count() << "ms\n";
    std::cout << mnps << " M nps\n\n";
//    std::cout << gen.coll.captures << " captures\n" << gen.coll.checks << " checks" << std::endl;
}

using Collector = MoveCollectorStandard<false>;

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    constexpr State state = STARTSTATE;
    Board board = STARTBOARD;

//    constexpr State nS = getNextState<state>();

//    Board nextBoard = STARTBOARD
//            .getNextBoard<state, Piece::Pawn, MoveFlag::Silent>(sqBB("c2"), sqBB("c3"))      // d2 - d3
//            .getNextBoard<nS, Piece::Pawn, MoveFlag::Silent>(sqBB("d7"), sqBB("d6"))         // c7 - c6
//            .getNextBoard<state, Piece::Queen, MoveFlag::Silent>(sqBB("d1"), sqBB("a4"));     // B c1 - d2
////            .getNextBoard<nS, Piece::Queen, MoveFlag::Silent>(sqBB("d8"), sqBB("h4"));          // Q d8 - a5
//
//    print_board(nextBoard);

//    time_movegen<Collector, state, 5>(board);
//
//    MoveCollectorDivide::print();


    Board b = Utils::loadFEN("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");

    Utils::print_board(b);

//    std::string s{};
//
//    std::cin >> s;

//    constexpr auto pr = loadFEN(s);
//    constexpr State st = pr.first;


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
