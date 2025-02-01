//
// Created by robin on 25.01.2025.
//

#ifndef DORY_ENGINE_H
#define DORY_ENGINE_H

#include "search.h"

namespace Dory{

    class Engine {
        MoveGenerator<Search::Searcher> mg;
        MoveGenerator<Search::Searcher, true> mgq;
        Search::Searcher searcher;


        Engine() {




        }


    };
}




#endif //DORY_ENGINE_H
