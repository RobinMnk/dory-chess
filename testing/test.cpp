//
// Created by robin on 29.07.2022.
//

#include <gtest/gtest.h>

#include "../src/movecollectors.h"
#include "../src/fenreader.h"

using uLong = unsigned long long;
using Collector = MoveCollectors::PerftCollector;

struct Runner {
    template<State state, int depth>
    static void main(Board& board) {
        Collector::template generateGameTree<state, depth>(board);
    }
};

TEST(NodeCounts, StartingPosition) {
    PieceSteps::load();
    Board board = STARTBOARD;

    std::vector<uLong> ground_truth{
            1, 20, 400, 8'902, 197'281, 4'865'609, 119'060'324, 3'195'901'860
    };

    Runner::template main<STARTSTATE, 6>(board);

    std::cout << "Testing Starting Position ..." << std::endl;
    for(int i{1}; i <= 6; i++) {
        uLong expected = ground_truth.at(i);
        uLong output = Collector::nodes.at(i);
        ASSERT_EQ(output, expected);
        std::cout << "Depth " << i << " OK! (" << Collector::nodes.at(i) << " nodes)" << std::endl;
    }
    std::cout << "PASSED!\n" << std::endl;
}

template<int depth>
void runNodeCountTest(std::string_view fen, std::vector<uLong> ground_truth, std::string_view name) {
    Utils::loadFEN<Runner, depth>(fen);

    std::cout << "Testing " << name << " ..." << std::endl;
    for(int i{1}; i <= depth; i++) {
        uLong expected = ground_truth.at(i);
        uLong output = Collector::nodes.at(i);
        ASSERT_EQ(output, expected);
        std::cout << "Depth " << i << " OK! (" << Collector::nodes.at(i) << " nodes)" << std::endl;
    }
    std::cout << "PASSED!\n" << std::endl;
}

TEST(NodeCounts, TestPos2) {
    PieceSteps::load();
    runNodeCountTest<4>(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
        std::vector<uLong> {
            1, 48, 2'039, 97'862, 4'085'603, 193'690'690
        },
        "Position 2"
    );
}

TEST(NodeCounts, TestPos3) {
    PieceSteps::load();
    runNodeCountTest<6>(
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
        std::vector<uLong> {
            1, 14, 191, 2'812, 43'238, 674'624, 11'030'083, 178'633'661
        },
        "Position 3"
    );
}

TEST(NodeCounts, TestPos4w) {
    PieceSteps::load();
    runNodeCountTest<5>(
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        std::vector<uLong> {
            1, 6, 264, 9'467, 422'333, 15'833'292, 706'045'033
        },
        "Position 4w"
    );
}

TEST(NodeCounts, TestPos4b) {
    PieceSteps::load();
    runNodeCountTest<4>(
        "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
        std::vector<uLong> {
            1, 6, 264, 9'467, 422'333, 15'833'292, 706'045'033
        },
        "Position 4b"
    );
}

TEST(NodeCounts, TestPos5) {
    PieceSteps::load();
    runNodeCountTest<5>(
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        std::vector<uLong> {
            1, 44, 1486, 62'379, 2'103'487, 89'941'194
        },
        "Position 5"
    );
}

TEST(NodeCounts, TestPos6) {
    PieceSteps::load();
    runNodeCountTest<5>(
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        std::vector<uLong> {
                1, 46, 2'079, 89'890, 3'894'594, 164'075'551, 6'923'051'137
        },
        "Position 6"
    );
}