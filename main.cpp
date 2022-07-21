#include <iostream>

#include "movecollectors.h"
#include "fenreader.h"

using Collector = MoveCollectorStandard<false>;

struct Runner {
    template<State state>
    static void main(Board& board) {
        Utils::time_movegen<Collector, state, 7>(board);
    }
};

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    Utils::loadFEN<Runner>("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");

    return 0;
}
