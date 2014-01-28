#include <iostream>
#include <fstream>

#include <SFML/Window.hpp>

#include "common/utils.hpp"
#include "common/game_parameters.hpp"

#include "client/client_world.hpp"
#include "client/glwidget.hpp"


const int  approximateTaskbarSize = 32;

const bool showFullscreen = false;


int main (int /*argc*/, char** /*argv*/) {
  sf::Window app;
  sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode ();
  if (showFullscreen) {
    app.create (desktopMode, "Cubricate", sf::Style::Fullscreen);
  }
  else {
    app.create (sf::VideoMode (SCREEN_WIDTH, SCREEN_HEIGHT, 32), "Cubricate");
    app.setPosition (sf::Vector2i ((int (desktopMode.width) - SCREEN_HEIGHT) / 2,
                                   xMax ((int (desktopMode.height) - SCREEN_WIDTH) / 2 - approximateTaskbarSize, 0)));
  }
  app.setMouseCursorVisible (false);
  sf::Mouse::setPosition (sf::Vector2i (app.getSize () / 2u), app);
  app.setActive ();

  GLWidget glWidget (app);

  while (app.isOpen ())
  {
    sf::Event event;
    while (app.pollEvent (event))
    {
      switch (event.type) {
        case sf::Event::Closed:
          app.close ();
          break;

        case sf::Event::Resized:
          glWidget.resizeEvent (event.size);
          break;

        case sf::Event::LostFocus:
          glWidget.lostFocusEvent ();
          break;

        case sf::Event::GainedFocus:
          glWidget.gainedFocusEvent ();
          break;

        case sf::Event::KeyPressed:
          glWidget.keyPressEvent (event.key);
          break;

        case sf::Event::KeyReleased:
          glWidget.keyReleaseEvent (event.key);
          break;

        case sf::Event::MouseButtonPressed:
          glWidget.mousePressEvent (event.mouseButton);
          break;

        case sf::Event::MouseWheelMoved:
          glWidget.mouseWheelEvent (event.mouseWheel);
          break;

        default:
          break;
      }
    }

    glWidget.timerEvent ();
    app.display ();
  }

  return 0;
}
