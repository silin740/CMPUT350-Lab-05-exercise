#include <iostream>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;
const int CURVE_SAMPLES = 100;
const float CONTROL_POINT_RADIUS = 7.0f;

using Point2D = sf::Vector2f;

// Part 1: four cubic Bézier control points, ordered from the curve's start to end.
const std::vector<Point2D> controlPoints = {
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

// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) { return Point2D{}; }

// TODO: (Part 2) Track animation time for the square moving along the curve.
// TODO: (Part 3) Track the index of the control point being dragged.

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
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

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======

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
