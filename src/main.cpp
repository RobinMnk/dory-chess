#include <iostream>

#include "movecollectors.h"
#include "fenreader.h"

using Collector = MoveCollectors::LimitedDFS<false, true>;
using Divide = MoveCollectors::Divide;

struct Runner {
    template<State s>
    static void main(Board& b) {

        constexpr State ns = getNextState<s>();

        Board nb = b.template getNextBoard<s, Piece::King, MoveFlag::Silent>(newMask(Utils::sqId("e1")), newMask(Utils::sqId("f2")))
                .template getNextBoard<ns, Piece::Pawn, MoveFlag::PawnDoublePush>(newMask(Utils::sqId("a7")), newMask(Utils::sqId("a5")))
                .template getNextBoard<s, Piece::King, MoveFlag::Silent>(newMask(Utils::sqId("f2")), newMask(Utils::sqId("g3")))
                .template getNextBoard<ns, Piece::Bishop, MoveFlag::Silent>(newMask(Utils::sqId("e7")), newMask(Utils::sqId("d6")));

        Utils::time_movegen<Divide, s, 1>(nb);
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
