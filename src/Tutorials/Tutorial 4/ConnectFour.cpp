#include "ConnectFour.h"

#include <SOIL.h>
#include <string>
#include <GLFW/glfw3.h> // GLFW helper library

using namespace glm;

ConnectFourBoard::ConnectFourBoard()
{
}

void ConnectFourBoard::InitializeBoard()
{
    ResetBoard();

    for (int i = 0; i < 4; i++)
    {
        std::string coinLocation = std::string(ASSETS) + std::string("Images/Coin") +
            std::to_string(i) + std::string(".png");

        coinTexture[i] = SOIL_load_OGL_texture(
            coinLocation.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );

        glBindTexture(GL_TEXTURE_2D, coinTexture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    background = SOIL_load_OGL_texture(
        ASSETS"Images/Background.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    glBindTexture(GL_TEXTURE_2D, background);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void ConnectFourBoard::ResetBoard()
{
    for (int i = 0; i < COLUMNS; i++)
    {
        for (int j = 0; j < ROWS; j++)
        {
            playingField[j][i] = '0'; // Reset the board here
        }
    }

    fallingTokens.clear();

    numberOfMoves = 0;
    currentTurn = Player::HUMAN;
}

bool ConnectFourBoard::DropCoin(int column, Player whichPlayer)
{
    if (playingField[0][column] == '0' && column >= 0 && column < COLUMNS)
    {
        for (int i = ROWS - 1; i >= 0; i--)
        {
            if (playingField[i][column] == '0')
            {
                playingField[i][column] = whichPlayer == Player::HUMAN ? '1' : '2';
                fallingTokens.push_back(glm::vec4(i, column, glfwGetTime(), whichPlayer == Player::HUMAN ? 0.0f : 1.0f));
                numberOfMoves += 1;
                return true;
            }
        }
    }

    // Couldn't drop a coin. Maybe it is full
    return false;
}

Player ConnectFourBoard::GetPlayerTurn()
{
    return currentTurn;
}

void ConnectFourBoard::SetPlayerTurn(Player player)
{
    currentTurn = player;
}

bool ConnectFourBoard::IsFinished()
{
    return (numberOfMoves == ROWS * COLUMNS);
}

int ConnectFourBoard::ScorePosition(int row, int column, int delta_y, int delta_x)
{
    int human_points = 0;
    int computer_points = 0;

    // Save winning positions in vector for later
    std::vector<ivec2> winningMoveForHuman;
    std::vector<ivec2> winningMoveForAI;

    // Determine score through amount of available chips
    for (int i = 0; i < 4; i++)
    {
        // '0' = no chip
        // '1' = Player chip
        // '2' = AI chip

        if (playingField[row][column] == '1')
        {
            winningMoveForHuman.push_back(ivec2(row, column));
            human_points++; // Add for each human chip
        }
        else if (playingField[row][column] == '2')
        {
            winningMoveForAI.push_back(ivec2(row, column));
            computer_points++; // Add for each computer chip
        }

        // Moving through our board
        row += delta_y;
        column += delta_x;
    }

    // If the human matched 4 in a row
    if (human_points == 4)
    {
        winningMove = winningMoveForHuman;
        // Human won (-100000)
        return -MAX_SCORE;
    }
    else if (computer_points == 4)
    {
        winningMove = winningMoveForAI;
        // Computer won (100000)
        return MAX_SCORE;
    }
    else
    {
        // Return normal points
        return computer_points;
    }
}

int ConnectFourBoard::SimpleScoring()
{
    // Board-size: 7x6 (width x height)

    int vertical_points     = 0;
    int horizontal_points   = 0;
    int diagonal_points_1   = 0;
    int diagonal_points_2   = 0;

    // Vertical points
    // Check each column for vertical score
    // 
    // Possible situations
    //  0  1  2  3  4  5  6
    // [x][ ][ ][ ][ ][ ][ ] 0
    // [x][x][ ][ ][ ][ ][ ] 1
    // [x][x][x][ ][ ][ ][ ] 2
    // [x][x][x][ ][ ][ ][ ] 3
    // [ ][x][x][ ][ ][ ][ ] 4
    // [ ][ ][x][ ][ ][ ][ ] 5
    for (int row = 0; row < ROWS - 3; row++) 
    {
        for (int column = 0; column < COLUMNS; column++) 
        {
            // -------------  Here's a freebie!
            int score = ScorePosition(row, column, 1, 0);
            if (score ==  MAX_SCORE) return  MAX_SCORE;
            if (score == -MAX_SCORE) return -MAX_SCORE;
            vertical_points += score;
        }
    }

    // Horizontal points
    // Check each row's score
    // 
    // Possible situations
    //  0  1  2  3  4  5  6
    // [x][x][x][x][ ][ ][ ] 0
    // [ ][x][x][x][x][ ][ ] 1
    // [ ][ ][x][x][x][x][ ] 2
    // [ ][ ][ ][x][x][x][x] 3
    // [ ][ ][ ][ ][ ][ ][ ] 4
    // [ ][ ][ ][ ][ ][ ][ ] 5
    for (int row = 0; row < ROWS; row++) 
    {
        for (int column = 0; column < COLUMNS - 3; column++) 
        {
			int score = ScorePosition(row, column, 0, 1);
			if (score == MAX_SCORE) return  MAX_SCORE;
			if (score == -MAX_SCORE) return -MAX_SCORE;
			horizontal_points += score;
        }
    }

    // Diagonal points 1 (left-bottom)
    //
    // Possible situation
    //  0  1  2  3  4  5  6
    // [x][ ][ ][x][ ][ ][ ] 0
    // [ ][x][ ][ ][x][ ][ ] 1
    // [x][ ][x][ ][ ][x][ ] 2
    // [ ][x][ ][x][ ][ ][x] 3
    // [ ][ ][x][ ][ ][ ][ ] 4
    // [ ][ ][ ][x][ ][ ][ ] 5
	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 3; column++)
		{
			int score = ScorePosition(row, column, 1, 1);
			if (score == MAX_SCORE) return  MAX_SCORE;
			if (score == -MAX_SCORE) return -MAX_SCORE;
			diagonal_points_1 += score;
		}
	}

    // Diagonal points 2 (right-bottom)
    //
    // Possible situation
    //  0  1  2  3  4  5  6
    // [ ][ ][ ][x][ ][ ][ ] 0
    // [ ][ ][x][ ][ ][ ][ ] 1
    // [ ][x][ ][ ][ ][ ][x] 2
    // [x][ ][ ][ ][ ][x][ ] 3
    // [ ][ ][ ][ ][x][ ][ ] 4
    // [ ][ ][ ][x][ ][ ][ ] 5
	for (int row = 0; row < 3; row++)
	{
		for (int column = 3; column < COLUMNS; column++)
		{
			int score = ScorePosition(row, column, 1, -1);
			if (score == MAX_SCORE) return  MAX_SCORE;
			if (score == -MAX_SCORE) return -MAX_SCORE;
			diagonal_points_2 += score;
		}
	}

    return vertical_points + horizontal_points + diagonal_points_1 + diagonal_points_2;
}

ConnectFourBoard ConnectFourBoard::CreateCopy()
{
    ConnectFourBoard newBoard;
    newBoard.currentTurn = this->currentTurn;
    newBoard.winningMove = this->winningMove;
    newBoard.numberOfMoves = this->numberOfMoves;

    // Don't need to worry about the textures, they're not used
    // And same with the constant values

    // But we need to copy the values in the board
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            newBoard.playingField[i][j] = this->playingField[i][j];
        }
    }

    return newBoard;
}

