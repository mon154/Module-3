#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <cstdint>
#include <chrono>
#include <functional>
#include <iostream>

const std::uint16_t WINDOW_WIDTH = 1600;
const std::uint16_t WINDOW_HEIGHT = 900;

const float UPDATE_MS = 33;

const float BALL_RADIUS = 10;
const float BALL_VELOCITY = 400;
const float BALL_VEL_INCR = 60;

const float PADDLE_WIDTH = 10;
const float PADDLE_LENGTH = 50;
const float PADDLE_PADDING = 20;
const float PADDLE_SPEED = 400;

const float COURT_MARGIN = 10;
const float COURT_OUTLINE_WIDTH = 5;

enum class GAME_STATE : std::uint_fast8_t
{
    MENU,
    IN_GAME,
    EXIT
};

enum class PLAY_STATE : std::uint_fast8_t
{
    SERVE_PLAYER_ONE,
    SERVE_PLAYER_TWO,
    TOWARD_PLAYER_ONE,
    TOWARD_PLAYER_TWO
};

enum class MOUSE_STATE : std::uint_fast8_t
{
    UP,
    DOWN
};

struct Vector2D
{
    float x;
    float y;
};

struct RectangleShape
{
    float x;
    float y;
    float width;
    float height;
};

class Court
{
public:
    Court(const RectangleShape dimensions)
        :
        m_dimensions(dimensions)
    {

    }

    const RectangleShape& GetDimensions() const
    {
        return m_dimensions;
    }

private:
    RectangleShape m_dimensions;
};

class Paddle
{
public:
    Paddle(const RectangleShape startingPosition)
        :
        m_rect(startingPosition)
    {
    }

    const RectangleShape& GetPositionSize() const
    {
        return m_rect;
    }

    void SetPositionSize(const RectangleShape newPositionSize)
    {
        m_rect = newPositionSize;
    }

    void SetPosition(const Vector2D newPosition)
    {
        m_rect.x = newPosition.x;
        m_rect.y = newPosition.y;
    }

private:
    RectangleShape m_rect;
};

class Ball
{
public:
    Ball(const Vector2D startPosition, const float radius)
        :
        m_position(startPosition),
        m_radius(radius),
        m_velocity({ 0,0 })
    {
    }

    const Vector2D& GetPosition() const
    {
        return m_position;
    }

    const float& GetRadius() const
    {
        return m_radius;
    }

    const Vector2D& GetVelocity() const
    {
        return m_velocity;
    }

    void SetPosition(const Vector2D newPosition)
    {
        m_position = newPosition;
    }

    void SetVelocity(const Vector2D newVelocity)
    {
        m_velocity = newVelocity;
    }

private:
    Vector2D m_position;
    float m_radius;
    Vector2D m_velocity;
};

class GameRenderer
{
public:

    static bool Init(sf::RenderTarget* target, sf::Font* font)
    {
        m_target = target;
        m_font = font;
    }

