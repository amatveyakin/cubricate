#include <iostream>
#include <fstream>

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include "common/utils.hpp"
#include "common/game_parameters.hpp"

// #include "client/cube_array.hpp"
#include "client/glwidget.hpp"


const bool showFullscreen = false;


// // 0 means success
// int loadGameMap (Visible_cube_set <GLfloat, GLfloat>& cubeArray) {
//   std::ifstream height_map ("resources/height_map" + stringify (TREE_HEIGHT) + ".txt");
//   if (!height_map.is_open ()) {
//     std::cout << "Unable to open height map!\n";
//     return 1;
//   }
//   for (int x = 0; x < MAP_SIZE; ++x) {
//     for (int y = 0; y < MAP_SIZE; ++y) {
//       int height;
//       height_map >> height;
//       if (height > MAP_SIZE / 2) {
//         cubeArray.add_cube (x, y, height - 1, 66);
//         height--;
//       }
//       for (int z = 0; z < height; ++z) {
//         cubeArray.add_cube (x, y, z, 2);
//       }
//     }
//   }
//   std::cout << "n_cubes = " << cubeArray.n_cubes () << std::endl;
//   return 0;
// }


int main (int argc, char** argv) {
  QApplication app (argc, argv);

//   Visible_cube_set <GLfloat, GLfloat> cubeArray (MAP_SIZE, MAP_SIZE, MAP_SIZE);
//   if (!loadGameMap (cubeArray))
//     return 1;

  QGLFormat glFormat = QGLFormat::defaultFormat ();
  glFormat.setSampleBuffers (true);
  QGLFormat::setDefaultFormat (glFormat);
  if (!QGLFormat::hasOpenGL ()) {
    std::cout << "Your system does not support OpenGL :-(" << std::endl;
    return 1;
  }

  GLWidget glWidget;
  if (!glWidget.format ().sampleBuffers ())
    std::cout << "Your system does not have sample buffer support :-(" << std::endl;

  if (showFullscreen) {
    glWidget.showFullScreen();
  }
  else {
    QRect screenGeometry = app.desktop ()->screenGeometry (-1);
//     QPoint diagonal = screenGeometry.bottomRight () - screenGeometry.topLeft ();
    //int size = (screenGeometry.bottom () - screenGeometry.top ()) * 15 / 16;
    glWidget.setGeometry (QRect (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT).translated ((screenGeometry.width () - SCREEN_WIDTH) / 2, (screenGeometry.height () - SCREEN_HEIGHT) / 2));
    glWidget.show ();
  }

  return app.exec ();
}
