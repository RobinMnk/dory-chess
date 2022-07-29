#include <iostream>

#include "movecollectors.h"
#include "fenreader.h"

using Collector = MoveCollectors::LimitedDFS<false, false>;
using Divide = MoveCollectors::Divide;

struct Runner {
    template<State state>
    static void main(Board& board) {
        Utils::time_movegen<Divide, state, 1>(board);
        Divide::print();
    }
};

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

//    Board b = STARTBOARD;
//    Runner::template main<STARTSTATE>(b);

    Utils::loadFEN<Runner>("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ");

    return 0;
}