    static void Render(const float& elapsedMilliseconds,
        const Paddle& playerOne,
        const Paddle& playerTwo,
        const Ball& ball,
        const Court& court,
        const std::uint_fast8_t& p1Score,
        const std::uint_fast8_t& p2Score)
    {
        sf::RectangleShape courtShape;
        const RectangleShape& cShape = court.GetDimensions();
        courtShape.setPosition({ cShape.x,cShape.y });
        courtShape.setSize({ cShape.width,cShape.height });
        courtShape.setFillColor(sf::Color::Transparent);
        courtShape.setOutlineColor(sf::Color::White);
        courtShape.setOutlineThickness(-COURT_OUTLINE_WIDTH);

        m_target->draw(courtShape);

        courtShape.setPosition({ WINDOW_WIDTH / 2 - COURT_OUTLINE_WIDTH / 2,COURT_MARGIN });
        courtShape.setSize({ COURT_OUTLINE_WIDTH,WINDOW_HEIGHT - COURT_MARGIN * 2 });
        m_target->draw(courtShape);

        sf::RectangleShape paddleShape;

        const RectangleShape& p1Shape = playerOne.GetPositionSize();
        paddleShape.setPosition({ p1Shape.x,p1Shape.y });
        paddleShape.setSize({ p1Shape.width,p1Shape.height });
        paddleShape.setFillColor(sf::Color::White);
        m_target->draw(paddleShape);

        const RectangleShape& p2Shape = playerTwo.GetPositionSize();
        paddleShape.setPosition({ p2Shape.x,p2Shape.y });
        paddleShape.setSize({ p2Shape.width,p2Shape.height });
        paddleShape.setFillColor(sf::Color::White);
        m_target->draw(paddleShape);

        sf::CircleShape ballShape;
        const Vector2D& ballPosition = ball.GetPosition();
        const float& ballRadius = ball.GetRadius();
        ballShape.setPosition({ ballPosition.x - BALL_RADIUS,ballPosition.y - BALL_RADIUS });
        ballShape.setRadius(ballRadius);
        ballShape.setFillColor(sf::Color::White);
        m_target->draw(ballShape);

        sf::Text score(std::to_string(p1Score) + "   " + std::to_string(p2Score), *m_font, 40);
        sf::FloatRect bounds = score.getLocalBounds();
        score.setPosition({ WINDOW_WIDTH / 2 - bounds.width / 2,COURT_MARGIN + COURT_OUTLINE_WIDTH + 5 });
        m_target->draw(score);
    }

private:
    static sf::RenderTarget* m_target;
    static sf::Font* m_font;
};

sf::RenderTarget* GameRenderer::m_target = nullptr;
sf::Font* GameRenderer::m_font = nullptr;

class PongGame
{
public:
    PongGame(const std::uint_fast8_t scoreToWin, sf::RenderTarget& target, sf::Font& font)
        :
        m_playerOneScore(0),
        m_playerTwoScore(0),
        m_maxScore(scoreToWin),
        m_court({
            COURT_MARGIN,
            COURT_MARGIN,
            WINDOW_WIDTH - COURT_MARGIN * 2,
            WINDOW_HEIGHT - COURT_MARGIN * 2
            }),
        m_ball({
                WINDOW_WIDTH / 2,
                WINDOW_HEIGHT / 2
            },
            BALL_RADIUS
        ),
        m_playerOne({
            COURT_MARGIN + PADDLE_PADDING,
            WINDOW_HEIGHT / 2 - (PADDLE_LENGTH / 2),
            PADDLE_WIDTH,
            PADDLE_LENGTH
            }),
        m_playerTwo({
            WINDOW_WIDTH - COURT_MARGIN - PADDLE_PADDING - PADDLE_WIDTH,
            WINDOW_HEIGHT / 2 - (PADDLE_LENGTH / 2),
            PADDLE_WIDTH,
            PADDLE_LENGTH
            }),
        m_playState(PLAY_STATE::SERVE_PLAYER_ONE)
    {
        GameRenderer::Init(&target, &font);
    }

