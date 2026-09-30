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
Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t, std::size_t startIndex = 0) {
    const float u = 1.0f - t;
    const float b0 = u * u * u;
    const float b1 = 3.0f * u * u * t;
    const float b2 = 3.0f * u * t * t;
    const float b3 = t * t * t;

    return b0 * pts[startIndex] + b1 * pts[startIndex + 1] + b2 * pts[startIndex + 2] +
           b3 * pts[startIndex + 3];
}

// Returns the tangent (derivative) of a cubic Bézier curve at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t, std::size_t startIndex = 0) {
    const float u = 1.0f - t;

    return 3.0f * u * u * (pts[startIndex + 1] - pts[startIndex]) +
           6.0f * u * t * (pts[startIndex + 2] - pts[startIndex + 1]) +
           3.0f * t * t * (pts[startIndex + 3] - pts[startIndex + 2]);
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
                const std::size_t selected = static_cast<std::size_t>(selectedPointIndex);
                controlPoints[selected] = {
                    static_cast<float>(mouse->position.x),
                    static_cast<float>(mouse->position.y),
                };

                // If an incoming handle moves, mirror the next outgoing handle
                // through the shared endpoint, preserving its previous length.
                if (selected % 3 == 2 && selected + 2 < controlPoints.size()) {
                    const std::size_t sharedEndpoint = selected + 1;
                    const std::size_t outgoingHandle = selected + 2;
                    const Point2D incoming = controlPoints[sharedEndpoint] - controlPoints[selected];
                    const Point2D oldOutgoing =
                        controlPoints[outgoingHandle] - controlPoints[sharedEndpoint];
                    const float incomingLength =
                        std::sqrt(incoming.x * incoming.x + incoming.y * incoming.y);
                    const float outgoingLength =
                        std::sqrt(oldOutgoing.x * oldOutgoing.x + oldOutgoing.y * oldOutgoing.y);

                    if (incomingLength > 0.0f) {
                        controlPoints[outgoingHandle] =
                            controlPoints[sharedEndpoint] + incoming * (outgoingLength / incomingLength);
                    }
                }
            }
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Add || key->code == sf::Keyboard::Key::Equal) {
                const Point2D endpoint = controlPoints.back();
                const Point2D lastHandle = controlPoints[controlPoints.size() - 2];
                Point2D direction = endpoint - lastHandle;
                if (direction.x == 0.0f && direction.y == 0.0f) {
                    direction = {100.0f, 0.0f};
                }

                controlPoints.push_back(endpoint + direction);
                controlPoints.push_back(endpoint + 2.0f * direction);
                controlPoints.push_back(endpoint + 3.0f * direction);
            } else if ((key->code == sf::Keyboard::Key::Subtract ||
                        key->code == sf::Keyboard::Key::Hyphen) &&
                       controlPoints.size() > 4) {
                controlPoints.resize(controlPoints.size() - 3);
                selectedPointIndex = -1;
            }
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);

    const std::size_t segmentCount = (controlPoints.size() - 1) / 3;

    // Part 4: draw the two handles for every cubic Bézier segment.
    sf::VertexArray handles(sf::PrimitiveType::Lines, segmentCount * 4);
    for (std::size_t segment = 0; segment < segmentCount; ++segment) {
        const std::size_t point = segment * 3;
        const std::size_t vertex = segment * 4;
        handles[vertex] = sf::Vertex{controlPoints[point], sf::Color(100, 100, 100)};
        handles[vertex + 1] = sf::Vertex{controlPoints[point + 1], sf::Color(100, 100, 100)};
        handles[vertex + 2] = sf::Vertex{controlPoints[point + 2], sf::Color(100, 100, 100)};
        handles[vertex + 3] = sf::Vertex{controlPoints[point + 3], sf::Color(100, 100, 100)};
    }
    window.draw(handles);

    // Draw every connected cubic Bézier segment.
    for (std::size_t segment = 0; segment < segmentCount; ++segment) {
        sf::VertexArray curve(sf::PrimitiveType::LineStrip, CURVE_SAMPLES + 1);
        const std::size_t start = segment * 3;
        for (int i = 0; i <= CURVE_SAMPLES; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(CURVE_SAMPLES);
            curve[i].position = getPoint(controlPoints, t, start);
            curve[i].color = sf::Color::Cyan;
        }
        window.draw(curve);
    }

    // Draw control points after the curve so they remain easy to see.
    sf::CircleShape controlPoint(CONTROL_POINT_RADIUS);
    controlPoint.setOrigin({CONTROL_POINT_RADIUS, CONTROL_POINT_RADIUS});
    controlPoint.setFillColor(sf::Color::Yellow);
    for (const Point2D& point : controlPoints) {
        controlPoint.setPosition(point);
        window.draw(controlPoint);
    }

    // Part 2: move a square repeatedly across all curve segments.
    const float elapsedSeconds = animationClock.getElapsedTime().asSeconds();
    const float progress = std::fmod(elapsedSeconds, ANIMATION_DURATION_SECONDS) /
                           ANIMATION_DURATION_SECONDS;
    const float segmentProgress = progress * static_cast<float>(segmentCount);
    const std::size_t squareSegment =
        static_cast<std::size_t>(segmentProgress) < segmentCount
            ? static_cast<std::size_t>(segmentProgress)
            : segmentCount - 1;
    const float t = segmentProgress - static_cast<float>(squareSegment);
    const std::size_t start = squareSegment * 3;
    const Point2D position = getPoint(controlPoints, t, start);
    const Point2D slope = getSlope(controlPoints, t, start);
    const float angleDegrees = std::atan2(slope.y, slope.x) * 180.0f / std::acos(-1.0f);

    sf::RectangleShape square({SQUARE_SIZE, SQUARE_SIZE});
    square.setOrigin({SQUARE_SIZE / 2.0f, SQUARE_SIZE / 2.0f});
    square.setPosition(position);
    square.setRotation(sf::degrees(angleDegrees));
    square.setFillColor(sf::Color::Magenta);
    window.draw(square);

    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.

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
