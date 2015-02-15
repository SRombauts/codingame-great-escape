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
#include <limits>
#include <stdexcept>

/// Define directions
enum EDirection {
    eRight,
    eLeft,
    eDown,
    eUp
};

/**
 * @brief Send commands to the game thru standard output
 * 
 * Commands are string like LEFT, RIGHT, UP, DOWN or "putX putY putOrientation".
 */
class Command {
public:
    /// Move the player in the specified direction
    static void move(const EDirection aDirection) {
        switch (aDirection) {
        case eRight:
            right("go go go!");
            break;
        case eLeft:
            left("back home");
            break;
        case eDown:
            down("down the path...");
            break;
        case eUp:
            up("up to the sky :)");
            break;
        default:
            throw std::logic_error("move: default");
            break;
        }
    }
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

    /// get coordinates of the next cell at the right of the current one
    Coords right() const {
        return Coords{ x + 1, y };
    }
    /// get coordinates of the next cell at the left of the current one
    Coords left() const {
        return Coords{ x - 1, y };
    }
    /// get coordinates of the next cell at the bottom of the current one
    Coords down() const {
        return Coords{ x, y + 1 };
    }
    /// get coordinates of the next cell at the top of the current one
    Coords up() const {
        return Coords{ x, y - 1 };
    }

    /// get coordinates of the next cell at the top-right of the current one
    Coords upright() const {
        return Coords{ x + 1, y - 1 };
    }
    /// get coordinates of the next cell at the bottom-left of the current one
    Coords downleft() const {
        return Coords{ x - 1, y + 1 };
    }
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

/// wall data for the list of walls
struct Wall {
    /// Vector of walls
    typedef std::vector<Wall> Vector;

    Coords coords;      ///< coordinates of the upper left corner of the wall
    char   orientation; ///< 'H'orizontal or 'V'ertical orientation
};

/// wall collision data for the path of walls
struct Collision {
    bool bRight;    ///< is there a wall on the right of this Cell
    bool bLeft;     ///< is there a wall on the left of this Cell
    bool bDown;     ///< is there a wall on the bottom of this Cell
    bool bUp;       ///< is there a wall on the top of this Cell
};

/// pathfinding data for a cell of the path
struct Cell {
    float       weight;     ///< weighted distance toward the destination
    EDirection  direction;  ///< direction of the shortest/best path
};

/// templated 2D matrix of generic TElement
template <typename TElement>
class Matrix {
public:
    /// Vector of matrices
    typedef std::vector<Matrix<TElement>> Vector;

