//
// Created by robin on 29.08.2023.
//

#ifndef DORY_RANDOM_H
#define DORY_RANDOM_H

#include <random>
using RNG = std::uniform_int_distribution<std::mt19937::result_type>;

namespace Utils {
    class Random {
        std::mt19937 rng;

    public:
        Random() {
            std::random_device dev;
            rng = std::mt19937(dev());
        }

        size_t randomNumberInRange(size_t lo, size_t hi) {
            RNG dist6(lo, hi); // distribution in range [1, 6]
            return dist6(rng);
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
            size_t num = randomNumberInRange(0, INT32_MAX);
            double res = static_cast<double>(num) / static_cast<double>(INT32_MAX);
            return res <= p;
        }

    };
}

#endif //DORY_RANDOM_H
