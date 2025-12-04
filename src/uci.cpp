//
// Created by Robin on 13.07.2024.
//

#include <string>
#include <iostream>

#include "dory.h"

class UciManager {
    enum UciStatus{ IDLE = 0, NEW_GAME, READY, RUNNING };
    UciStatus status{IDLE};

    Dory::Board board{};
    Dory::Engine dory{};
    bool whiteToMove{true};

    static void respond(std::string_view resp) {
        std::cout << resp << std::endl;
    }

    void processCommand(std::string_view command) {
        std::stringstream stream(command.data());
        std::string segment;
        std::vector<std::string> seglist;
        while(std::getline(stream, segment, ' ')) seglist.push_back(segment);

        auto& cmd = seglist.at(0);

        if(cmd == "uci") {
            respond("id name Dory");
            respond("id author Robin Münk");
            respond("option name Hash type spin default 16 min 1 max 1024");
            respond("uciok");
        }
        else if(cmd == "ucinewgame") {
            status = NEW_GAME;
        }
        else if(cmd == "isready") {
            respond("readyok");
        }
        else if (cmd == "setoption") {
            if(seglist.at(2) == "Hash") {
                size_t value = std::stoi(seglist.at(4));
                dory.setHashTableSize(value);
            }
        }
        else if(cmd == "ping") {
            std::cout << "pong " << seglist.at(1) << std::endl;
        }
        else if(cmd == "position") {
            if(seglist.at(1) == "startpos") { board = Dory::STARTBOARD; whiteToMove = true; }
            else {
                auto [b, w] = Dory::Utils::parseFEN(seglist, 2);
                board = b;
                whiteToMove = w;
            }

            unsigned int ix = 2;
            while(ix < seglist.size() && seglist.at(ix) != "moves") ix++;
            ix++;

            while(ix < seglist.size()) {
                Dory::Move move = Dory::Utils::parseMove(board, whiteToMove, seglist.at(ix));
                board.makeMove(move, whiteToMove);
                whiteToMove = !whiteToMove;
                ++ix;
            }
            status = READY;
        }
        else if (cmd == "go") {
            status = RUNNING;
            auto [eval, line] = dory.searchTime(board, 1000, whiteToMove);
            std::cout << "bestmove " << Dory::Utils::moveFullNotation(line.back()) << std::endl;
            status = READY;
        }
    }

public:
    void run() {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);

        std::string cmd;
        while(cmd != "quit") {
            std::getline(std::cin, cmd, '\n');
            if(!cmd.empty()) processCommand(cmd);
        }
    }
};

int main() {
    UciManager uci;
    uci.run();
}