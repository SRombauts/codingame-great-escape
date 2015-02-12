/**
 * @file    Main.cpp
 * @brief   My attempt at the Multiplayer CodinGame "The Great Escape".
 *
 * Copyright (c) 2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// TODO(SRombauts) add a structure for orientation/direction data
// 'H'orizontal or 'V'ertical orientation

// TODO(SRombauts) add a class for commands (+text messages)
// LEFT, RIGHT, UP, DOWN
// or "putX putY putOrientation"

/// coordinates
struct Coords {
    size_t x;   ///< x-coordinate (column)
    size_t y;   ///< y-coordinate (row/line)
};

/// player data
struct Player {
    /// Vector of players
    typedef std::vector<Player> Vector;

    size_t id;          ///< id (orientation) of the player
    Coords coords;      ///< coordinates of the player
    size_t wallsLeft;   ///< number of walls available for the player
    bool   bIsAlive;    ///< true while the player is alive
};

/// wall data
struct Wall {
    Coords coords;      ///< coordinates of the upper left corner of the wall
    char   orientation; ///< 'H'orizontal or 'V'ertical orientation
};

/// pathfinding data for a cell of the grid
struct Cell {
    double weight;      ///< weighted distance toward the destination
    size_t direction;   ///< direction of the shortest/best path
};

/// templated 2D matrix of generic TElement
template <typename TElement>
class Matrix {
public:
    /**
    * ctor allocating the matrix to the specified size
    *
    * @param aWidthX    Nb of columns (X coordinate)
    * @param aHeightY   Nb of lines   (Y coordinate)
    */
    Matrix(const size_t aWidthX, const size_t aHeightY) {
        matrix.resize(aWidthX);
        for (auto& line : matrix) {
            line.resize(aHeightY);
        }
    }

    /// getter for cell at [X, Y] (const reference) uses vector::at() with safety check : can throw std::out_of_range
    const TElement& get(const size_t aX, const size_t aY) const {
        return matrix.at(aX).at(aY); // return matrix[aX][aY];
    }
    /// "setter" for cell at [X, Y] (reference) uses vector::at() with safety check : can throw std::out_of_range
    TElement& set(const size_t aX, const size_t aY) {
        return matrix.at(aX).at(aY);
    }

    /// debug:
    void dump() const {
        std::cerr << "  |  ";
        for (size_t x = 0; x < matrix.size(); ++x) {
            std::cerr << x << "  |  ";
        }
        std::cerr << std::endl;
        for (size_t y = 0; y < matrix[0].size(); ++y) {
            std::cerr << y << " | ";
            for (size_t x = 0; x < matrix.size(); ++x) {
                std::cerr << matrix[x][y].weight << " " << matrix[x][y].direction << " | ";
            }
            std::cerr << std::endl;
        }
    }

private:
    /// Vector of Elements
    typedef std::vector<TElement>   Vector1D;
    /// Matrix as a vector of vectors of Elements
    typedef std::vector<Vector1D>   Vector2D;

    Vector2D matrix;    ///< Matrix as a vector of vectors of Elements
};

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 *
 * @return 0
 */
int main() {
    size_t w; // width of the board
    size_t h; // height of the board
    size_t playerCount; // number of players (2 or 3)
    size_t myId; // id of my player (0 = 1st player, 1 = 2nd player, ...)
    std::cin >> w >> h >> playerCount >> myId; std::cin.ignore();

    size_t turn = 0;

    // TODO(SRombauts) test a pathfinding algorithm using the grid
    Matrix<Cell> grid(w, h);

    // all players statuses
    Player::Vector players(playerCount);
    Player& MySelf = players[myId];

    // game loop
    while (1) {
        for (size_t id = 0; id < playerCount; ++id) {
            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            size_t wallsLeft; // number of walls available for the player
            std::cin >> x >> y >> wallsLeft; std::cin.ignore();

            players[id].id          = id; // redundant, but useful for debug
            players[id].wallsLeft   = wallsLeft;
            // if player still playing, add it to the list of players
            if ((x >= 0) && (y >= 0)) {
                players[id].coords.x = static_cast<size_t>(x);
                players[id].coords.y = static_cast<size_t>(y);
                players[id].bIsAlive = true;

                // set the player on the grid
                grid.set(players[id].coords.x, players[id].coords.y);

                // debug:
                if (id == myId) {
                    std::cerr << "me(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                } else {
                    std::cerr << "player(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                }
            } else {
                players[id].bIsAlive = false;
                std::cerr << "dead(" << id << "): [" << x << ", " << y << "]\n";
            }
        }

        // debug:
        grid.dump();

        size_t wallCount; // number of walls on the board
        std::cin >> wallCount; std::cin.ignore();
        for (size_t wall = 0; wall < wallCount; ++wall) {
            int wallX; // x-coordinate of the wall
            int wallY; // y-coordinate of the wall
            std::string wallOrientation; // wall orientation ('H' or 'V')
            std::cin >> wallX >> wallY >> wallOrientation; std::cin.ignore();
            std::cerr << "wall(" << wall << "): [" << wallX << ", " << wallY << "]\n";
        }

        // Write an action using std::cout. DON'T FORGET THE "<< std::endl"
        // To debug: std::cerr << "Debug messages..." << std::endl;

        ++turn;
        std::cerr << "turn " << turn << std::endl;

        // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
        std::cout << "RIGHT" << std::endl;
    }

    return 0;
}
