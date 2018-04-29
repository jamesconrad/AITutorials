#ifndef CONNECT_FOUR_H
#define CONNECT_FOUR_H

#include <GL/gl3w.h>
#include <functional>
#include <GLM/glm.hpp>
#include <vector>

enum Player
{
    HUMAN   = 0,
    AI      = 1,
};

enum Color
{
    GOLD    = 0,
    RED     = 1,
    BLUE    = 2,
    GREEN   = 3,
};

class ConnectFourBoard
{
public:
    ConnectFourBoard();

    void InitializeBoard();

    void ResetBoard();
    bool DropCoin(int column, Player whichPlayer);

    Player GetPlayerTurn();
    void SetPlayerTurn(Player player);

    bool IsFinished();

    int ScorePosition(int row, int column, int delta_y, int delta_x);
    int SimpleScoring();

    ConnectFourBoard CreateCopy();

    void DrawBoard(std::function<void(glm::vec2, glm::vec2)> drawQuadFunction);

public:
    // The winning and losing score
    const int MAX_SCORE = 100000;

    const static int PIXEL_WIDTH = 392;
    const static int PIXEL_HEIGHT = 408;

    const static int ROWS = 6;
    const static int COLUMNS = 7;

private:
    char playingField[ROWS][COLUMNS];

    std::vector<glm::vec4> fallingTokens;

    Player currentTurn = Player::HUMAN;
    GLuint coinTexture[4];
    GLuint background;

    const Color playerColor = Color::BLUE;
    const Color aiColor     = Color::RED;

    std::vector<glm::ivec2> winningMove;
    int numberOfMoves   = 0;
};

#endif