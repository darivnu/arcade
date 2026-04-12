/*
** EPITECH PROJECT, 2026
** bootstrap_arcade
** File description:
** SFML_lib
*/

#include "SFML_lib.hpp"

namespace {
    char g_lastInputChar = '\0';
}

static sf::Color toSfColor(Color color)
{
    switch (color) {
        case BLACK:   return sf::Color::Black;
        case RED:     return sf::Color::Red;
        case GREEN:   return sf::Color::Green;
        case BLUE:    return sf::Color::Blue;
        case YELLOW:  return sf::Color::Yellow;
        case MAGENTA: return sf::Color::Magenta;
        case CYAN:    return sf::Color::Cyan;
        case WHITE:   return sf::Color::White;
        default:      return sf::Color::White;
    }
}

SFML_lib::SFML_lib()
{
}

void SFML_lib::init()
{
    if (_window.isOpen())
        _window.close();

    unsigned int winW = WIDTH  * _tileSize + 40;
    unsigned int winH = HEIGHT * _tileSize + 40;

    _window.create(sf::VideoMode(winW, winH), "Arcade - SFML");
    _window.setFramerateLimit(60);

    if (!_font.loadFromFile("assets/Oswald-Bold.ttf"))
        _font.loadFromFile("assets/AmazDooMLeft.ttf");

    _frameBorder.setPosition(19.f, 19.f);
    _frameBorder.setSize(sf::Vector2f(WIDTH * _tileSize + 2, HEIGHT * _tileSize + 2));
    _frameBorder.setFillColor(sf::Color::Transparent);
    _frameBorder.setOutlineColor(sf::Color::White);
    _frameBorder.setOutlineThickness(1.f);

    updateLayout();
}

void SFML_lib::stop()
{
    if (_window.isOpen())
        _window.close();
}

void SFML_lib::updateLayout()
{
    sf::Vector2u size = _window.getSize();
    _originX = (static_cast<int>(size.x) - static_cast<int>(WIDTH  * _tileSize)) / 2;
    _originY = (static_cast<int>(size.y) - static_cast<int>(HEIGHT * _tileSize)) / 2;
}

sf::Vector2f SFML_lib::tileToPixel(int x, int y) const
{
    return sf::Vector2f(
        static_cast<float>(_originX + x * static_cast<int>(_tileSize)),
        static_cast<float>(_originY + y * static_cast<int>(_tileSize))
    );
}

bool SFML_lib::isInsideGameArea(int x, int y) const
{
    return x >= 0 && x < static_cast<int>(_width)
        && y >= 0 && y < static_cast<int>(_height);
}

void SFML_lib::clear()
{
    _window.clear(sf::Color::Black);
}

void SFML_lib::draw()
{
    _window.draw(_frameBorder);
    _window.display();
}

EventType SFML_lib::pollEvents()
{
    sf::Event event;
    EventType result = OTHER;

    while (_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            return QUIT;
        }
        if (event.type == sf::Event::TextEntered) {
            uint32_t c = event.text.unicode;
            if (c >= 32 && c <= 126)
                g_lastInputChar = static_cast<char>(c);
            else if (c == '\r' || c == '\n')
                g_lastInputChar = '\n';
            continue;
        }
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Escape: return QUIT;
                case sf::Keyboard::Q:      return QUIT;
                case sf::Keyboard::W:
                case sf::Keyboard::Up:     return W_KEY;
                case sf::Keyboard::A:
                case sf::Keyboard::Left:   return A_KEY;
                case sf::Keyboard::S:
                case sf::Keyboard::Down:   return S_KEY;
                case sf::Keyboard::D:
                case sf::Keyboard::Right:  return D_KEY;
                case sf::Keyboard::Space:  return SPACE_KEY;
                case sf::Keyboard::M:      return MENU;
                case sf::Keyboard::Enter:  return ENTER;
                case sf::Keyboard::Num1:   return NUM_1;
                case sf::Keyboard::Num2:   return NUM_2;
                case sf::Keyboard::Num3:   return NUM_3;
                case sf::Keyboard::Num4:   return NUM_4;
                default: result = OTHER; break;
            }
        }
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left)  return MOUSE_L;
            if (event.mouseButton.button == sf::Mouse::Right) return MOUSE_R;
        }
    }
    return result;
}

