#include <iostream>
#include <ranges>

#include "board.h"
#include "engine/engine.h"
#include "engine/monte_carlo.h"
#include "movecollectors.h"
#include "fenreader.h"

void timeEvaluation(const Board& board, const State state, int depth) {
    EngineMC::reset();
    auto t1 = std::chrono::high_resolution_clock::now();
//    auto [eval, line] = EngineMC::searchDepth(board, state, depth);
    auto [ev, ln] = EngineMC::iterativeDeepening(board, state, depth);
    auto t2 = std::chrono::high_resolution_clock::now();

    auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

//    std::cout << "Evaluation: " << eval << std::endl;

    std::chrono::duration<double> seconds = t2 - t1;

    std::cout << "Searched " << EngineMC::nodesSearched << " nodes in " << ms_int.count() << "ms";

    double knps = (static_cast<double>(EngineMC::nodesSearched) / 1000) / seconds.count();
    if (knps < 1000) {
        std::cout << "\t\t(" << knps << " k nps)\n\n";
    } else {
        std::cout << "\t\t(" << (knps / 1000) << " M nps)\n\n";
    }

//    return {eval, line};
}

void monteCarlo(const Board& board, const State state) {
    auto fen = MonteCarlo::simulateGame(board, state);
    std::cout << "FEN: \n" << fen << std::endl;
}


template<State state, int depth>
void enumerateMoves(const Board& board) {
    MoveCollectors::nodes.resize(depth + 1);

    auto t1 = std::chrono::high_resolution_clock::now();
    MoveCollectors::PerftCollector<depth>::template generateGameTree<state>(board);
    auto t2 = std::chrono::high_resolution_clock::now();

    auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

    std::chrono::duration<double> seconds = t2 - t1;

    unsigned long long nodes = MoveCollectors::nodes.at(1);

    std::cout << "Generated " <<  nodes << " nodes in " << ms_int.count() << "ms";

    double knps = (static_cast<double>(nodes) / 1000) / seconds.count();
    if (knps < 1000) {
        std::cout << "\t\t(" << knps << " k nps)\n\n";
    } else {
        std::cout << "\t\t(" << (knps / 1000) << " M nps)\n\n";
    }
}


struct Runner {
    template<State state, int depth>
    static void main(const Board& board) {
//        auto [ev, ln] = timeEvaluation(board, state, 1);

        timeEvaluation(board, state, 4);

//        std::cout << "Lines:" << std::endl;

        for(auto& [line, eval]: EngineMC::bestLines) {
            Utils::printLine(line, eval);
        }
//
//        std::cout << "Best Move(s) " << std::endl;
////        printLine(ln, ev);
//        for (auto& move: EngineMC::topLevelLegalMoves()) {
//            Utils::printMove(move.second);
//        }

        std::cout << "\nTable lookups:\t" << EngineMC::trTable.lookups << std::endl;
        std::cout << "Table size:\t" << EngineMC::trTable.size() << " kB" << std::endl;
        std::cout << "Searched " << EngineMC::nodesSearched << " nodes";


//        monteCarlo(board, state);
    }
};

int main() {
    std::string fen, depth_str, num_lines_str;
    std::getline(std::cin, fen);
    std::getline(std::cin, depth_str);
    int depth = static_cast<int>(std::strtol(depth_str.c_str(), nullptr, 10));
    std::getline(std::cin, num_lines_str);
    auto num_lines = static_cast<unsigned int>(std::strtol(num_lines_str.c_str(), nullptr, 10));

    PieceSteps::load();
    Zobrist::init();
    NUM_LINES = num_lines;



    auto [board, state] = Utils::loadFEN(fen);
    timeEvaluation(board, state, depth);

    std::cout << "\nTable lookups:\t" << EngineMC::trTable.lookups << std::endl;
    std::cout << "Table size:\t" << EngineMC::trTable.size() << " kB" << std::endl;
    std::cout << "Searched " << EngineMC::nodesSearched << " nodes";

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