    GAME_STATE Update(const float elapsedMilliseconds)
    {
        float timeMultiplier = elapsedMilliseconds / 1000.0f;

        const RectangleShape& paddle1 = m_playerOne.GetPositionSize();
        const RectangleShape& paddle2 = m_playerTwo.GetPositionSize();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            m_playerOne.SetPosition({ paddle1.x,paddle1.y - PADDLE_SPEED * timeMultiplier });
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
            m_playerOne.SetPosition({ paddle1.x,paddle1.y + PADDLE_SPEED * timeMultiplier });

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
            m_playerTwo.SetPosition({ paddle2.x,paddle2.y - PADDLE_SPEED * timeMultiplier });
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period))
            m_playerTwo.SetPosition({ paddle2.x,paddle2.y + PADDLE_SPEED * timeMultiplier });

        switch (m_playState)
        {
        case PLAY_STATE::SERVE_PLAYER_ONE:
        {
            m_ball.SetVelocity({ 0,0 });
            const RectangleShape& paddle = m_playerOne.GetPositionSize();
            m_ball.SetPosition({ paddle.x + PADDLE_WIDTH,paddle.y + PADDLE_LENGTH / 2 });

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                m_ball.SetVelocity({ BALL_VELOCITY,0 });
                m_playState = PLAY_STATE::TOWARD_PLAYER_TWO;
            }
            break;
        }
        case PLAY_STATE::SERVE_PLAYER_TWO:
        {
            m_ball.SetVelocity({ 0,0 });
            const RectangleShape& paddle = m_playerTwo.GetPositionSize();
            m_ball.SetPosition({ paddle.x - BALL_RADIUS,paddle.y + PADDLE_LENGTH / 2 });

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                m_ball.SetVelocity({ -BALL_VELOCITY,0 });
                m_playState = PLAY_STATE::TOWARD_PLAYER_ONE;
            }
            break;
        }
        default:
            break;
        }

        Vector2D ballPos = m_ball.GetPosition();
        Vector2D ballVelocity = m_ball.GetVelocity();
        const RectangleShape& courtShape = m_court.GetDimensions();

        ballPos.x += ballVelocity.x * timeMultiplier;
        ballPos.y += ballVelocity.y * timeMultiplier;

        m_ball.SetPosition(ballPos);

        switch (m_playState)
        {
        case PLAY_STATE::TOWARD_PLAYER_ONE:
        {
            if (ballPos.x - BALL_RADIUS > paddle1.x + PADDLE_WIDTH)
                break; // ball hasn't reached player 1

            if (ballPos.y + BALL_RADIUS >= paddle1.y && ballPos.y - BALL_RADIUS <= paddle1.y + PADDLE_LENGTH)
            {
                m_ball.SetPosition({ paddle1.x + PADDLE_WIDTH + BALL_RADIUS + 1, ballPos.y });

                ballVelocity.x = -ballVelocity.x;

                if (ballPos.y + BALL_RADIUS <= paddle1.y + PADDLE_LENGTH / 3)
                    ballVelocity.y -= BALL_VELOCITY / 2;
                else if (ballPos.y - BALL_RADIUS >= paddle1.y + PADDLE_LENGTH / 3 * 2)
                    ballVelocity.y += BALL_VELOCITY / 2;

                if (ballVelocity.x > 0)
                    ballVelocity.x += BALL_VEL_INCR;
                else
                    ballVelocity.x -= BALL_VEL_INCR;

                if (ballVelocity.y > 0)
                    ballVelocity.y += BALL_VEL_INCR;
                else
                    ballVelocity.y -= BALL_VEL_INCR;

                m_ball.SetVelocity(ballVelocity);
                m_playState = PLAY_STATE::TOWARD_PLAYER_TWO;

                break;
            }

            if (ballPos.x + BALL_RADIUS < paddle1.x)
            {
                ++m_playerTwoScore;
                m_playState = PLAY_STATE::SERVE_PLAYER_ONE;
            }

            break;
        }
        case PLAY_STATE::TOWARD_PLAYER_TWO:
        {
            if (ballPos.x + BALL_RADIUS < paddle2.x)
                break;

            if (ballPos.y + BALL_RADIUS >= paddle2.y && ballPos.y - BALL_RADIUS <= paddle2.y + PADDLE_LENGTH)
            {
                m_ball.SetPosition({ paddle2.x - BALL_RADIUS - 1, ballPos.y });

                ballVelocity.x = -ballVelocity.x;

                if (ballPos.y + BALL_RADIUS <= paddle2.y + PADDLE_LENGTH / 3)
                    ballVelocity.y -= BALL_VELOCITY / 2;
                else if (ballPos.y - BALL_RADIUS >= paddle2.y + PADDLE_LENGTH / 3 * 2)
                    ballVelocity.y += BALL_VELOCITY / 2;

                if (ballVelocity.x > 0)
                    ballVelocity.x += BALL_VEL_INCR;
                else
                    ballVelocity.x -= BALL_VEL_INCR;

                if (ballVelocity.y > 0)
                    ballVelocity.y += BALL_VEL_INCR;
                else
                    ballVelocity.y -= BALL_VEL_INCR;

                m_ball.SetVelocity(ballVelocity);
                m_playState = PLAY_STATE::TOWARD_PLAYER_ONE;

                break;
            }

            if (ballPos.x - BALL_RADIUS > paddle2.x + PADDLE_WIDTH)
            {
                ++m_playerOneScore;
                m_playState = PLAY_STATE::SERVE_PLAYER_TWO;
            }

            break;
        }
        default:
            break;
        }

        if (ballPos.y <= courtShape.y)
        {
            m_ball.SetPosition({ ballPos.x,courtShape.y });
            m_ball.SetVelocity({ ballVelocity.x,-ballVelocity.y });
        }
        else if (ballPos.y >= courtShape.y + courtShape.height)
        {
            m_ball.SetPosition({ ballPos.x,courtShape.y + courtShape.height });
            m_ball.SetVelocity({ ballVelocity.x,-ballVelocity.y });
        }

        if (m_playerOneScore >= m_maxScore || m_playerTwoScore >= m_maxScore)
            return GAME_STATE::MENU;

        return GAME_STATE::IN_GAME;
    }

    void Render(const float elapsedMilliseconds) const
    {
        GameRenderer::Render(elapsedMilliseconds, m_playerOne, m_playerTwo, m_ball, m_court, m_playerOneScore, m_playerTwoScore);
    }

