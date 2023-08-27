#include <iostream>

#include "board.h"
#include "fenreader.h"
#include "engine/engine_move_collector.h"

//using Collector = MoveCollectors::LimitedDFS<false, false>;

struct Runner {
    template<State state, int depth>
    static void main(Board& board) {
//        Utils::time_movegen<Collector, state, depth>(board);
        auto eval = EngineMC::template generateGameTree<state>(board, 6);
        std::cout << "FINAL EVALUATION: " << eval << std::endl;
//        std::cout << "Best Move: " << Utils::moveName<state.whiteToMove>(EngineMC::bestMove) << std::endl;

        std::cout << "Best Move(s) " << std::endl;
//        Utils::printMoveList(EngineMC::line.);
        for (auto& move: EngineMC::bestMoves) {
//            if (move.from + move.to == 0) break;
            Utils::printMove(move);
        }
    }
};

int main(int argc, char* argv[]) {
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
