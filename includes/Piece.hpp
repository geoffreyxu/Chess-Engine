#ifndef PIECE_HPP
#define PIECE_HPP

#include <iostream>
#include <string>
using namespace std;

enum class PieceName { Empty, Pawn, Knight, Bishop, Rook, Queen, King }

class Board;

class Piece
{
public:
    Piece();
    ~Piece();
    int xPosition() { return xPos; }
    int yPosition() { return yPos; }
    int relativeValue();
    bool getColor() { return color; }
    PieceName getName() { return name; }
    virtual bool possibleMove(int, int);
    virtual enemyOnDiag(bool, bool);
    void setMove(int, int);
    bool isAlive();
    void setDead();
private:
    PieceName name = PieceName::Empty
    bool color = false;
    bool isAlive = false;
    int xPos = -1
    int yPos = -1
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Pawn : public Piece
{
public:
    Pawn(bool, int, int);
    ~Pawn();
    using Piece::getColor;
    using Piece::getName;
    using Piece::xPosition;
    using Piece::yPosition
    virtual bool possibleMove(int, int);
    virtual void enemyOnDiag(bool, bool);
private:
    bool leftDiagTarget = false;
    bool rightDiagTarget = false;
}

class Knight : public Piece
{
public:
    Knight(bool, int, int);
    ~Knight();
    using Piece::getColor;
    using Piece::getName;
    using Piece::xPosition;
    using Piece::yPosition
    virtual bool possibleMove(int, int);
}

class Bishop : public Piece
{
public:
    Bishop(bool, int, int);
    ~Bishop();
    using Piece::getColor;
    using Piece::getName;
    using Piece::xPosition;
    using Piece::yPosition
    virtual bool possibleMove(int, int);
    virtual void enemyOnDiag(bool, bool);
private:
    bool leftDiagTarget = false;
    bool rightDiagTarget = false;
}