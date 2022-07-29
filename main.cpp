#include <iostream>

#include "movecollectors.h"
#include "fenreader.h"
#include "board.h"

using Collector = MoveCollectors::LimitedDFS<false, false>;
using Divide = MoveCollectors::Divide;

struct Runner {
    template<State s>
    static void main(Board& b) {

//        constexpr State ns = getNextState<s>();
//        Board nb = b.template getNextBoard<s, Piece::Pawn, MoveFlag::Silent>(newMask(Utils::sqId("a2")), newMask(Utils::sqId("a3")))
//                    .template getNextBoard<ns, Piece::Pawn, MoveFlag::Silent>(newMask(Utils::sqId("a7")), newMask(Utils::sqId("a6")));

        Utils::time_movegen<Collector , s, 4>(b);
        Divide::print();
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
