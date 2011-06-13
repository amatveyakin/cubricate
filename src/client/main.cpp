#include <iostream>
#include <fstream>

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include "common/utils.hpp"
#include "common/game_parameters.hpp"

#include "client/cube_array.hpp"

#include "glwidget.hpp"


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

  GLWidget glwidget;
  QRect screenGeometry = app.desktop ()->screenGeometry (-1);
  QPoint diagonal = screenGeometry.bottomRight() - screenGeometry.topLeft();
  QRect windowGeometry = QRect (diagonal / 4, diagonal * 3 / 4);
  glwidget.setGeometry (windowGeometry);
  glwidget.show ();
//   glwidget.showFullScreen();

  return app.exec ();
}
