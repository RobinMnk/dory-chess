//
// Created by Robin on 29.08.2023.
//

#ifndef DORY_RANDOM_H
#define DORY_RANDOM_H

#include <random>
#include <ctime>

using RNG_INT = std::uniform_int_distribution<std::mt19937::result_type>;
using RNG_REAL = std::uniform_real_distribution<>;

namespace Utils {
    class Random {
        std::mt19937 rng;

    public:
        Random() {
            std::random_device dev;
            rng = std::mt19937(dev());
            rng.seed(std::time(nullptr));
//            rng.seed(4263489725);
        }

        void setSeed(size_t s) {
            if(s) rng.seed(s);
        }

        unsigned int randomNumberInRange(size_t lo, size_t hi) {
            RNG_INT dist(lo, hi);
            return dist(rng);
        }

        template<typename T>
        T randomElementOf(std::vector<T>&& list) {
            size_t index = randomNumberInRange(0, list.size()-1);
            return list.at(index);
        }

        template<typename T>
        T randomElementOf(std::vector<T>& list) {
            size_t index = randomNumberInRange(0, list.size()-1);
            return list.at(index);
        }

        bool bernoulli(double p) {
            RNG_REAL dist(0., 1.);
            return dist(rng) <= p;
        }
    };
}

#endif //DORY_RANDOM_H
