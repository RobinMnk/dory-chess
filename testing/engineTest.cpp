//
// Created by robin on 22.12.2023.
//

#include <gtest/gtest.h>
#include <fstream>
#include "../src/fenreader.h"
#include "../src/engine/engine.h"
#include "../src/engine/monte_carlo.h"

using uLong = unsigned long long;

const std::array<std::string, 2> puzzleFiles{
    "../resources/puzzles2000.txt",
    "../resources/puzzlesSacrifices.txt"
};

std::vector<std::pair<std::string, std::string>> loadTestCases(int index, int limit) {
    /// Setup Engine
    PieceSteps::load();
    Zobrist::init();

    std::vector<std::pair<std::string, std::string>> testCases;
    std::string fen, solution;
    std::ifstream file (puzzleFiles.at(index));
    int count = 0;
    if (file.is_open()) {
        while(count++ <= limit && std::getline(file, fen)) {
            std::getline(file, solution);
            testCases.emplace_back(fen, solution);
        }
        file.close();
    }
    return testCases;
}

typedef std::pair<std::string, std::string> TestParam;

class EngineTest : public testing::TestWithParam<TestParam> {};

TEST_P(EngineTest, NegamaxEngine) {
    auto [fen, solution] = GetParam();
    auto [board, state] = Utils::loadFEN(fen);

    auto [_, line] = EngineMC::iterativeDeepening(board, state, 8);
    std::string output = Utils::moveNameShortNotation(line.back());

    ASSERT_EQ(output, solution);
}

//TEST_P(EngineTest, MonteCarlo) {
//    auto [fen, solution] = GetParam();
//    auto [board, state] = Utils::loadFEN(fen);
//
//    Move move = MCTS(board, state, 1000000);
//    std::string output = Utils::moveNameShortNotation(move);
//
//    ASSERT_EQ(output, solution);
//}

INSTANTIATE_TEST_SUITE_P(
        Puzzles2000,
        EngineTest,
        testing::ValuesIn(loadTestCases(0, 50))
);

INSTANTIATE_TEST_SUITE_P(
        PuzzlesSacrifices,
        EngineTest,
        testing::ValuesIn(loadTestCases(1, 50))
);