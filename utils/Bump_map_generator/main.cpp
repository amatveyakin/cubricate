#include <cmath>
#include <cassert>
#include <iostream>

#include <QImage>


void fillFunction (double x, double y, double& resultX, double& resultY, double& resultZ) {
  resultX = -2 * M_PI * cos (2 * M_PI * (x + y));
  resultY = resultX;
  resultZ = 5;
  double norm = sqrt (resultX * resultX  +  resultY * resultY  +  resultZ * resultZ);
  resultX /= norm;
  resultY /= norm;
  resultZ /= norm;
}

int toColor (double value) {
  int result = (value + 1.) * 128;
  assert (result >= 0);
  return result > 255 ? 255 : result;
}

int main (int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " map_size" << std::endl;
    return 1;
  }

  int mapSize = atoi (argv[1]);
  assert (mapSize > 0);

  QImage mapImage (mapSize, mapSize, QImage::Format_ARGB32);

  for (int i = 0; i < mapSize; ++i) {
    for (int j = 0; j < mapSize; ++j) {
      double x = i / double (mapSize);
      double y = j / double (mapSize);
      double r, g, b;
      fillFunction (x, y, r, g, b);
      mapImage.setPixel (i, j, qRgba (toColor (r), toColor (g), toColor (b), 255));
    }
  }

  mapImage.save ("result.tga");

  return 0;
}
