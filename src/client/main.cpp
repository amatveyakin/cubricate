#include <iostream>
#include <fstream>

#include <SFML/Window.hpp>

#include "common/utils.hpp"
#include "common/game_parameters.hpp"

#include "client/client_world.hpp"
#include "client/glwidget.hpp"


const bool showFullscreen = false;


int main (int /*argc*/, char** /*argv*/) {
  sf::Window app;
  sf::VideoMode desktopMode = sf::VideoMode::GetDesktopMode();
  if (showFullscreen) {
    app.Create (desktopMode, "Cubricate", sf::Style::Fullscreen);
  }
  else {
    app.Create (sf::VideoMode (SCREEN_WIDTH, SCREEN_HEIGHT, 32), "Cubricate");
    app.SetPosition ((desktopMode.Width - SCREEN_WIDTH) / 2, (desktopMode.Height - SCREEN_HEIGHT) / 2);
  }
  app.ShowMouseCursor (false);
  app.SetCursorPosition (app.GetWidth() / 2, app.GetHeight() / 2);
  app.SetActive();

  GLWidget glWidget (app);
  renderingEngine = &glWidget;
  glWidget.initializeGL();

  while (app.IsOpened())
  {
    sf::Event event;
    while (app.GetEvent(event))
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

    app.SetActive();            // It's useless here because active window is always the same,
    glWidget.timerEvent();
    app.Display();
  }

  return 0;
}
