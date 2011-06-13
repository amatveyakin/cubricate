// TODO refactor variableNames and (brackets)

#ifndef CUBE_ARRAY_HPP
#define CUBE_ARRAY_HPP


#include <cassert>
#include <vector>


template< typename Cube_position_t, typename Cube_type_t >
class Cube_array {
public:
  Cube_array( int size_x, int size_y, int size_z ) :
    m_size_x( size_x ),
    m_size_y( size_y ),
    m_size_z( size_z ),
    m_max_cubes( size_x * size_y * size_z ),
    m_n_cubes( 0 ),

    m_cube_position_array( 0 ),
    m_cube_types_array( 0 ),
    m_map_position_to_cube_index( m_max_cubes, -1 )
  {
  }

  ~Cube_array() { }

  int         size_x() const            { return m_size_x; }
  int         size_y() const            { return m_size_y; }
  int         size_z() const            { return m_size_z; }
  int         max_cubes() const         { return m_max_cubes; }
  int         n_cubes() const           { return m_n_cubes; }

  const Cube_position_t*  cube_positions() const  { return m_cube_position_array; }
  const Cube_type_t*      cube_types() const      { return m_cube_types_array; }

  void set_pointers( Cube_position_t* cube_positions, Cube_type_t* cube_types ) {
    m_cube_position_array  = cube_positions;
    m_cube_types_array      = cube_types;
  }

  void add_cube( int x, int y, int z, int type ) {
    if ( is_empty( type ) )
      return remove_cube( x, y, z );
    check_coordinates( x, y, z );
    int position = xyz_to_position( x, y, z );
    int index = m_map_position_to_cube_index[ position ];
    if ( index >= 0 ) {
      m_cube_types_array[ index ] = type;
      return;
    }
    m_map_position_to_cube_index[ position ] = m_n_cubes;
    m_cube_position_array[ m_n_cubes * 4     ] = x;
    m_cube_position_array[ m_n_cubes * 4 + 1 ] = y;
    m_cube_position_array[ m_n_cubes * 4 + 2 ] = z;
    m_cube_position_array[ m_n_cubes * 4 + 3 ] = 0.5;
    m_cube_types_array[ m_n_cubes ] = type;
    m_n_cubes++;
  }

  void remove_cube( int x, int y, int z ) {
    check_coordinates( x, y, z );
    int position = xyz_to_position( x, y, z );
    int index = m_map_position_to_cube_index[ position ];
    if ( index < 0 )
      return;
    m_n_cubes--;
    int last_cube_position = xyz_to_position( m_cube_position_array[ m_n_cubes * 4 ], m_cube_position_array[ m_n_cubes * 4 + 1 ], m_cube_position_array[ m_n_cubes * 4 + 2 ] );
    m_map_position_to_cube_index[ position ] = -1;
    m_map_position_to_cube_index[ last_cube_position ] = index;
    std::copy( m_cube_position_array + m_n_cubes * 4, m_cube_position_array + ( m_n_cubes + 1 ) * 4, m_cube_position_array + index * 4 );
    m_cube_types_array[ index ] = m_cube_types_array[ m_n_cubes ];
  }

protected:
  int   m_size_x;
  int   m_size_y;
  int   m_size_z;
  int   m_max_cubes;
  int   m_n_cubes;

  Cube_position_t*    m_cube_position_array;
  Cube_type_t*        m_cube_types_array;
  std::vector< int >  m_map_position_to_cube_index;

  void check_coordinates( int x, int y, int z ) const {
    assert( x >= 0 );
    assert( y >= 0 );
    assert( z >= 0 );
    assert( x < m_size_x );
    assert( y < m_size_y );
    assert( z < m_size_z );
  }

  int xyz_to_position( int x, int y, int z ) const {
    return  x * m_size_y * m_size_z  +  y * m_size_z  +  z;
  }

  static Cube_type_t empty_cube()                           { return Cube_type_t(); }
  static Cube_type_t is_empty( Cube_type_t cube_type )      { return cube_type == empty_cube(); }
};


