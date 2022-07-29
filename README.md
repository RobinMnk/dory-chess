# Chess (Move Generation) Engine

A work-in-progress chess engine, written in C++20. So far, the main focus of the project lies on optimizing the move generation, which is currently peaking around **320 Million nodes per second**.

### Move Collectors

The engines main advantage is its modularity. It uses the concept of Move Collectors to handle the behavior when discovering a new position. Upon finding a new move the Move Collector specifies whether to just count the moves, save them to a list or print them to stdout. It is therefore very easy and straightforward to adapt the engines behavior and incorporate it in future work.