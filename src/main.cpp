    #include "raylib.h"
    #include "Math.h"

    constexpr float SCREEN_WIDTH = 1200.0f;
    constexpr float SCREEN_HEIGHT = 800.0f;
    constexpr Vector2 CENTER{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };

    // Ball can move half the screen width per-second
    constexpr float BALL_SPEED = SCREEN_WIDTH * 0.5f;
    constexpr float BALL_SIZE = 40.0f;

    // Paddles can move half the screen height per-second
    constexpr float PADDLE_SPEED = SCREEN_HEIGHT * 0.5f;
    constexpr float PADDLE_WIDTH = 40.0f;
    constexpr float PADDLE_HEIGHT = 80.0f;

    struct Box
    {
        float xMin;
        float xMax;
        float yMin;
        float yMax;
    };

    bool BoxOverlap(Box box1, Box box2)
    {
        bool x = box1.xMax >= box2.xMin && box1.xMin <= box2.xMax;
        bool y = box1.yMax >= box2.yMin && box1.yMin <= box2.yMax;
        return x && y;
    }

    Rectangle BoxToRec(Box box)
    {
        Rectangle rec;
        rec.x = box.xMin;
        rec.y = box.yMin;
        rec.width = box.xMax - box.xMin;
        rec.height = box.yMax - box.yMin;
        return rec;
    }

    Box BallBox(Vector2 position)
    {
        Box box;
        box.xMin = position.x - BALL_SIZE * 0.5f;
        box.xMax = position.x + BALL_SIZE * 0.5f;
        box.yMin = position.y - BALL_SIZE * 0.5f;
        box.yMax = position.y + BALL_SIZE * 0.5f;
        return box;
    }

    Box PaddleBox(Vector2 position)
    {
        Box box;
        box.xMin = position.x - PADDLE_WIDTH * 0.5f;
        box.xMax = position.x + PADDLE_WIDTH * 0.5f;
        box.yMin = position.y - PADDLE_HEIGHT * 0.5f;
        box.yMax = position.y + PADDLE_HEIGHT * 0.5f;
        return box;
    }

    void ResetBall(Vector2& position, Vector2& direction)
    {
        position = CENTER;
        direction.x = rand() % 2 == 0 ? -1.0f : 1.0f;
        direction.y = 0.0f;
        direction = Rotate(direction, Random(0.0f, 45.0f) * DEG2RAD);
    }

    void DrawBall(Vector2 position, Color color)
    {
        Box ballBox = BallBox(position);
        DrawRectangleRec(BoxToRec(ballBox), color);
    }

    void DrawPaddle(Vector2 position, Color color)
    {
        Box paddleBox = PaddleBox(position);
        DrawRectangleRec(BoxToRec(paddleBox), color);
    }

    int main()
    {
        Vector2 ballPosition;
        Vector2 ballDirection;
        ResetBall(ballPosition, ballDirection);

        Vector2 paddle1Position, paddle2Position;
        paddle1Position.x = SCREEN_WIDTH * 0.05f;
        paddle2Position.x = SCREEN_WIDTH * 0.95f;
        paddle1Position.y = paddle2Position.y = CENTER.y;

        int Player1Score = 0;
        int Player2Score = 0;

        bool gameOver = false;

        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");
        SetTargetFPS(60);
        while (!WindowShouldClose())
        {
            if (Player1Score >= 5)
            {
                bool gameOver = true;
            }
            else if (Player2Score >= 5)
            {
                bool gameOver = true;
            }

            float dt = GetFrameTime();
            float ballDelta = BALL_SPEED * dt;
            float paddleDelta = PADDLE_SPEED * dt;

            // Move player 1 paddle with key input
            if (IsKeyDown(KEY_W))
                paddle1Position.y -= paddleDelta;
            if (IsKeyDown(KEY_S))
                paddle1Position.y += paddleDelta;
            // Move player 2 paddle with key input
            if (IsKeyDown(KEY_UP))
                paddle2Position.y -= paddleDelta;
            if (IsKeyDown(KEY_DOWN))
                paddle2Position.y += paddleDelta;

            float phh = PADDLE_HEIGHT * 0.5f;
            paddle1Position.y = Clamp(paddle1Position.y, phh, SCREEN_HEIGHT - phh);
            paddle2Position.y = Clamp(paddle2Position.y, phh, SCREEN_HEIGHT - phh);

            // Change the ball's direction on-collision
            Vector2 ballPositionNext = ballPosition + ballDirection * ballDelta;
            Box ballBox = BallBox(ballPositionNext);
            Box paddle1Box = PaddleBox(paddle1Position);
            Box paddle2Box = PaddleBox(paddle2Position);

            // TODO -- increment the scoring player's score after they've touched the ball and the ball goes too far right/left
            if (ballBox.xMin < 0.0f || ballBox.xMax > SCREEN_WIDTH)
            {
                ballDirection.x *= -1.0f;
            }
            if (ballBox.yMin < 0.0f || ballBox.yMax > SCREEN_HEIGHT)
            {
                ballDirection.y *= -1.0f;
            }
            if (BoxOverlap(ballBox, paddle1Box) || BoxOverlap(ballBox, paddle2Box))
            {
                ballDirection.x *= -1.0f;
            }
            if (ballBox.xMin < 0.0f)
            {
                Player1Score++;
                ResetBall(ballPosition, ballDirection);
            }
            if (ballBox.xMax > SCREEN_WIDTH)
            {
                Player2Score++;
                ResetBall(ballPosition, ballDirection);
            }

            // Update ball position after collision resolution, then render
            ballPosition = ballPosition + ballDirection * ballDelta;

            BeginDrawing();
            ClearBackground(BLACK);

            if ((gameOver) && (Player1Score >= 5))
            {
                ClearBackground(WHITE);
                DrawText("Player 1 Wins!", 50, 50, 60, BLUE);
            }
            else if ((gameOver) && (Player2Score >= 5))
            {
                ClearBackground(WHITE);
                DrawText("Player 2 Wins!", 50, 50, 60, RED);
            }
            else 
            {
                DrawBall(ballPosition, WHITE);
                DrawPaddle(paddle1Position, WHITE);
                DrawPaddle(paddle2Position, WHITE);

                // Text format requires you to put a '%i' wherever you want an integer, then add said integer after the comma
                const char* Player1ScoreText = TextFormat("Player 1 Score: %i ", Player1Score);
                const char* Player2ScoreText = TextFormat("Player 2 Score: %i ", Player2Score);

                // We can measure our text for more exact positioning. This puts our score in the center of our screen!
                DrawText(Player1ScoreText, SCREEN_WIDTH * 0.5f - MeasureText(Player1ScoreText, 30) * 1.5f, 50, 30, BLUE);
                DrawText(Player2ScoreText, SCREEN_WIDTH * 0.5f - MeasureText(Player2ScoreText, 30) * -0.5f, 50, 30, RED);

            }

            EndDrawing();
        }

        CloseWindow();
        return 0;
    }
