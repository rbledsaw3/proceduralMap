#include <iterator>
#include <memory>
#include <vector>
#include <utility>
#include <random>
#include <queue>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <iostream>

enum class Tile {
    Unexplored,
    Room,
    Hallway,
    Door,
    SecretDoor
};

constexpr size_t WIDTH = 100;
constexpr size_t HEIGHT = 100;
constexpr size_t MIN_PARTITION_SIZE = 20;

struct Door {
    size_t x;
    size_t y;
    bool isSecret;

    Door(size_t x, size_t y, bool isSecret): x(x), y(y), isSecret(isSecret) {}
};

struct Room {
    size_t x;
    size_t y;
    size_t width;
    size_t height;
    std::vector<Door> doors;

    // this member function checks if a given point is inside the room
    bool contains(size_t pointX, size_t pointY) const {
        return pointX >= x && pointX < x + width && pointY >= y && pointY < y + height;
    }
};

struct Partition {
    size_t x, y, width, height;

    Partition (size_t x, size_t y, size_t width, size_t height): x(x), y(y), width(width), height(height) {}

    std::pair<Partition, Partition> split() {
        bool splitHorizontally = (rand() % 2 == 0) ? true : false;
        if (width > height && static_cast<double>(width) / height >= 1.25)
            splitHorizontally = false;
        else if (height > width && static_cast<double>(height) / width >= 1.25)
            splitHorizontally = true;

        size_t max = (splitHorizontally ? height : width) - MIN_PARTITION_SIZE;
        if (max <= MIN_PARTITION_SIZE) 
            max = MIN_PARTITION_SIZE + 1;

        size_t split = rand() % max;

        if (split <= MIN_PARTITION_SIZE) 
            split = MIN_PARTITION_SIZE;

        if (splitHorizontally) {
            Partition p1(x, y, width, split);
            Partition p2(x, y + split, width, height - split);
            return std::make_pair(p1, p2);
        } else {
            Partition p1(x, y, split, height);
            Partition p2(x + split, y, width - split, height);
            return std::make_pair(p1, p2);
        }
    }

    Room createRoom() const{
        std::random_device rd;
        std::mt19937 generator(rd());

        std::uniform_int_distribution<size_t> distX(x + 1, x + width - 2);
        std::uniform_int_distribution<size_t> distY(y + 1, y + height - 2);
        size_t roomX = distX(generator);
        size_t roomY = distY(generator);
        std::uniform_int_distribution<size_t> distWidth(1, x + width - roomX - 1);
        std::uniform_int_distribution<size_t> distHeight(1, y + height - roomY - 1);
        size_t roomWidth = distWidth(generator);
        size_t roomHeight = distHeight(generator);

        return Room{roomX, roomY, roomWidth, roomHeight, {}};
    }
};

struct Edge {
    Room* room1;
    Room* room2;
    double weight;

