#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <cstdint>
#include <chrono>
#include <functional>
#include <iostream>

// global variables
const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 600;

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

//classes

//point class
class Point {
protected:
    float x = 0.0;
    float y = 0.0;
};

//velocity class
class Velocity {
protected:
    float dx = 0.0;
    float dy = 0.0;
};

class Ball{
public:
    //attributes
    int center_x = 0;
    int center_y = rand() % 599 + 1;
    int velocity_dx = rand() % 4 + 4;
    int velocity_dy = rand() % -16 + 8;
    int radius = BALL_VELOCITY;

    //methods
    void draw_ball() {  // Method/function defined inside the class
       sf::CircleShape circle(BALL_RADIUS);
       window.draw(circle);
    }
};





int main()
{
    //creates the window
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pong");
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // clear the window with black color
        window.clear(sf::Color::Black);

        Ball ball1;
        ball1.draw_ball();
        // draw everything here...
        // window.draw(...);

        // end the current frame
        window.display();
    }


    return 0;
}