private:
    std::uint_fast8_t m_playerOneScore;
    std::uint_fast8_t m_playerTwoScore;
    std::uint_fast8_t m_maxScore;
    const Court m_court;
    Ball m_ball;
    Paddle m_playerOne;
    Paddle m_playerTwo;
    PLAY_STATE m_playState;
};

class Button
{
public:
    typedef std::function<void(void)> CallbackFunc;

    enum class STATE : std::uint_fast8_t
    {
        UP,
        DOWN,
        HOVER
    };

    Button(const std::string text, const RectangleShape positionAndSize)
        :
        m_text(text),
        m_positionAndSize(positionAndSize),
        m_colorUp(sf::Color::Black),
        m_colorDown(sf::Color::Red),
        m_colorHover(sf::Color::Yellow),
        m_callback([]() {}),
        m_state(STATE::UP)
    {
    }

    const RectangleShape& GetPositionAndSize() const
    {
        return m_positionAndSize;
    }

    void SetPositionAndSize(const RectangleShape newPositionAndSize)
    {
        m_positionAndSize = newPositionAndSize;
    }

    void SetPosition(const Vector2D newPosition)
    {
        m_positionAndSize.x = newPosition.x;
        m_positionAndSize.y = newPosition.y;
    }

    void SetSize(const Vector2D newSize)
    {
        m_positionAndSize.width = newSize.x;
        m_positionAndSize.height = newSize.y;
    }

    bool HandleInput(const Vector2D& mousePosition, const MOUSE_STATE& mouseState)
    {
        if (mousePosition.x >= m_positionAndSize.x &&
            mousePosition.x <= m_positionAndSize.x + m_positionAndSize.width &&
            mousePosition.y >= m_positionAndSize.y &&
            mousePosition.y <= m_positionAndSize.y + m_positionAndSize.height)
        {
            m_state = STATE::HOVER;
        }
        else
            m_state = STATE::UP;

        if (mouseState == MOUSE_STATE::DOWN && m_state == STATE::HOVER)
        {
            m_state = STATE::DOWN;
            m_callback();
            return true;
        }

        return false;
    }

    void SetColors(const sf::Color upColor, const sf::Color downColor, const sf::Color hoverColor)
    {
        m_colorUp = upColor;
        m_colorHover = downColor;
        m_colorHover = hoverColor;
    }

    void SetCallback(const CallbackFunc callback)
    {
        m_callback = callback;
    }

    const Button::STATE& GetState() const
    {
        return m_state;
    }

    void SetState(const Button::STATE newState)
    {
        m_state = newState;
    }