    Edge(Room* room1, Room* room2): room1(room1), room2(room2) {
        weight = std::sqrt(std::pow(room1->x - room2->x, 2) + std::pow(room1->y - room2->y, 2));
    }

    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

int main() {
    srand(time(NULL));
    std::vector<std::vector<Tile>> map(WIDTH, std::vector<Tile>(HEIGHT, Tile::Unexplored));

    Partition initialPartition(0, 0, WIDTH, HEIGHT);

    std::vector<Room> rooms;

    std::queue<Partition> partitions;
    partitions.push(initialPartition);

    while (!partitions.empty()) {
        Partition currentPartition = partitions.front();
        partitions.pop();
        
        std::cout << "Processing partition: x = " << currentPartition.x
                  << ", y = " << currentPartition.y
                  << ", width = " << currentPartition.width
                  << ", height = " << currentPartition.height << std::endl;
        if (currentPartition.width / 2 >= MIN_PARTITION_SIZE || currentPartition.height / 2 >= MIN_PARTITION_SIZE) {
            auto subpartitions = currentPartition.split();
            partitions.push(subpartitions.first);
            partitions.push(subpartitions.second);

            std::cout << "Splitting partition into: " << std::endl;
            std::cout << "Partition 1: x = " << subpartitions.first.x 
                << ", y = " << subpartitions.first.y
                << ", width = " << subpartitions.first.width
                << ", height = " << subpartitions.first.height << std::endl;
            std::cout << "Partition 2: x = " << subpartitions.second.x 
                << ", y = " << subpartitions.second.y
                << ", width = " << subpartitions.second.width
                << ", height = " << subpartitions.second.height << std::endl;
        } else {
            Room room = currentPartition.createRoom();

            for (size_t i = 0; i < room.width && room.x + i < WIDTH; ++i) {
                for (size_t j = 0; j < room.height && room.y + j < HEIGHT; ++j) {
                    map[room.x + i][room.y + j] = Tile::Room;
                }
            }

            rooms.push_back(room);
        }
    }

    std::vector<Edge> edges;
    for (size_t i = 0; i < rooms.size(); ++i) {
        for (size_t j = i + 1; j < rooms.size(); ++j) {
            edges.push_back(Edge(&rooms[i], &rooms[j]));
        }
    }

    std::sort(edges.begin(), edges.end());

    std::vector<Edge> hallways;
    std::map<Room*, std::shared_ptr<std::set<Room*>>> connectedComponents;
    for (Room& room : rooms) {
        connectedComponents[&room] = std::make_shared<std::set<Room*>>();
        connectedComponents[&room]->insert(&room);
    }
    for (Edge& edge : edges) {
        if (*connectedComponents[edge.room1] != *connectedComponents[edge.room2]) {
            hallways.push_back(edge);
            std::shared_ptr<std::set<Room*>> merged = std::make_shared<std::set<Room*>>(*connectedComponents[edge.room1]);
            merged->insert(connectedComponents[edge.room2]->begin(), connectedComponents[edge.room2]->end());
            for (Room* room : *merged) {
                connectedComponents[room] = merged;
            }
        }
    }

    for (Edge& edge : hallways) {
        Room* room1 = edge.room1;
        Room* room2 = edge.room2;

        size_t room1CenterX = room1->x + room1->width / 2;
        size_t room2CenterX = room2->x + room2->width / 2;
        size_t room1CenterY = room1->y + room1->height / 2;
        size_t room2CenterY = room2->y + room2->height / 2;

        size_t hallwayStartX = std::min(room1CenterX, room2CenterX);
        size_t hallwayEndX = std::max(room1CenterX, room2CenterX);
        for (size_t x = hallwayStartX; x <= hallwayEndX && x < WIDTH; ++x) {
            if(room1CenterY < HEIGHT) map[x][room1CenterY] = Tile::Hallway;
        }

        size_t hallwayStartY = std::min(room1CenterY, room2CenterY);
        size_t hallwayEndY = std::max(room1CenterY, room2CenterY);
        for (size_t y = hallwayStartY; y <= hallwayEndY && y < HEIGHT; ++y) {
            if(room2CenterX < WIDTH) map[room2CenterX][y] = Tile::Hallway;
        }
    }

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1,2);
    std::uniform_int_distribution<int> secretDistribution(1,5);

    for (Room& room : rooms) {
        int numDoors = (room.width / 3 + room.height / 3) * distribution(generator);

        for (int i = 0; i < numDoors; ++i) {
            // Randomly decide the door position and whether it's a secret door
            bool isSecret = (secretDistribution(generator) == 1);
            size_t doorX, doorY;

            if (i % 4 == 0) {
                // Place the door on the north wall
                doorX = room.x + distribution(generator) % room.width;
                doorY = room.y;
            } else if (i % 4 == 1) {
                // Place the door on the south wall
                doorX = room.x + distribution(generator) % room.width;
                doorY = room.y + room.height - 1;
            } else if (i % 4 == 2) {
                // Place the door on the west wall
                doorX = room.x;
                doorY = room.y + distribution(generator) % room.height;
            } else {
                // Place the door on the east wall
                doorX = room.x + room.width - 1;
                doorY = room.y + distribution(generator) % room.height;
            }

            // Bounds checking
            if (doorX >= WIDTH - 1) doorX = WIDTH - 2;
            if (doorY >= HEIGHT - 1) doorY = HEIGHT - 2;

            // Add the door to the room and the map
            room.doors.push_back(Door(doorX, doorY, isSecret));
            if (isSecret) {
                map[doorX][doorY] = Tile::SecretDoor;
            } else {
                map[doorX][doorY] = Tile::Door;
            }
        }
    }


    // Printing out the map
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            switch (map[x][y]) {
                case Tile::Unexplored:
                    std::cout << ' ';
                    break;
                case Tile::Room:
                    std::cout << '.';
                    break;
                case Tile::Hallway:
                    std::cout << '#';
                    break;
                case Tile::Door:
                    std::cout << 'D';
                    break;
                case Tile::SecretDoor:
                    std::cout << 'S';
                    break;
            }
        }
        std::cout << '\n';
    }
            
    return 0;
}
