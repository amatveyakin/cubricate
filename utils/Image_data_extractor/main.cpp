#include <cstdlib>
#include <iostream>
#include <QtGui/QImage>


int log2( int x ) {
  int result = 0;
  while ( x / 2 != 0 ) {
    ++result;
    x /= 2;
  }
  return result;
}

int pow2( int x ) {
  return 1 << x;
}


void wrong_usage( const char* program_name ) {
  std::cerr << "Usage:  " << program_name << " image_file [height_coeff]\n";
  exit( 1 );
}

int main( int argc, char **argv ) {
  if ( argc < 2 || argc > 3 )
    wrong_usage( argv[ 0 ] );

  const char* image_file = argv[ 1 ];
  QImage image( image_file );
  if ( image.isNull() ) {
    std::cerr << "Unable to open image file ``" << image_file << "''\n";
    return 1;
  }

  double height_coeff = 1.;
  if ( argc == 3 ) {
    char* endptr;
    height_coeff = std::strtod( argv[ 2 ], &endptr );
    if ( *endptr )
      wrong_usage( argv[ 0 ] );
    if ( height_coeff < 0 || height_coeff > 1 ) {
      std::cerr << "Height coeff must be in [0, 1] interval\n";
      return 1;
    }
  }

  int width  = image.width();
  int height = image.height();

  if ( width != height ) {
    std::cerr << "Warning: the image isn't square\n";
  }
  else {
    if ( width != pow2( log2( width ) ) )
      std::cerr << "Warning: image size is not a power of 2\n";
  }

  int max_size = std::max( width, height );

  for ( int i = 0; i < width; ++i ) {
    for ( int j = 0; j < height; ++j ) {
      int height = ( double( image.pixel( i, j ) & 0xff ) / 256. * height_coeff + ( 1. - height_coeff ) / 2. ) * max_size;
      std::cout << std::min( std::max( height, 1 ), 255 ) << " ";
    }
    std::cout << "\n";
  }

  return 0;
}
