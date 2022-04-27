#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Geometric Sudoku
 *
 * Place a digit 1-9 into each square of rectangle 9x9 so that
 * no digits repeat in any row, column or given area. For some
 * rows sums of the first and/or last 2 digits are given. For 
 * some columns products of the first and/or last 2 digits are given.
 *
 */

std::array<std::array<int, 9>, 9> areaIds = {
  1, 1, 1, 1, 1, 1, 2, 3, 3,
  4, 1, 1, 2, 2, 2, 2, 2, 3,
  4, 4, 1, 4, 4, 2, 2, 2, 3,
  5, 4, 4, 4, 4, 6, 6, 3, 3,
  5, 5, 6, 6, 6, 6, 6, 3, 3,
  5, 5, 6, 6, 7, 7, 7, 7, 3,
  5, 8, 8, 8, 7, 7, 9, 7, 7,
  5, 8, 8, 8, 8, 8, 9, 9, 7,
  5, 5, 8, 9, 9, 9, 9, 9, 9
};

std::array<int, 9> rowFront = { -1, 7, 10, 11, 9, 11, 12, -1, -1 };
std::array<int, 9> rowBack = { -1, -1, 8, 11, 8, 8, 10, 14, -1 };
std::array<int, 9> columnTop = { 14, 15, 72, -1, -1, -1, 30, 32, 28 };
std::array<int, 9> columnBottom = { -1, 28, 2, 63, -1, 72, 8, 30, -1 };
class GeometricSudoku : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  GeometricSudoku(const SizeOptions& opt)
    : Script(opt),
      x(*this, 81, 1, 9) {

    Matrix<IntVarArray> m(x, 9, 9);

    std::vector<IntVarArgs> areas(9);
    for ( int i = 0; i < 9; ++i ) {
      for ( int j = 0; j < 9; ++j ) {
        areas.at(areaIds[i][j] - 1) << m( j, i );
      }
    }

    for ( int i = 0; i < 9; ++i ) {
      //1) cisla jsou ruzny v kazdym radku
      distinct( *this, m.row(i) );
      //2) cisla jsou ruzny v kazdym sloupci
      distinct( *this, m.col(i) );
      //3) cisla jsou ruzny v kazdy zadany arey
      distinct( *this, areas[i] );
      //4) rowFront je soucet prvnich dvou v radku, kdyz neni -1
      if ( rowFront[i] != -1 ) {
        rel( *this, m( 0, i ) + m ( 1, i ) == rowFront[i] );
      }
      //5) rowBack je soucet poslednich dvou v radku, kdyz neni -1
      if ( rowBack[i] != -1 ) {
        rel( *this, m( 7, i ) + m ( 8, i ) == rowBack[i] );
      }
      //6) columnTop je soucin prvnich dvou v sloupci, kdyz neni -1
      if ( columnTop[i] != -1 ) {
        rel( *this, m( i, 0 ) * m ( i, 1 ) == columnTop[i] );
      }
      //7) columnBottom je soucin poslednich dvou v sloupci, kdyz neni -1
      if ( columnBottom[i] != -1 ) {
        rel( *this, m( i, 7 ) * m ( i, 8 ) == columnBottom[i] );
      }
    }

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for ( int r = 0; r < 9; ++r ) {
      for ( int c = 0; c < 9; ++c ) {
        os << x[r*9+c] << '\t';
      }
      os << '\n';
    }
  }
  /// Constructor for cloning \a s
  GeometricSudoku(GeometricSudoku& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new GeometricSudoku(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Geometric sudoku (K&H, Logika 2022, 2. kolo, 1. uloha)");
  opt.parse(argc,argv);
  Script::run<GeometricSudoku,DFS,SizeOptions>(opt);
}
