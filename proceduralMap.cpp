#include <iostream>
#include <vector>
#include <random>

constexpr size_t WIDTH = 50;
constexpr size_t HEIGHT = 50;
constexpr size_t ITERATIONS = 5;

enum class Tile {
    Wall,
    Floor
};

// Function to get the count of surrounding walls
int getSurroundingWallCount(size_t x, size_t y, std::vector<std::vector<Tile>>& map) {
    int wallCount = 0;
    for (size_t neighborX = x - 1; neighborX <= x + 1; neighborX++) {
        for (size_t neighborY = y - 1; neighborY <= y + 1; neighborY++) {
            if (neighborX < WIDTH && neighborY < HEIGHT) {
                if (neighborX != x || neighborY != y) {
                    wallCount += map[neighborX][neighborY] == Tile::Wall;
                }
            } else {
                wallCount++;
            }
        }
    }
    return wallCount;
}

// Generate the initial map
void generateMap(std::vector<std::vector<Tile>>& map) {
    std::random_device rd;
    std::uniform_int_distribution<int> distribution(0, 100);

    for (size_t x = 0; x < WIDTH; x++) {
        for (size_t y = 0; y < HEIGHT; y++) {
            map[x][y] = distribution(rd) < 45 ? Tile::Wall : Tile::Floor;
        }
    }
}

// Use cellular automata to shape the map
void doSimulationStep(std::vector<std::vector<Tile>>& map) {
    std::vector<std::vector<Tile>> newMap = map;

    for (size_t x = 0; x < map.size(); x++) {
        for (size_t y = 0; y < map[0].size(); y++) {
            int nbs = getSurroundingWallCount(x, y, map);
            if (map[x][y] == Tile::Wall) {
                newMap[x][y] = nbs >= 4 ? Tile::Wall : Tile::Floor;
            } else {
                newMap[x][y] = nbs > 5 ? Tile::Wall : Tile::Floor;
            }
        }
    }

    map = newMap;
}

// Print the map to the console
void printMap(std::vector<std::vector<Tile>>& map) {
    for (size_t x = 0; x < map.size(); x++) {
        for (size_t y = 0; y < map[0].size(); y++) {
            std::cout << (map[x][y] == Tile::Wall ? "#" : ".");
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<Tile>> map(WIDTH, std::vector<Tile>(HEIGHT, Tile::Wall));
    generateMap(map);

    for (size_t i = 0; i < ITERATIONS; i++) {
        doSimulationStep(map);
    }

    printMap(map);

    return 0;
}
