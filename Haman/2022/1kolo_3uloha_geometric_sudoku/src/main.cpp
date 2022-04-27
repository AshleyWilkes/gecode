#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Geometric Sudoku
 *
 * Place a digit 1-9 into each square of rectangle 9x9 so that
 * no digits repeat in any row, column or given area. For some
 * rows products of the first and/or last 2 digits are given. For 
 * some columns sums of the first and/or last 2 digits are given.
 *
 */

std::array<std::array<int, 9>, 9> areaIds = {
  1, 1, 2, 3, 3, 3, 3, 3, 3,
  1, 2, 2, 3, 4, 4, 4, 3, 4,
  1, 2, 2, 3, 5, 4, 4, 4, 4,
  1, 2, 5, 5, 5, 4, 6, 6, 6,
  1, 2, 2, 2, 5, 7, 7, 7, 6,
  1, 1, 1, 8, 5, 5, 5, 7, 6,
  8, 8, 8, 8, 5, 9, 7, 7, 6,
  8, 9, 8, 8, 8, 9, 7, 7, 6,
  9, 9, 9, 9, 9, 9, 7, 6, 6
};

std::array<int, 9> rowFront = { 10, -1, 12, 42, -1, 54, 36, -1, 14 };
std::array<int, 9> rowBack = { -1, 30, 8, 16, -1, 20, 14, 12, -1 };
std::array<int, 9> columnTop = { 13, 3, -1, 8, -1, 12, -1, 12, 9 };
std::array<int, 9> columnBottom = { -1, 12, 14, 10, -1, 10, 13, 9, -1 };
class GeometricSudoku : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  GeometricSudoku(const SizeOptions& opt)
    : Script(opt),
      x(*this, 81, 1, 9) {

    Matrix<IntVarArray> m(x, 9, 9);

    //toto je humus, rucne prepsane info z areaIds; zjevne jsem jeste
    //nemel dost jasno v API gecodu; v 2kolo_1uloha_geometric_sudoku
    //je pouzitelnejsi verze, ktera areas generuje z areaIds
    std::vector<IntVarArgs> areas {
      { x[0], x[1], x[9], x[18], x[27], x[36], x[45], x[46], x[47] },
      { x[2], x[10], x[11], x[19], x[20], x[28], x[37], x[38], x[39] },
      { x[3], x[4], x[5], x[6], x[7], x[8], x[12], x[16], x[21] },
      { x[13], x[14], x[15], x[17], x[23], x[24], x[25], x[26], x[32] },
      { x[22], x[29], x[30], x[31], x[40], x[49], x[50], x[51], x[58] },
      { x[33], x[34], x[35], x[44], x[53], x[62], x[71], x[79], x[80] },
      { x[41], x[42], x[43], x[52], x[60], x[61], x[69], x[70], x[78] },
      { x[48], x[54], x[55], x[56], x[57], x[63], x[65], x[66], x[67] }, 
      { x[59], x[64], x[68], x[72], x[73], x[74], x[75], x[76], x[77] }
    };

    for ( int i = 0; i < 9; ++i ) {
      //1) cisla jsou ruzny v kazdym radku
      distinct( *this, m.row(i) );
      //2) cisla jsou ruzny v kazdym sloupci
      distinct( *this, m.col(i) );
      //3) cisla jsou ruzny v kazdy zadany arey
      distinct( *this, areas[i] );
      //4) rowFront je soucin prvnich dvou v radku, kdyz neni -1
      if ( rowFront[i] != -1 ) {
        rel( *this, m( 0, i ) * m ( 1, i ) == rowFront[i] );
      }
      //5) rowBack je soucin poslednich dvou v radku, kdyz neni -1
      if ( rowBack[i] != -1 ) {
        rel( *this, m( 7, i ) * m ( 8, i ) == rowBack[i] );
      }
      //6) columnTop je soucet prvnich dvou v sloupci, kdyz neni -1
      if ( columnTop[i] != -1 ) {
        rel( *this, m( i, 0 ) + m ( i, 1 ) == columnTop[i] );
      }
      //7) columnBottom je soucet poslednich dvou v sloupci, kdyz neni -1
      if ( columnBottom[i] != -1 ) {
        rel( *this, m( i, 7 ) + m ( i, 8 ) == columnBottom[i] );
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
  SizeOptions opt("Geometric sudoku (K&H, Logika 2022, 1. kolo, 3. uloha)");
  opt.parse(argc,argv);
  Script::run<GeometricSudoku,DFS,SizeOptions>(opt);
}