char SFML_lib::getInputChar()
{
    // Pump the event queue so TextEntered events populate g_lastInputChar.
    // This is needed during handle_name_input() which never calls pollEvents().
    sf::Event event;
    while (_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _window.close();
            break;
        }
        if (event.type == sf::Event::TextEntered) {
            uint32_t c = event.text.unicode;
            if (c >= 32 && c <= 126)
                g_lastInputChar = static_cast<char>(c);
            else if (c == '\r' || c == '\n')
                g_lastInputChar = '\n';
        }
    }
    char c = g_lastInputChar;
    g_lastInputChar = '\0';
    return c;
}

void SFML_lib::drawTile(ShapeType shape, Color color, int x, int y)
{
    if (!isInsideGameArea(x, y) || shape == EMPTY)
        return;

    sf::Color sfCol = toSfColor(color);
    sf::Vector2f pos = tileToPixel(x, y);

    if (shape == CIRCLE) {
        float radius = static_cast<float>(_tileSize) / 2.f - 1.f;
        sf::CircleShape circle(radius);
        circle.setFillColor(sfCol);
        circle.setPosition(pos);
        _window.draw(circle);
    } else if (shape == SQUARE) {
        float side = static_cast<float>(_tileSize) - 1.f;
        sf::RectangleShape rect(sf::Vector2f(side, side));
        rect.setFillColor(sfCol);
        rect.setPosition(pos);
        _window.draw(rect);
    }
}

void SFML_lib::drawText(const std::string &text, Color color, int x, int y)
{
    sf::Text sfText;
    sfText.setFont(_font);
    sfText.setString(text);
    sfText.setCharacterSize(16);
    sfText.setFillColor(toSfColor(color));
    sfText.setPosition(
        static_cast<float>(_originX + x * static_cast<int>(_tileSize)),
        static_cast<float>(_originY + y * static_cast<int>(_tileSize))
    );
    _window.draw(sfText);
}

void SFML_lib::drawSprite(const Sprite &sprite, int x, int y)
{
    if (sprite.path.empty() || !isInsideGameArea(x, y)) {
        drawTile(sprite.fallback, sprite.fallbackColor, x, y);
        return;
    }
    if (_failedTextures.count(sprite.path)) {
        drawTile(sprite.fallback, sprite.fallbackColor, x, y);
        return;
    }
    if (!_textures.count(sprite.path)) {
        sf::Texture tex;
        if (!tex.loadFromFile("assets/" + sprite.path)) {
            _failedTextures.insert(sprite.path);
            drawTile(sprite.fallback, sprite.fallbackColor, x, y);
            return;
        }
        _textures[sprite.path] = std::move(tex);
    }

    sf::Sprite sfSprite(_textures[sprite.path]);

    if (sprite.srcW > 0 && sprite.srcH > 0) {
        sfSprite.setTextureRect(sf::IntRect(
            sprite.srcX, sprite.srcY,
            sprite.srcW, sprite.srcH
        ));
        float scaleX = static_cast<float>(_tileSize) / static_cast<float>(sprite.srcW);
        float scaleY = static_cast<float>(_tileSize) / static_cast<float>(sprite.srcH);
        sfSprite.setScale(scaleX, scaleY);
    } else {
        sf::Vector2u texSize = _textures[sprite.path].getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            float scaleX = static_cast<float>(_tileSize) / static_cast<float>(texSize.x);
            float scaleY = static_cast<float>(_tileSize) / static_cast<float>(texSize.y);
            sfSprite.setScale(scaleX, scaleY);
        }
    }

    sfSprite.setPosition(tileToPixel(x, y));
    _window.draw(sfSprite);
}

extern "C" {

IDisplayModule *create()
{
    return new SFML_lib();
}

void destroy(IDisplayModule *instance)
{
    instance->stop();
    delete instance;
}

}