    void Render(sf::RenderTarget& target, const sf::Font& font) const
    {
        sf::Text buttonText(m_text, font, 60);
        buttonText.setColor(m_colorUp);
        if (m_state == STATE::DOWN)
            buttonText.setColor(m_colorDown);
        else if (m_state == STATE::HOVER)
            buttonText.setColor(m_colorHover);

        buttonText.setPosition({ m_positionAndSize.x,m_positionAndSize.y });

        sf::RectangleShape bg;
        bg.setPosition({ m_positionAndSize.x,m_positionAndSize.y });
        bg.setSize({ m_positionAndSize.width,m_positionAndSize.height });
        bg.setFillColor(sf::Color::White);

        target.draw(bg);
        target.draw(buttonText);
    }

private:

    std::string m_text;
    RectangleShape m_positionAndSize;
    sf::Color m_colorUp;
    sf::Color m_colorDown;
    sf::Color m_colorHover;
    CallbackFunc m_callback;
    STATE m_state;
};

class PongMenu
{
public:
    PongMenu(sf::RenderTarget& target, sf::Font& font)
        :
        m_target(target),
        m_font(font),
        m_playButton("PLAY", { WINDOW_WIDTH / 2,WINDOW_HEIGHT / 2,140,65 }),
        m_exitButton("EXIT", { WINDOW_WIDTH / 2,WINDOW_HEIGHT / 2 + 100,130,65 }),
        m_shouldExit(false),
        m_shouldStart(false)
    {
        m_playButton.SetCallback([this]() {m_shouldStart = true; });
        m_exitButton.SetCallback([this]() {m_shouldExit = true; });
    }

    GAME_STATE Update(const float elapsedMilliseconds, const Vector2D& mousePos)
    {
        MOUSE_STATE state = MOUSE_STATE::UP;
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
            sf::Mouse::isButtonPressed(sf::Mouse::Middle) ||
            sf::Mouse::isButtonPressed(sf::Mouse::Right))
            state = MOUSE_STATE::DOWN;

        m_playButton.HandleInput(mousePos, state);
        m_exitButton.HandleInput(mousePos, state);

        if (m_shouldExit)
            return GAME_STATE::EXIT;
        if (m_shouldStart)
            return GAME_STATE::IN_GAME;
        return GAME_STATE::MENU;
    }

    void Render(const float elapsedMilliseconds) const
    {
        m_playButton.Render(m_target, m_font);
        m_exitButton.Render(m_target, m_font);
    }

    void Reset()
    {
        m_shouldExit = false;
        m_shouldStart = false;
    }

private:

    sf::RenderTarget& m_target;
    sf::Font& m_font;

    Button m_playButton;
    Button m_exitButton;

    bool m_shouldExit;
    bool m_shouldStart;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pong");

    sf::Font font;
    if (!font.loadFromFile("SourceSansPro-Regular.otf"))
    {
        std::cerr << "could not load font " << std::endl;
        return 0;
    }

    GAME_STATE gameState = GAME_STATE::MENU;

    PongGame pong(3, window, font);
    PongMenu menu(window, font);

    std::chrono::system_clock::time_point lastTime = std::chrono::system_clock::now();
    float frameLag = 0;

    while (window.isOpen() && gameState != GAME_STATE::EXIT)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
        std::chrono::milliseconds elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
        lastTime = currentTime;
        frameLag += elapsedTime.count();

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        while (frameLag >= UPDATE_MS)
        {
            frameLag -= UPDATE_MS;
            if (gameState == GAME_STATE::MENU)
                gameState = menu.Update(UPDATE_MS, { mousePos.x,mousePos.y });
            else
            {
                gameState = pong.Update(UPDATE_MS);
                if (gameState == GAME_STATE::MENU)
                    menu.Reset();
            }
        }

        window.clear();

        if (gameState == GAME_STATE::MENU)
            menu.Render(elapsedTime.count());
        else
            pong.Render(elapsedTime.count());

        window.display();
    }

    window.close();

    return 0;
}