    /**
     * ctor allocating the matrix to the specified size (with default initialisation)
     *
     * @param aWidthX    Nb of columns (X coordinate)
     * @param aHeightY   Nb of lines   (Y coordinate)
     */
    Matrix(const size_t aWidthX, const size_t aHeightY) {
        mMatrix.resize(aWidthX);
        for (auto& line : mMatrix) {
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
        mMatrix.resize(aWidthX);
        for (auto& line : mMatrix) {
            line.resize(aHeightY, aInitValue);
        }
    }

    /// width of the matrix (Nb of columns, X axis)
    size_t width() const {
        return mMatrix.size();
    }
    /// height of the matrix (Nb of lines, Y axis)
    size_t height() const {
        return mMatrix.at(0).size();
    }

    /// getter for cell at [X, Y] (const reference) uses vector::at() with safety check : can throw std::out_of_range
    const TElement& get(const size_t aX, const size_t aY) const {
        return mMatrix.at(aX).at(aY); // return mMatrix[aX][aY];
    }
    /// getter for cell at [X, Y] (const reference) uses vector::at() with safety check : can throw std::out_of_range
    const TElement& get(const Coords& aCoords) const {
        return mMatrix.at(aCoords.x).at(aCoords.y);
    }
    /// "setter" for cell at [X, Y] (reference) uses vector::at() with safety check : can throw std::out_of_range
    TElement& set(const size_t aX, const size_t aY) {
        return mMatrix.at(aX).at(aY);
    }
    /// "setter" for cell at [X, Y] (reference) uses vector::at() with safety check : can throw std::out_of_range
    TElement& set(const Coords& aCoords) {
        return mMatrix.at(aCoords.x).at(aCoords.y);
    }

private:
    /// Vector of Elements
    typedef std::vector<TElement>   Vector1D;
    /// Matrix as a vector of vectors of Elements
    typedef std::vector<Vector1D>   Vector2D;

    Vector2D mMatrix;    ///< Matrix as a vector of vectors of Elements
};

/// Set a wall into the collision matrix
void wall(Matrix<Collision>& aCollisions, const Wall& aWall) {
    // TODO(SRombauts) add those walls into the 8x8 matrix of left/right/up/down walls
    if (aWall.orientation == 'H') { // 'H' --
        // x,y-1 x+1,y-1
        // x,y   x+1,y
        aCollisions.set(aWall.coords.up())     .bDown = true;
        aCollisions.set(aWall.coords.upright()).bDown = true;
        aCollisions.set(aWall.coords)          .bUp   = true;
        aCollisions.set(aWall.coords.right())  .bUp   = true;
    } else { // 'V' |
        // x-1,y   x,y
        // x-1,y-1 x,y-1
        aCollisions.set(aWall.coords.left()).bRight     = true;
        aCollisions.set(aWall.coords.downleft()).bRight = true;
        aCollisions.set(aWall.coords).bLeft             = true;
        aCollisions.set(aWall.coords.down()).bLeft      = true;
    }
}

/// Recursive shortest path algorithm
void shortest(Matrix<Cell>& aMatrix, const Coords& aCoords, const float aWeight, const EDirection aDirection) {
    // If the weight ot this path is less than any preceding one on this cell
    if (aMatrix.get(aCoords).weight > aWeight) {
        // Update the cell
        aMatrix.set(aCoords).weight = aWeight;
        aMatrix.set(aCoords).direction = aDirection;

        // Recurse into adjacent cells
        if (aCoords.x > 0) {
            shortest(aMatrix, aCoords.left(),  aWeight + 1.0f, eRight);
        }
        if (aCoords.x < aMatrix.width() - 1) {
            shortest(aMatrix, aCoords.right(), aWeight + 1.0f, eLeft);
        }
        if (aCoords.y > 0) {
            shortest(aMatrix, aCoords.up(),    aWeight + 1.0f, eDown);
        }
        if (aCoords.y < aMatrix.height() - 1) {
            shortest(aMatrix, aCoords.down(),  aWeight + 1.0f, eUp);
        }
    }
}
/// Shortest path algorithm
void shortest(Matrix<Cell>& aMatrix, const Player& aPlayer) {
    const float weight = 1.0f;
    size_t x;
    size_t y;

    switch (aPlayer.id) {
    case 0: {}
        x = 8;
        for (y = 0; y < aMatrix.height() - 1; ++y) {
            shortest(aMatrix, Coords{ x, y }, weight, eRight);
        }
        break;
    case 1:
        x = 0;
        for (y = 0; y < aMatrix.height() - 1; ++y) {
            shortest(aMatrix, Coords{ x, y }, weight, eLeft);
        }
        break;
    case 2:
        y = 8;
        for (x = 0; x < aMatrix.width() - 1; ++x) {
            shortest(aMatrix, Coords{ x, y }, weight, eDown);
        }
        break;
    default:
        throw std::logic_error("shortest: default");
        break;
    }
}

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
            const float weight = (aMatrix.get(x, y).weight > 9.9f) ? (9.9f) : (aMatrix.get(x, y).weight);
            std::cerr << std::fixed << std::setprecision(1) << weight
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

    // all players statuses
    Player::Vector players(playerCount);
    Player& MySelf = players[myId];

    // game loop
    while (1) {
        // TODO(SRombauts) a grid for pathfinding by player
        Matrix<Cell> path(w, h, Cell{ std::numeric_limits<float>::max(), eRight });
        for (size_t id = 0; id < playerCount; ++id) {
            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            size_t wallsLeft; // number of walls available for the player
            std::cin >> x >> y >> wallsLeft; std::cin.ignore();

            players[id].id          = id; // redundant, but useful for debug
            players[id].wallsLeft   = wallsLeft;

            // if player still playing
            if ((x >= 0) && (y >= 0)) {
                players[id].coords.x = static_cast<size_t>(x);
                players[id].coords.y = static_cast<size_t>(y);
                players[id].bIsAlive = true;

                // debug:
                if (id == myId) {
                    std::cerr << "myself(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                } else {
                    // set the player on the path : 0.0 weight to forbid this cell
                    path.set(players[id].coords.x, players[id].coords.y).weight = 0.0;
                    std::cerr << "player(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                }
            } else {
                players[id].bIsAlive = false;
                std::cerr << "_dead_(" << id << "): [" << x << ", " << y << "]\n";
            }
        }

        size_t          wallCount; // number of walls on the board
        std::cin >> wallCount; std::cin.ignore();

        Wall::Vector        walls(wallCount);
        Matrix<Collision>   collisions(w, h);
        for (size_t idx = 0; idx < wallCount; ++idx) {
            std::cin >> walls[idx].coords.x >> walls[idx].coords.y >> walls[idx].orientation; std::cin.ignore();
            std::cerr << "idx(" << idx << "): [" << walls[idx].coords.x << ", " << walls[idx].coords.y << "] '"
                      << walls[idx].orientation <<"'\n";
            wall(collisions, walls[idx]);
        }

        ++turn;
        std::cerr << "turn " << turn << std::endl;

        // TODO(SRombauts) test of a pathfinding algorithm:
        shortest(path, MySelf);
        dump(path);

        // use the path to command
        EDirection bestDirection = path.get(MySelf.coords.x, MySelf.coords.y).direction;
        std::cerr << "[" << MySelf.coords.x << ", " << MySelf.coords.y << "]=>" << bestDirection << std::endl;
        Command::move(bestDirection);

        // return 1; // for debug purpose
    }

    return 0;
}