void ConnectFourBoard::DrawBoard(std::function<void(vec2, vec2)> drawQuadFunction)
{
    vec2 coinSize = vec2(PIXEL_WIDTH, PIXEL_HEIGHT) / vec2((float)COLUMNS, (float)ROWS);
    vec2 coinBias = coinSize / 2.0f;

    glBindTexture(GL_TEXTURE_2D, background);
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            vec2 pos = coinBias + (coinSize * vec2(j, ROWS - 1 - i));

            drawQuadFunction(pos, coinSize);
        }
    }

    for (auto itr = fallingTokens.begin(); itr != fallingTokens.end(); itr++)
    {
        vec4 ft = (*itr);

        int x = (int)ft.y;
        int y = (int)ft.x;

        float deltaTime = min(1.0f, (float)glfwGetTime() - ft.z + 0.25f);

        const float a = 3.1415f * (4.0f * deltaTime - 1.0f);
        float interp = 1.0f - abs(sin(a) / a);
        {
            if (ft.w < 0.5f)    glBindTexture(GL_TEXTURE_2D, coinTexture[(int)playerColor]);
            else                glBindTexture(GL_TEXTURE_2D, coinTexture[(int)aiColor]);

            vec2 posA = coinBias + (coinSize * vec2(x, ROWS * 2 - y));
            vec2 posB = coinBias + (coinSize * vec2(x, ROWS - 1 - y));

            vec2 position = mix(posA, posB, interp);

            drawQuadFunction(position, coinSize);
        }
    }
}