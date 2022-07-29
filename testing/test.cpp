//
// Created by robin on 29.07.2022.
//

#include <gtest/gtest.h>

#include "../src/movecollectors.h"
#include "../src/fenreader.h"

using Collector = MoveCollectors::PerftCollector;

template<int depth>
struct Runner {
    template<State state>
    static void main(Board& board) {
        Collector::template generateGameTree<state, depth>(board);
    }
};

TEST(NodeCounts, StartingPosition) {
    PieceSteps::load();
    Board board = STARTBOARD;

    std::vector<unsigned long long> ground_truth{
        1, 20, 400, 8902, 197281, 4865609, 119060324, 3195901860
    };

    Runner<6>::template main<STARTSTATE>(board);

    std::cout << Collector::nodes.size() << std::endl;

    for(int i{1}; i <= 6; i++) {
        ASSERT_EQ(Collector::nodes.at(i), ground_truth.at(i));
    }
}