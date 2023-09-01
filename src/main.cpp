#include <iostream>

#include "board.h"
#include "fenreader.h"
#include "engine/engine_move_collector.h"
#include "engine/monte_carlo.h"

//using Collector = MoveCollectors::LimitedDFS<false, false>;

template<State state>
double timeEvaluation(const Board& board, int depth) {
    auto t1 = std::chrono::high_resolution_clock::now();
    double eval = EngineMC::beginEvaluation(board, state, depth);
    auto t2 = std::chrono::high_resolution_clock::now();

    auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

    std::cout << "Evaluation: " << eval << "     (time " << ms_int.count() << "ms)\n";
    return eval;
}

struct Runner {
    template<State state, int depth>
    static void main(Board& board) {

//        MonteCarlo mc;
//        auto fen = mc.simulateGame(board, state);
//        std::cout << "FEN: \n" << fen << std::endl;

        timeEvaluation<state>(board, 2);
        std::cout << "Best Move(s) " << std::endl;
//        Utils::printMoveList(EngineMC::line.);
        for (auto& move: EngineMC::bestMoves) {
//            if (move.from + move.to == 0) break;
            Utils::printMove(move);
        }

        std::cout << EngineMC::nodesSearched << " nodes searched. " << std::endl;
        std::cout << "Table lookups: " << EngineMC:: lookups << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string fen, depth_str;
    std::getline(std::cin, fen);
    std::getline(std::cin, depth_str);
    int depth = static_cast<int>(std::strtol(depth_str.c_str(), nullptr, 10));

    PieceSteps::load();
    Zobrist::init();

    if (fen == "startpos" || fen == "start") {
        Utils::startingPositionAtDepth<Runner>(depth);
    } else {
        Utils::loadFEN<Runner>(fen, depth);
    }

    return 0;
}

int mainMoveEnumeration(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << R"(Usage: ./Dory "<FEN>" <Depth>)" << std::endl;
        return 1;
    }

    std::string_view fen{argv[1]};
    int depth = static_cast<int>(std::strtol(argv[2], nullptr, 10));

    PieceSteps::load();

    if (fen == "startpos" || fen == "start") {
        Utils::startingPositionAtDepth<Runner>(depth);
    } else {
        Utils::loadFEN<Runner>(fen, depth);
    }

    return 0;
}
