# Dory - Chess Move Generation Engine

Dory is a powerful chess move generation engine, written in C++20 capable of enumerating legal moves at a speed peaking over **1 Billion positions per second**. It is easy to customize and highly performant, making it a great core for chess engines, chess data analysis or any other chess projects!

## Main Advantages

- **Customization** [[more](#Move-Collectors)]</br>
  The move enumeration behavior can be adjusted with custom Move Collectors to serve different purposes. This allows for and efficient implementation of advanced search techniques.
- **Efficiency** [[more](#Performance)]</br>
  From the starting position the engine produces positions at roughly 240 million nodes/sec, with increasing speed to over 1 billion node/sec on emptier boards using only a single thread.
- **Correctness** [[more](#Perft Testing)]</br>
  The implementation is thoroughly Perft tested to ensure its correctness, saving the developer from the tedious task of fully bug-checking their engines implementation.

### Move Collectors

Dory allows the developer to customize the move enumeration process and specify how to handle the discovery of the next position. For example one might want to just count the positions, print them or save them in a list. This is realized by implementing a custom Move Collectors to give very fine-grained control of the move generation.

Crucially, this allows for advanced techniques such as pre-selecting or even pre-evaluating positions before ever saving them in a list or further processing them. This opens the door for creative 

### Performance

The implementation relies heavily on compile time programming, making the execution significantly faster at runtime. From the starting position the legal moves can enumerated at a speed of roughly 240 Million nodes per second. On emptier boards the speed can increase to surpass the mark of 1 Billion nodes per second:

See for example this endgame position at depth 6:

```
./Dory "8/pp2k2p/2nppn2/2p5/1P3N2/3P2N1/P1PK3P/8 w - - 0 1" 6
Generated 250923676 nodes in 243ms
1029.1 M nps
```

### Perft Testing

A standard way of verifying the correctness of a chess engines move generation algorithm is via [Perft Testing](https://www.chessprogramming.org/Perft). We count the number of nodes that are reachable from a given position at a certain depth and compare that to the consensus number computed by other trusted engines, such as Stockfish.

To test the implementation and run the full test suites, build the program as described [below](#Installation) and run the `tester`:

```bash
./tester
[==========] Running 21 tests from 2 test suites.
...
[  PASSED  ] 21 tests.
```

## Installation

Make sure you have recent versions of Cmake and of a C++ compiler installed. Then, to build run the following commands from the root directory

```bash
mkdir build
cmake -G Ninja -B build
cmake --build build --target Dory
```

Note: If you also wish to run the [test suites](#Perft Testing), omit the `--target Dory` flag in the last command.

### Usage

To just get the number of legal moves from a given position, first build the program as described above and then switch to the build directory and run

```bash
./Dory "<FEN String>" <depth>
```

with the corresponding FEN string of the position (or `startpos` for the starting position). </br>
*Note that the FEN string has to be wrapped in quotes!*

For example, the nodes at depth 6 from the starting position can be generated like this:

```
./Dory startpos 6
Generated 119060324 nodes in 512ms
232.54 M nps
```

## References

This project is a successor of an earlier chess move generation project of mine which was written in Java. It is based on the same algorithm, but enhanced significantly with efficient compile-time programming.

- [Chessprogramming Wiki](https://www.chessprogramming.org) (Lots of super helpful information)
- [Gigantua ](https://github.com/Gigantua/Gigantua) (inspiration and some implementation details)

</br>

##### Feel free to fork the project and experiment with your own engine!
