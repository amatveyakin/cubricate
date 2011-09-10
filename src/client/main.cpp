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
  sf::VideoMode desktopMode = sf::VideoMode::GetDesktopMode();
  if (showFullscreen) {
    app.Create (desktopMode, "Cubricate", sf::Style::Fullscreen);
  }
  else {
    app.Create (sf::VideoMode (SCREEN_WIDTH, SCREEN_HEIGHT, 32), "Cubricate");
    app.SetPosition ((int (desktopMode.Width) - SCREEN_HEIGHT) / 2,
                     xMax ((int (desktopMode.Height) - SCREEN_WIDTH) / 2 - approximateTaskbarSize, 0));
  }
  app.ShowMouseCursor (false);
  sf::Mouse::SetPosition (sf::Vector2i (app.GetWidth() / 2, app.GetHeight() / 2), app);
  app.SetActive();

  GLWidget glWidget (app);

  while (app.IsOpened())
  {
    sf::Event event;
    while (app.PollEvent (event))
    {
      switch (event.Type) {
        case sf::Event::Closed:
          app.Close();
          break;

        case sf::Event::Resized:
          glWidget.resizeEvent (event.Size);
          break;

        case sf::Event::KeyPressed:
          glWidget.keyPressEvent (event.Key);
          break;

        case sf::Event::KeyReleased:
          glWidget.keyReleaseEvent (event.Key);
          break;

        case sf::Event::MouseButtonPressed:
          glWidget.mousePressEvent (event.MouseButton);
          break;

        case sf::Event::MouseWheelMoved:
          glWidget.mouseWheelEvent (event.MouseWheel);
          break;

        default:
          break;
      }
    }

    glWidget.timerEvent();
    app.Display();
  }

  return 0;
}
