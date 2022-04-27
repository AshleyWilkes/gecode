#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Magnety
 *
 * Board consists of magnetic and nonmagnetic
 * tiles. Magnetic tile has one plus and one minus 
 * value. Nonmagnetic tile has two nonmagnetic
 * values. No two plus or two minus values may
 * share an edge. Counts of plus and minus
 * values are given for each row and column.
 *
 */

std::array<std::array<int, 10>, 10> areaIds = {
   1,  2,  2,  3,  3,  4,  5,  5,  6,  7,
   1,  8,  9,  9, 10,  4, 11, 11,  6,  7,
  12,  8, 13, 14, 10, 15, 15, 16, 16, 17,
  12, 18, 13, 14, 19, 19, 20, 20, 21, 17,
  22, 18, 23, 23, 24, 25, 25, 26, 21, 27,
  22, 28, 28, 29, 24, 30, 30, 26, 31, 27,
  32, 32, 33, 29, 34, 34, 35, 35, 31, 36,
  37, 38, 33, 39, 39, 40, 41, 42, 42, 36,
  37, 38, 43, 43, 44, 40, 41, 45, 46, 46,
  47, 47, 48, 48, 44, 49, 49, 45, 50, 50
};

std::array<std::array<int, 2>, 10> rows = {
  5, 4,
  4, 5,
  4, 3, 
  3, 3, 
  4, 4,
  3, 3,
  4, 4,
  2, 3,
  3, 4,
  5, 4
}, columns = {
  4, 4,
  3, 4,
  4, 3,
  4, 5,
  4, 3,
  4, 3,
  4, 4,
  3, 3,
  2, 4,
  5, 4
};

class Magnets : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  Magnets(const SizeOptions& opt)
    : Script(opt),
      //0 - minus; 1 - plus; 2 - nonmagnetic
      x(*this, 100, 0, 2) {
    Matrix<IntVarArray> m{ x, 10, 10 };

    std::vector<IntVarArgs> areas( 50 );
    for ( int i = 0; i < 10; ++i ) {
      for ( int j = 0; j < 10; ++j ) {
        areas.at(areaIds[i][j] - 1) << m( j, i );
      }
    }
    for ( int i = 0; i < 50; ++i ) {
      //kosticka je magneticka nebo ne
      //tj. count nonmagnetic je 0 nebo 2
      count( *this, areas[i], 2, IRT_NQ, 1 );
    }

    for ( int row = 0; row < 9; ++row ) {
      for ( int col = 0; col < 10; ++col ) {
        IntVarArgs aR{ m(col,row), m(col,row+1) };
        IntVarArgs aC{ m(row,col), m(row+1,col) };

        //minusy se nedotykaji stranou
        count( *this, aR, 0, IRT_LE, 2 );
        count( *this, aC, 0, IRT_LE, 2 );

        //plusy se nedotykaji stranou
        count( *this, aR, 1, IRT_LE, 2 );
        count( *this, aC, 1, IRT_LE, 2 );
      }
    }

    //pocty jsou zadane pro plusy a minusy
    for ( int i = 0; i < 10; ++i ) {
      count( *this, m.row(i), 1, IRT_EQ, rows[i][0] );
      count( *this, m.row(i), 0, IRT_EQ, rows[i][1] );
      count( *this, m.col(i), 1, IRT_EQ, columns[i][0] );
      count( *this, m.col(i), 0, IRT_EQ, columns[i][1] );
    }

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m{ x, 10, 10 };
    os << m;
  }
  /// Constructor for cloning \a s
  Magnets(Magnets& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Magnets(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Magnets (K&H, Logika 2022, 2. kolo, 6. uloha)");
  opt.parse(argc,argv);
  Script::run<Magnets,DFS,SizeOptions>(opt);
}
