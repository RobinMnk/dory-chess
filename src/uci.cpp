//
// Created by Robin on 13.07.2024.
//

#include <string>
#include <iostream>

#include "dory.h"

class UciManager {
    enum UciStatus{ IDLE = 0, NEW_GAME, READY, RUNNING };
    UciStatus status{IDLE};

    Dory::Engine dory{};
    Dory::Board board{};
    bool whiteToMove{true};

    int wtime = 0, btime = 0, winc = 0, binc = 0;

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
        else if (cmd == "go") { // go btime 300000 wtime 298000 binc 2000 winc 2000
            // Parse time control arguments (all values are in milliseconds)
            for (size_t i = 1; i < seglist.size(); ++i) {
                const auto& arg = seglist.at(i);
                try {
                    if (arg == "wtime" && i + 1 < seglist.size()) {
                        wtime = std::stoi(seglist.at(i + 1));
                        i++;
                    } else if (arg == "btime" && i + 1 < seglist.size()) {
                        btime = std::stoi(seglist.at(i + 1));
                        i++;
                    } else if (arg == "winc" && i + 1 < seglist.size()) {
                        winc = std::stoi(seglist.at(i + 1));
                        i++;
                    } else if (arg == "binc" && i + 1 < seglist.size()) {
                        binc = std::stoi(seglist.at(i + 1));
                        i++;
                    }
                    // Other 'go' parameters like 'depth', 'infinite' would be handled here
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing time value: " << e.what() << std::endl;
                }
            }

            int time = whiteToMove ? wtime : btime;
            int inc = whiteToMove ? winc : binc;

            status = RUNNING;
            auto [eval, line] = dory.analyze(board, whiteToMove, time, inc);
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