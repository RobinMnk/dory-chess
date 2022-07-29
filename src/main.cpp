#include <iostream>

#include "movecollectors.h"
#include "fenreader.h"

using Collector = MoveCollectors::LimitedDFS<false, true>;
using Divide = MoveCollectors::Divide;

struct Runner {
    template<State s>
    static void main(Board& b) {
        Utils::time_movegen<Collector , s, 1>(b);
    }
};

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    Board b = STARTBOARD;
    Runner::template main<STARTSTATE>(b);

//    Utils::loadFEN<Runner>("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

    return 0;
}
