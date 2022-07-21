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

struct Runner {
    template<State state>
    static void main(Board& board) {
        time_movegen<Collector, state, 7>(board);
    }
};

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    Utils::loadFEN<Runner>("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");

    return 0;
}
