#include <cmath>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;
const int CURVE_SAMPLES = 100;
const float CONTROL_POINT_RADIUS = 7.0f;
const float SQUARE_SIZE = 18.0f;
const float ANIMATION_DURATION_SECONDS = 4.0f;

using Point2D = sf::Vector2f;

// Part 1: four cubic Bézier control points, ordered from the curve's start to end.
std::vector<Point2D> controlPoints = {
    {100.0f, 600.0f},
    {200.0f, 100.0f},
    {600.0f, 100.0f},
    {700.0f, 600.0f},
};

// Returns the point on a cubic Bézier curve at t in [0, 1].
Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) {
    const float u = 1.0f - t;
    const float b0 = u * u * u;
    const float b1 = 3.0f * u * u * t;
    const float b2 = 3.0f * u * t * t;
    const float b3 = t * t * t;

    return b0 * pts[0] + b1 * pts[1] + b2 * pts[2] + b3 * pts[3];
}

// Returns the tangent (derivative) of a cubic Bézier curve at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) {
    const float u = 1.0f - t;

    return 3.0f * u * u * (pts[1] - pts[0]) + 6.0f * u * t * (pts[2] - pts[1]) +
           3.0f * t * t * (pts[3] - pts[2]);
}

sf::Clock animationClock;

// Part 3: -1 means no control point is currently being dragged.
int selectedPointIndex = -1;

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                const Point2D mousePosition = {
                    static_cast<float>(mouse->position.x),
                    static_cast<float>(mouse->position.y),
                };

                float closestDistanceSquared = std::numeric_limits<float>::max();
                for (std::size_t i = 0; i < controlPoints.size(); ++i) {
                    const Point2D offset = controlPoints[i] - mousePosition;
                    const float distanceSquared = offset.x * offset.x + offset.y * offset.y;
                    if (distanceSquared < closestDistanceSquared) {
                        closestDistanceSquared = distanceSquared;
                        selectedPointIndex = static_cast<int>(i);
                    }
                }
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                selectedPointIndex = -1;
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            if (selectedPointIndex >= 0) {
                controlPoints[static_cast<std::size_t>(selectedPointIndex)] = {
                    static_cast<float>(mouse->position.x),
                    static_cast<float>(mouse->position.y),
                };
            }

            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);

    // Part 3: draw the two control handles before drawing the curve.
    sf::VertexArray handles(sf::PrimitiveType::Lines, 4);
    handles[0] = sf::Vertex{controlPoints[0], sf::Color(100, 100, 100)};
    handles[1] = sf::Vertex{controlPoints[1], sf::Color(100, 100, 100)};
    handles[2] = sf::Vertex{controlPoints[2], sf::Color(100, 100, 100)};
    handles[3] = sf::Vertex{controlPoints[3], sf::Color(100, 100, 100)};
    window.draw(handles);

    // Part 1: approximate the curve with short lines between sampled points.
    sf::VertexArray curve(sf::PrimitiveType::LineStrip, CURVE_SAMPLES + 1);
    for (int i = 0; i <= CURVE_SAMPLES; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(CURVE_SAMPLES);
        curve[i].position = getPoint(controlPoints, t);
        curve[i].color = sf::Color::Cyan;
    }
    window.draw(curve);

    // Draw the four control points after the curve so they remain easy to see.
    sf::CircleShape controlPoint(CONTROL_POINT_RADIUS);
    controlPoint.setOrigin({CONTROL_POINT_RADIUS, CONTROL_POINT_RADIUS});
    controlPoint.setFillColor(sf::Color::Yellow);
    for (const Point2D& point : controlPoints) {
        controlPoint.setPosition(point);
        window.draw(controlPoint);
    }

    // Part 2: move a square along the curve and align it with the tangent.
    const float elapsedSeconds = animationClock.getElapsedTime().asSeconds();
    const float t = std::fmod(elapsedSeconds, ANIMATION_DURATION_SECONDS) /
                    ANIMATION_DURATION_SECONDS;
    const Point2D position = getPoint(controlPoints, t);
    const Point2D slope = getSlope(controlPoints, t);
    const float angleDegrees = std::atan2(slope.y, slope.x) * 180.0f / std::acos(-1.0f);

    sf::RectangleShape square({SQUARE_SIZE, SQUARE_SIZE});
    square.setOrigin({SQUARE_SIZE / 2.0f, SQUARE_SIZE / 2.0f});
    square.setPosition(position);
    square.setRotation(sf::degrees(angleDegrees));
    square.setFillColor(sf::Color::Magenta);
    window.draw(square);


    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.


    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
