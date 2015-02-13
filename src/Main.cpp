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
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

// TODO(SRombauts) add a structure for orientation/direction data
// 'H'orizontal or 'V'ertical orientation

/**
 * @brief Send commands to the game thru standard output
 * 
 * Commands are string like LEFT, RIGHT, UP, DOWN or "putX putY putOrientation".
 */
class Command {
public:
    /// Move the player to the right of the board (x++)
    static void right(const char* apMessage) {
        std::cout << "RIGHT " << apMessage << std::endl;
    }
    /// Move the player to the left of the board (x--)
    static void left(const char* apMessage) {
        std::cout << "LEFT " << apMessage  << std::endl;
    }
    /// Move the player to the bottom of the board (y++)
    static void down(const char* apMessage) {
        std::cout << "DOWN " << apMessage  << std::endl;
    }
    /// Move the player to the top of the board (y--)
    static void up(const char* apMessage) {
        std::cout << "UP " << apMessage  << std::endl;
    }
    /// Put a new Wall to a specified location and orientation
    static void put(const size_t aX, const size_t aY, const char aOrientation, const char* apMessage) {
        std::cout << aX << aY << aOrientation << " " << apMessage  << std::endl;
    }
};

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
    /// Vector of walls
    typedef std::vector<Wall> Vector;

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
     * ctor allocating the matrix to the specified size (with default initialisation)
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

    /**
     * ctor allocating the matrix to the specified size with explicit initialisation
     *
     * @param aWidthX    Nb of columns (X coordinate)
     * @param aHeightY   Nb of lines   (Y coordinate)
     * @param aInitValue Initial Value of every elements
     */
    Matrix(const size_t aWidthX, const size_t aHeightY, const TElement& aInitValue) {
        matrix.resize(aWidthX);
        for (auto& line : matrix) {
            line.resize(aHeightY, aInitValue);
        }
    }

    /// width of the matrix (Nb of columns, X axis)
    size_t width() const {
        return matrix.size();
    }
    /// height of the matrix (Nb of lines, Y axis)
    size_t height() const {
        return matrix.at(0).size();
    }

    /// getter for cell at [X, Y] (const reference) uses vector::at() with safety check : can throw std::out_of_range
    const TElement& get(const size_t aX, const size_t aY) const {
        return matrix.at(aX).at(aY); // return matrix[aX][aY];
    }
    /// "setter" for cell at [X, Y] (reference) uses vector::at() with safety check : can throw std::out_of_range
    TElement& set(const size_t aX, const size_t aY) {
        return matrix.at(aX).at(aY);
    }

private:
    /// Vector of Elements
    typedef std::vector<TElement>   Vector1D;
    /// Matrix as a vector of vectors of Elements
    typedef std::vector<Vector1D>   Vector2D;

    Vector2D matrix;    ///< Matrix as a vector of vectors of Elements
};

/// debug: dump content of the Matrix of Cells
void dump(const Matrix<Cell>& aMatrix) {
    std::cerr << "  |  ";
    for (size_t x = 0; x < aMatrix.width(); ++x) {
        std::cerr << x << "  |  ";
    }
    std::cerr << std::endl;
    for (size_t y = 0; y < aMatrix.height(); ++y) {
        std::cerr << y << " |";
        for (size_t x = 0; x < aMatrix.width(); ++x) {
            std::cerr << std::fixed << std::setprecision(1) << aMatrix.get(x, y).weight
                      << " " << aMatrix.get(x, y).direction << "|";
        }
        std::cerr << std::endl;
    }
}


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
    Matrix<Cell> grid(w, h, Cell{1.0, 0});

    // all players statuses
    Player::Vector players(playerCount);
    Player& MySelf = players[myId];

    // game loop
    while (1) {
        for (size_t id = 0; id < playerCount; ++id) {
            players[id].id          = id; // redundant, but useful for debug

            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            std::cin >> x >> y >> players[id].wallsLeft; std::cin.ignore();

            // if player still playing
            if ((x >= 0) && (y >= 0)) {
                players[id].coords.x = static_cast<size_t>(x);
                players[id].coords.y = static_cast<size_t>(y);
                players[id].bIsAlive = true;

                // set the player on the grid : 0.0 weight to forbid this cell
                grid.set(players[id].coords.x, players[id].coords.y).weight = 0.0;

                // debug:
                if (id == myId) {
                    std::cerr << "myself(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                } else {
                    std::cerr << "player(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                }
            } else {
                players[id].bIsAlive = false;
                std::cerr << "_dead_(" << id << "): [" << x << ", " << y << "]\n";
            }
        }

        // debug:
        dump(grid);

        size_t          wallCount; // number of walls on the board
        std::cin >> wallCount; std::cin.ignore();
        Wall::Vector    walls(wallCount);
        for (size_t wall = 0; wall < wallCount; ++wall) {
            std::cin >> walls[wall].coords.x >> walls[wall].coords.y >> walls[wall].orientation; std::cin.ignore();
            std::cerr << "wall(" << wall << "): [" << walls[wall].coords.x << ", " << walls[wall].coords.y << "] '"
                      << walls[wall].orientation <<"'\n";
        }

        ++turn;
        std::cerr << "turn " << turn << std::endl;

        // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
        Command::right("go go go!");

        // return 1; // for debug purpose
    }

    return 0;
}