// TODO: rename (?)
template< typename Cube_position_t, typename Cube_type_t >
class Visible_cube_set : public Cube_array< Cube_position_t, Cube_type_t > {
private:
  typedef Cube_array< Cube_position_t, Cube_type_t > Parent;

public:
  Visible_cube_set( int size_x, int size_y, int size_z ) :
    Parent( size_x, size_y, size_z ),
    m_map_cube_types( Parent::m_max_cubes ),
    m_map_n_cube_neighbours( Parent::m_max_cubes, 0 )
  {
    for ( int x = 0; x < size_x; ++x )
      for ( int y = 0; y < size_y; ++y ) {
        m_map_n_cube_neighbours[ Parent::xyz_to_position( x, y, 0 ) ]++;
        m_map_n_cube_neighbours[ Parent::xyz_to_position( x, y, size_z - 1 ) ]++;
      }

    for ( int y = 0; y < size_y; ++y )
      for ( int z = 0; z < size_z; ++z ) {
        m_map_n_cube_neighbours[ Parent::xyz_to_position( 0, y, z ) ]++;
        m_map_n_cube_neighbours[ Parent::xyz_to_position( size_x - 1, y, z ) ]++;
      }

    for ( int x = 0; x < size_x; ++x )
      for ( int z = 0; z < size_z; ++z ) {
        m_map_n_cube_neighbours[ Parent::xyz_to_position( x, 0, z ) ]++;
        m_map_n_cube_neighbours[ Parent::xyz_to_position( x, size_y - 1, z ) ]++;
      }
  }

  ~Visible_cube_set() { }

  void add_cube( int x, int y, int z, int type ) {
    Parent::check_coordinates( x, y, z );
    int position = Parent::xyz_to_position( x, y, z );

    if ( m_map_cube_types[ position ] == type )
      return;

    if ( Parent::is_empty( type ) )
      return remove_cube( x, y, z );

    Cube_type_t old_type = m_map_cube_types[ position ];
    m_map_cube_types[ position ] = type;
    if ( m_map_n_cube_neighbours[ position ] < 6 )
      Parent::add_cube( x, y, z, type );

    if ( !Parent::is_empty( old_type ) )
      return;

    int i_min, j_min, k_min, i_max, j_max, k_max;
    get_neighbour_limits( x, y, z, i_min, j_min, k_min, i_max, j_max, k_max );

    for ( int i = i_min; i <= i_max; i += 2 )
      add_neighbour( i, y, z );
    for ( int j = j_min; j <= j_max; j += 2 )
      add_neighbour( x, j, z );
    for ( int k = k_min; k <= k_max; k += 2 )
      add_neighbour( x, y, k );
  }

  void remove_cube( int x, int y, int z ) {
    Parent::check_coordinates( x, y, z );
    int position = Parent::xyz_to_position( x, y, z );
    if ( Parent::is_empty( m_map_cube_types[ position ] ) )
      return;

    m_map_cube_types[ position ] = Parent::empty_cube();
//     Parent::remove_cube( x, y, z );
    Parent::add_cube( x, y, z, Parent::empty_cube() );

    int i_min, j_min, k_min, i_max, j_max, k_max;
    get_neighbour_limits( x, y, z, i_min, j_min, k_min, i_max, j_max, k_max );

    for ( int i = i_min; i <= i_max; i += 2 )
      remove_neighbour( i, y, z );
    for ( int j = j_min; j <= j_max; j += 2 )
      remove_neighbour( x, j, z );
    for ( int k = k_min; k <= k_max; k += 2 )
      remove_neighbour( x, y, k );
  }

  Cube_type_t cube_type( int x, int y, int z ) const {
    Parent::check_coordinates( x, y, z );
    int position = Parent::xyz_to_position( x, y, z );
    return m_map_cube_types[ position ];
  }

  bool cube_presents( int x, int y, int z ) const {
    return !Parent::is_empty( cube_type( x, y, z ) );
  }

protected:
  std::vector< Cube_type_t > m_map_cube_types;
  std::vector< Cube_type_t > m_map_n_cube_neighbours;

  void get_neighbour_limits( int x, int y, int z, int& x_min, int& y_min, int& z_min, int& x_max, int& y_max, int& z_max ) {
    x_min = x - 1;
    y_min = y - 1;
    z_min = z - 1;
    if ( x_min < 0 )  x_min += 2;
    if ( y_min < 0 )  y_min += 2;
    if ( z_min < 0 )  z_min += 2;

    x_max = x + 1;
    y_max = y + 1;
    z_max = z + 1;
    if ( x_max > Parent::m_size_x - 1 )  x_max -= 2;
    if ( y_max > Parent::m_size_y - 1 )  y_max -= 2;
    if ( z_max > Parent::m_size_z - 1 )  z_max -= 2;
  }

  void remove_neighbour( int x, int y, int z ) {
    int position = Parent::xyz_to_position( x, y, z );
    if ( m_map_n_cube_neighbours[ position ] == 6 )
      Parent::add_cube( x, y, z, m_map_cube_types[ position ] );
    assert( m_map_n_cube_neighbours[ position ] > 0 );
    m_map_n_cube_neighbours[ position ]--;
  }

  void add_neighbour( int x, int y, int z ) {
    int position = Parent::xyz_to_position( x, y, z );
    assert( m_map_n_cube_neighbours[ position ] < 6 );
    m_map_n_cube_neighbours[ position ]++;
    if ( m_map_n_cube_neighbours[ position ] == 6 )
      Parent::remove_cube( x, y, z );
  }
};


#endif
