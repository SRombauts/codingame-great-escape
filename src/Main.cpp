/**
 * @file    Main.cpp
 * @brief   My attempt at the Multiplayer CodinGame "The Great Escape".
 *
 * Copyright (c) 2015 Sebastien Rombauts (sebastien.rombauts@gmail.com, http://srombauts.github.io)
 *
 * Original source code available at GitHub https://github.com/SRombauts/codingame-great-escape
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <chrono> // NOLINT(build/c++11)

/// Define directions
enum EDirection {
    eNone,
    eRight,
    eLeft,
    eDown,
    eUp
};

/// Convert a player Id to the direction where it shall go
EDirection fromPlayerId(const size_t aId) {
    EDirection direction;
    switch (aId) {
    case 0: direction = eRight; break;
    case 1: direction = eLeft;  break;
    case 2: direction = eDown;  break;
    default:
        throw std::logic_error("fromPlayerId: default");
    }
    return direction;
}

/// Convert a direction to an explicit visual character for debug dump
char toChar(const EDirection aDirection) {
    char direction;
    switch (aDirection) {
    case eNone:     direction = ' ';    break;
    case eRight:    direction = '>';    break;
    case eLeft:     direction = '<';    break;
    case eDown:     direction = 'v';    break;
    case eUp:       direction = '^';    break;
    default:
        throw std::logic_error("toChar: default");
    }
    return direction;
}

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

    /// get coordinates of the next cell at the bottom-right of the current one
    Coords downright() const {
        return Coords{ x + 1, y + 1 };
    }
    /// get coordinates of the next cell at the bottom-left of the current one
    Coords downleft() const {
        return Coords{ x - 1, y + 1 };
    }
    /// get coordinates of the next cell at the top-right of the current one
    Coords upright() const {
        return Coords{ x + 1, y - 1 };
    }
    /// get coordinates of the next cell at the top-left of the current one
    Coords upleft() const {
        return Coords{ x - 1, y - 1 };
    }

    /// get next coordinates into the specified direction
    Coords next(EDirection aDirection) {
        switch (aDirection) {
        case eRight:
            return right();
        case eLeft:
            return left();
        case eDown:
            return down();
        case eUp:
            return up();
        case eNone:
        default:
            throw std::logic_error("walls: default");
            break;
        }
    }

    /// Comparaison operator
    bool operator== (const Coords& aCoords) const {
        return ((x == aCoords.x) && (y == aCoords.y));
    }
};

/// Debug dump
std::ostream& operator<<(std::ostream& aStream, const Coords& aCoords) {
    aStream << aCoords.x << ',' << aCoords.y;
    return aStream;
}

/// wall data for the list of walls
struct Wall {
    /// Vector of walls
    typedef std::vector<Wall> Vector;

    Coords coords;      ///< coordinates of the upper left corner of the wall
    char   orientation; ///< 'H'orizontal or 'V'ertical orientation
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
        case eNone:
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
        std::cout << "LEFT " << apMessage << std::endl;
    }
    /// Move the player to the bottom of the board (y++)
    static void down(const char* apMessage) {
        std::cout << "DOWN " << apMessage << std::endl;
    }
    /// Move the player to the top of the board (y--)
    static void up(const char* apMessage) {
        std::cout << "UP " << apMessage << std::endl;
    }
    /// Put a new Wall to a specified location and orientation
    static void put(const Wall& aWall, const char* apMessage) {
        std::cout << aWall.coords.x << " " << aWall.coords.y << " " << aWall.orientation << " " << apMessage
                  << std::endl;
    }
};

/// wall collision data of a cell for the matrix of walls
struct Collision {
    bool bRight;    ///< is there a wall on the right of this Cell
    bool bLeft;     ///< is there a wall on the left of this Cell
    bool bDown;     ///< is there a wall on the bottom of this Cell
    bool bUp;       ///< is there a wall on the top of this Cell

    /// Debug dump (for the bellow generic templated Matrix::dump() method)
    void dump() const {
        std::cerr << (bLeft ? '<' : ' ') << (bDown ? 'v' : ' ')
                  << (bUp ? '^' : ' ') << (bRight ? '>' : ' ') << "|";
    }
};

/// data of a cell for the matrix of pathfinding
struct Cell {
    size_t      distance;   ///< distance toward the destination
    EDirection  direction;  ///< direction of the shortest/best path

    /// Debug dump (for the bellow generic templated Matrix::dump() method)
    void dump() const {
        std::cerr << std::fixed << std::setprecision(1) << std::setw(2) << distance << " "
                  << toChar(direction) << "|";
    }
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

    /// Initialize all the matrix with the provided value
    void init(const TElement& aInitValue) {
        for (auto& line : mMatrix) {
            for (auto& cell : line) {
                cell = aInitValue;
            }
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

    /// debug: dump content of the Matrix of TElement, using a required TElement::dump() method
    void dump() {
        std::cerr << " |";
        for (size_t x = 0; x < width(); ++x) {
            std::cerr << x << "   |";
        }
        std::cerr << std::endl;
        for (size_t y = 0; y < height(); ++y) {
            std::cerr << y << "|";
            for (size_t x = 0; x < width(); ++x) {
                get(x, y).dump();
            }
            std::cerr << std::endl;
        }
    }

private:
    /// Vector of Elements
    typedef std::vector<TElement>   Vector1D;
    /// Matrix as a vector of vectors of Elements
    typedef std::vector<Vector1D>   Vector2D;

    /// TODO(SRombauts) try and compare performances (construction, copy, usage) with a 1D vector using math
    ///                 Then compare with a std::array
    Vector2D mMatrix;    ///< Matrix as a vector of vectors of Elements
};

/// player data
struct Player {
    /// Vector of players
    typedef std::vector<Player> Vector;
    /// Vector of pointers of players (for sorting by rank)
    typedef std::vector<Player*> VectorPtr;

    /// Init the Matrix
    Player(const size_t aWidthX, const size_t aHeightY) :
        paths(aWidthX, aHeightY),
        id(0),
        bIsMySelf(false),
        orientation(eNone),
        coords(),
        wallsLeft(0),
        distance(0),
        order(0),
        rank(0),
        bIsAlive(false) {
    }

    Matrix<Cell>    paths;       ///< grid for pathfinding of the player
    size_t          id;          ///< id of the player (implicit orientation)
    bool            bIsMySelf;   ///< explicite shortcut for (id == myId) and/or (order == 0)
    EDirection      orientation; ///< general direction of the path to exit (explicit orientation)
    Coords          coords;      ///< coordinates of the player
    size_t          wallsLeft;   ///< number of walls available for the player
    size_t          distance;    ///< distance left to reach the destination
    size_t          order;       ///< order of the player into the turn based on its id vs me (I am playing = order 0)
    size_t          rank;        ///< rank based on the distance left and the order of the player
    bool            bIsAlive;    ///< true while the player is alive

    /// ranking of each player : distance left, and take into account the order of the player into the turn
    static bool compare(const Player* apA, const Player* apB) {
    if (apA->distance != apB->distance) {
        return (apA->distance < apB->distance);
    } else  {
        return (apA->order < apB->order);
    }
}
};

/// Time measure using C++11 std::chrono
class Measure {
public:
    /// Start a time measure
    void start() {
        // std::chrono::steady_clock would be more stable, but does not exist in Travis CI GCC 4.6
        mStartTime = std::chrono::high_resolution_clock::now();
    }
    /// Get time elapsed since first time measure
    double get() {
        auto diffTime = (std::chrono::high_resolution_clock::now() - mStartTime);
        return std::chrono::duration<double, std::milli>(diffTime).count();
    }

private:
    std::chrono::high_resolution_clock::time_point   mStartTime; ///< Store the first time measure
};

/// Recursive shortest path algorithm
void findShortest(Matrix<Cell>& aOutPaths, const Matrix<Collision>& aCollisions, const EDirection aOrientation,
    const Coords& aCoords, const size_t aDistance, const EDirection aDirection) {
    // If the distance of this path is less than any preceding one on this cell
    // In case of equal distance, go into the preferred direction (player orientation)
    if ((aOutPaths.get(aCoords).distance > aDistance)
        || ((aOutPaths.get(aCoords).distance == aDistance) && (aDirection == aOrientation))) {
        // Update the cell
        aOutPaths.set(aCoords).distance = aDistance;
        aOutPaths.set(aCoords).direction = aDirection;

        // Recurse into adjacent cells
        if ((aCoords.x > 0) && (!aCollisions.get(aCoords).bLeft)) {
            findShortest(aOutPaths, aCollisions, aOrientation, aCoords.left(), aDistance + 1, eRight);
        }
        if ((aCoords.x < aOutPaths.width() - 1) && (!aCollisions.get(aCoords).bRight)) {
            findShortest(aOutPaths, aCollisions, aOrientation, aCoords.right(), aDistance + 1, eLeft);
        }
        if ((aCoords.y > 0) && (!aCollisions.get(aCoords).bUp)) {
            findShortest(aOutPaths, aCollisions, aOrientation, aCoords.up(), aDistance + 1, eDown);
        }
        if ((aCoords.y < aOutPaths.height() - 1) && (!aCollisions.get(aCoords).bDown)) {
            findShortest(aOutPaths, aCollisions, aOrientation, aCoords.down(), aDistance + 1, eUp);
        }
    }
}
/// Shortest path algorithm
void findShortest(Matrix<Cell>& aOutPaths, const Matrix<Collision>& aCollisions, const EDirection aOrientation) {
    size_t x;
    size_t y;

    switch (aOrientation) {
    case eRight:
        x = 8;
        for (y = 0; y < aOutPaths.height(); ++y) {
            findShortest(aOutPaths, aCollisions, aOrientation, Coords{ x, y }, 0, eNone);
        }
        break;
    case eLeft:
        x = 0;
        for (y = 0; y < aOutPaths.height(); ++y) {
            findShortest(aOutPaths, aCollisions, aOrientation, Coords{ x, y }, 0, eNone);
        }
        break;
    case eDown:
        y = 8;
        for (x = 0; x < aOutPaths.width(); ++x) {
            findShortest(aOutPaths, aCollisions, aOrientation, Coords{ x, y }, 0, eNone);
        }
        break;
    case eUp:
    case eNone:
    default:
        throw std::logic_error("shortest: default");
        break;
    }
}

/// Set a wall into the collision matrix
void addWallCollisions(Matrix<Collision>& aOutCollisions, const Wall& aWall, const bool abValue = true) {
    if (aWall.orientation == 'H') { // 'H' --
        // x,y-1 x+1,y-1
        // x,y   x+1,y
        aOutCollisions.set(aWall.coords.up())     .bDown = abValue;
        aOutCollisions.set(aWall.coords.upright()).bDown = abValue;
        aOutCollisions.set(aWall.coords)          .bUp   = abValue;
        aOutCollisions.set(aWall.coords.right())  .bUp   = abValue;
    } else { // .orientation == 'V'
        // x-1,y   x,y
        // x-1,y-1 x,y-1
        aOutCollisions.set(aWall.coords.left()).bRight     = abValue;
        aOutCollisions.set(aWall.coords.downleft()).bRight = abValue;
        aOutCollisions.set(aWall.coords).bLeft             = abValue;
        aOutCollisions.set(aWall.coords.down()).bLeft      = abValue;
    }
}


/// Test compatibility of a new wall against a wall already on the board
bool isCompatible(const Wall& aExistingWall, const Wall& aNewWall) {
    bool bIsCompatible = true;
    if (aExistingWall.orientation == 'H') {
        if        ((aNewWall.orientation == 'H')
               &&    ((aExistingWall.coords.left()  == aNewWall.coords)
                   || (aExistingWall.coords         == aNewWall.coords)
                   || (aExistingWall.coords.right() == aNewWall.coords))) {
            bIsCompatible = false;
        } else if ((aNewWall.orientation == 'V') && (aExistingWall.coords.upright() == aNewWall.coords)) {
            bIsCompatible = false;
        }
    } else { // gWall.orientation == 'V'
        if        ((aNewWall.orientation == 'V')
               &&   ((aExistingWall.coords.up()     == aNewWall.coords)
                  || (aExistingWall.coords          == aNewWall.coords)
                  || (aExistingWall.coords.down()   == aNewWall.coords))) {
            bIsCompatible = false;
        } else if ((aNewWall.orientation == 'H') && ((aExistingWall.coords.downleft() == aNewWall.coords))) { // 'V'
            bIsCompatible = false;
        }
    }
    return bIsCompatible;
}

/// Test compatibility of a new wall based solely on coordinates
bool isCompatible(const size_t aWidthX, const size_t aHeightY, const Wall& aWall) {
    bool bIsCompatible = true;
    if (aWall.orientation == 'H') {
        if ((aWall.coords.x >= aWidthX - 1) || (aWall.coords.y == 0) || (aWall.coords.y > aHeightY)) {
            bIsCompatible = false;
        }
    } else { // .orientation == 'V'
        if ((aWall.coords.y >= aHeightY - 1) || (aWall.coords.x == 0) || (aWall.coords.x > aWidthX)) {
            bIsCompatible = false;
        }
    }
    // std::cerr << "isCompatible()=" << bIsCompatible << std::endl;
    return bIsCompatible;
}

/// Test compatibility of a new wall against all walls already on the board
bool isCompatible(const size_t aWidthX, const size_t aHeightY, const Wall::Vector& aExistingWalls, const Wall& aWall) {
    bool bIsCompatible = isCompatible(aWidthX, aHeightY, aWall);
    if (bIsCompatible) {
        Wall::Vector::const_iterator iWall = aExistingWalls.begin();
        while ((iWall != aExistingWalls.end()) && (bIsCompatible == true)) {
            bIsCompatible = isCompatible(*iWall, aWall);
            ++iWall;
        }
        std::cerr << "isCompatible([" << aWall.coords << "] " << aWall.orientation << ")="
            << bIsCompatible << std::endl;
    }
    return bIsCompatible;
}

/// Evaluation of impacts of the placement of a wall
struct Evaluation {
    bool    bIsValid;       ///< Does this structure represent a valide result (no player blocked)
    Wall    wall;           ///< Wall to evaluate
    size_t  impactOnFirst;  ///< Increase of distance on the shortest path of the first player [O:
    size_t  impactOnMySelf; ///< Increase of distance on the shortest path of myself
    size_t  impactOnOther;  ///< Increase of distance on the shortest path of the other player if any

    /// evaluation of the best result to keep (equal is to keep the LAST best eval, ie next to the exit
    bool operator<= (const Evaluation& aEvaluation) {
        return (((100.f*impactOnFirst)-(70.f*impactOnMySelf)+(40.f*impactOnOther))
            <= ((100.f*aEvaluation.impactOnFirst)-(70.f*aEvaluation.impactOnMySelf)+(40.f*aEvaluation.impactOnOther)));
    }
};

/// Evaluation of all impacts of a wall
void evalWall(Matrix<Cell>& aPaths, Matrix<Collision>& aCollisions,
              const Player::Vector& aPlayers, const Wall::Vector& aExistingWalls,
              const Wall& aWall, Evaluation& aBestEval) {
    bool bIsCompatible = isCompatible(aPaths.width(), aPaths.height(), aExistingWalls, aWall);
    if (bIsCompatible) {
        Evaluation eval;
        eval.bIsValid       = true;
        eval.impactOnFirst  = 0;
        eval.impactOnMySelf = 0;
        eval.impactOnOther  = 0;

        addWallCollisions(aCollisions, aWall, true);    // set

        for (const auto& player : aPlayers) {
            if (player.bIsAlive) {
                aPaths.init(Cell{std::numeric_limits<size_t>::max(), eNone});
                findShortest(aPaths, aCollisions, player.orientation);
                const size_t nextDistance = aPaths.get(player.coords).distance;
                if (nextDistance < std::numeric_limits<size_t>::max()) {
                    std::cerr << "nextDistance(" << player.id << " [" << player.coords << "])="
                              << nextDistance << std::endl;
                    if (player.rank == 0) {
                        eval.impactOnFirst    = (nextDistance - player.distance);
                        std::cerr << "impactOnFirst(" << nextDistance << "-" << player.distance
                                  << ")=" << eval.impactOnFirst << std::endl;
                    } else if (player.bIsMySelf) {
                        eval.impactOnMySelf   = (nextDistance - player.distance);
                        std::cerr << "impactOnMySelf(" << nextDistance << "-" << player.distance
                            << ")=" << eval.impactOnMySelf << std::endl;
                    } else {
                        eval.impactOnOther    = (nextDistance - player.distance);
                        std::cerr << "impactOnOther(" << nextDistance << "-" << player.distance
                            << ")=" << eval.impactOnOther << std::endl;
                    }
                } else {
                    eval.bIsValid = false;
                    break;
                }
            }
        }
        // evaluation of the result to keep the best (keep the last one, ie near the exit)
        if ((eval.bIsValid) && (eval.impactOnFirst > 0) && ((aBestEval <= eval) || (!aBestEval.bIsValid))) {
            std::cerr << "new best[" << aWall.coords << "] " << aWall.orientation
                << " (" << eval.impactOnFirst << ";" << eval.impactOnMySelf << ";" << eval.impactOnOther << ")\n";
            eval.wall = aWall;
            aBestEval = eval;
            std::cerr << "new best[" << aBestEval.wall.coords << "] " << aBestEval.wall.orientation << "\n";
            std::cerr << "new best(" << aBestEval.impactOnFirst << ";" << aBestEval.impactOnMySelf
                << ";" << aBestEval.impactOnOther << ")\n";
        }

        addWallCollisions(aCollisions, aWall, false);   // reset
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

    // all players statuses
    Player::Vector players(playerCount, Player(w, h));
    Player& mySelf = players[myId];
    mySelf.bIsMySelf = true;
    Measure measure;
    bool bModeWall = false; // memory to keep putting walls after the first one

    // game loop
    for (size_t turn = 0; turn < 100; ++turn) {
        // wait and read players data
        for (size_t id = 0; id < playerCount; ++id) {
            Player& player = players[id];

            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            size_t wallsLeft; // number of walls available for the player
            std::cin >> x >> y >> wallsLeft; std::cin.ignore();

            player.id          = id;               // redundant with the index, but useful
            player.orientation = fromPlayerId(id); // redundant with the id, but useful
            player.wallsLeft   = wallsLeft;

            // if player still playing
            if ((x >= 0) && (y >= 0)) {
                player.coords.x = static_cast<size_t>(x);
                player.coords.y = static_cast<size_t>(y);
                player.bIsAlive = true;

                /* debug:
                if (player.bIsMySelf) {
                    std::cerr << "myself(" << player.id << "): [" << player.coords.x
                              << ", " << player.coords.y << "] (wallsLeft=" << player.wallsLeft << ")\n";
                } else {
                    std::cerr << "player(" << player.id << "): [" << player.coords.x
                              << ", " << player.coords.y << "] (wallsLeft=" << player.wallsLeft << ")\n";
                }
                */
            } else {
                player.bIsAlive = false;
                std::cerr << "_dead_(" << id << "): [" << x << ", " << y << "]\n";
            }
        }

        // read walls data
        size_t wallCount; // number of walls on the board
        std::cin >> wallCount; std::cin.ignore();

        Wall::Vector        walls(wallCount);
        Matrix<Collision>   collisions(w, h);
        for (auto& wall : walls) {
            std::cin >> wall.coords.x >> wall.coords.y >> wall.orientation; std::cin.ignore();
            /* std::cerr << "wall[" << wall.coords << "] '"
                      << wall.orientation <<"'\n"; */
            addWallCollisions(collisions, wall);
        }

        // Start-counting the time after the input are all read
        measure.start();

    //  std::cerr << "turn " << turn << std::endl;

        // debug dump:
    //  std::cerr << "matrix of walls:" << std::endl;
    //  collisions.dump();

    //  std::cerr << "matrices of paths:" << std::endl;
        // pathfinding for each player (taking walls into account)
        for (auto& player : players) {
            // re-init pathfinding data
            player.paths.init(Cell{ std::numeric_limits<size_t>::max(), eNone });
            // if player still playing
            if (player.bIsAlive) {
                // pathfinding algorithm:
                findShortest(player.paths, collisions, player.orientation);
                // debug dump:
            //  player.paths.dump();
                player.distance = player.paths.get(player.coords).distance;
                // debug dump:
                std::cerr << player.id << ": distance: " << player.distance << std::endl;
            } else {
                player.distance = std::numeric_limits<size_t>::max(); // dead player is far far away...
            }
        }

        // order of the player into the turn based on its id vs my id (it is my turn, so I have the order 0)
        // (a dead player is always last in the ranking since its distance left is set to max => is is removed later)
        Player::VectorPtr rankedPlayers;
        for (size_t order = 0; order < playerCount; ++order) {
           size_t id = (mySelf.id + order) % playerCount;
           players[id].order = order;
           // std::cerr << id << ": order: " << players[id].order << std::endl;
           rankedPlayers.push_back(&players[id]);
        }

        // ranking of each player : distance left, and take into account the order of the player into the turn
        std::sort(rankedPlayers.begin(), rankedPlayers.end(), Player::compare);
        // explicit rank
        for (size_t rank = 0; rank < rankedPlayers.size(); rank++) {
           rankedPlayers[rank]->rank = rank;
        }
        // remove the dead player (always the last one if any)
        if (!rankedPlayers.back()->bIsAlive) {
            rankedPlayers.pop_back();
        }
        // Debug dump:
        std::cerr << "ranks: ";
        for (const auto& player : rankedPlayers) {
            std::cerr << player->id << ", ";
        }
        std::cerr << std::endl;

        // list of players before me based on ranking
        Player::VectorPtr   playersBeforeMe;
        if (!rankedPlayers[0]->bIsMySelf) {
            playersBeforeMe.push_back(rankedPlayers[0]);
            if (!rankedPlayers[1]->bIsMySelf) {
                playersBeforeMe.push_back(rankedPlayers[0]);
            }
        }

        bool bNewWall = false;

        // Only put a wall if :
        // - I have walls left AND
        //   - I am not the first player AND
        //     - The first player is at a distance < 4 (AFTER the middle of the board)
        //       -    I am the last one (2nd out of 2 or 3d out of 3 alive players)
        //       - OR I am the 2nd out of 3 AND the 3rd player is at a distance > 1
        std::cerr << mySelf.wallsLeft << " wall(s) left\n";
        if (mySelf.wallsLeft > 0) {         // I have walls left AND
            std::cerr << playersBeforeMe.size() << " player(s) before me\n";
            if (playersBeforeMe.size() > 0) {       // I am not the first player AND
                const Player& firstPlayer = players[playersBeforeMe[0]->id];
                std::cerr << "first player id=" << firstPlayer.id << " distance=" << firstPlayer.distance << std::endl;

                if ((firstPlayer.distance < 4) || (bModeWall)) {     // The first player is not far from the end
                    if (rankedPlayers.back()->bIsMySelf) {
                        std::cerr << "I am the last player!\n";
                    } else {
                        std::cerr << "I am the 2nd player out of 3!\n";
                        std::cerr << "last player id=" << rankedPlayers.back()->id
                                  << " distance=" << rankedPlayers.back()->distance << std::endl;
                    }
                    //    I am the last one (2nd out of 2 or 3d out of 3 alive players)
                    // OR I am the 2nd out of 3 AND the 3rd player is at a distance > 2
                    if ((rankedPlayers.back()->bIsMySelf) || (rankedPlayers.back()->distance > 2) || (bModeWall)) {
                        Matrix<Collision>   nextCollisions = collisions;
                        Matrix<Cell>        nextPaths(w, h);
                        Evaluation          bestEval;
                        bestEval.bIsValid       = false;
                        bestEval.impactOnFirst  = 0;
                        bestEval.impactOnMySelf = 0;
                        bestEval.impactOnOther  = 0;

                        bModeWall = true; // memory to keep putting walls

                        // iterate on the path of the first player
                        Coords coords   = firstPlayer.coords;
                        size_t distance = firstPlayer.distance;
                        while (distance > 0) {
                            const Cell& cell = firstPlayer.paths.get(coords);
                            std::cerr << "path[" << coords << "]" << std::endl;

                            // calculate all blocking walls
                            switch (cell.direction) {
                            case eRight:
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords.right(), 'V'}, bestEval);
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords.upright(), 'V'}, bestEval);
                                break;
                            case eLeft:
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords, 'V'}, bestEval);
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords.up(), 'V'}, bestEval);
                                break;
                            case eDown:
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords.down(), 'H'}, bestEval);
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords.downleft(), 'H'}, bestEval);
                                break;
                            case eUp:
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords, 'H'}, bestEval);
                                evalWall(nextPaths, nextCollisions, players, walls,
                                         Wall{coords.left(), 'H'}, bestEval);
                                break;
                            case eNone:
                            default:
                                throw std::logic_error("walls: default");
                                break;
                            }

                            coords   = coords.next(cell.direction);
                            const Cell& nextCell = firstPlayer.paths.get(coords);
                            distance = nextCell.distance;
                        }
                        // if a best evaluation is available, put the wall
                        if (bestEval.bIsValid) {
                            std::cerr << "best eval (" << bestEval.impactOnFirst << ";" << bestEval.impactOnMySelf
                                      << ";" << bestEval.impactOnOther << ")\n";
                            bNewWall = true;
                            Command::put(bestEval.wall, "stop here!");
                        }
                    }
                }
            }
        }

        if (false == bNewWall) {
            // use the matrix of shortest paths to issue a command
            EDirection bestDirection = mySelf.paths.get(mySelf.coords).direction;
            std::cerr << "[" << mySelf.coords << "]=>'" << toChar(bestDirection) << "'\n";
            Command::move(bestDirection);
        }

        // Calculate the time elapsed since start of this turn
        std::cerr << std::fixed << measure.get() << "ms\n";
    }

    return 0;
}
