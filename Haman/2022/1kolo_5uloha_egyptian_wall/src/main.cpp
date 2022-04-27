#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Egyptian wall of bricks
 *
 * Fill each field in the grid with a number 1-8 so that numbers
 * don't repeat in any row, column or diagonal. Some numbers are given.
 * Grid is split into so called bricks. In each brick of size 3
 * even and odd numbers must alternate. Bricks of size 2 must
 * contain 1 even and 1 odd number. Same numbers must not touch
 * each other with a corner.
 *
 */

//1 means this field and a field to the right belong to the same brick
//0 means there is a border between bricks to the right of this field
//nothing is specified for the rightmost column
std::array<std::array<int, 7>, 8> bricks = {
  1, 1, 0, 1, 1, 0, 1,
  1, 0, 1, 1, 0, 1, 1,
  1, 1, 0, 1, 1, 0, 1,
  1, 0, 1, 1, 0, 1, 1,
  1, 1, 0, 1, 1, 0, 1,
  1, 0, 1, 1, 0, 1, 1,
  1, 1, 0, 1, 1, 0, 1,
  1, 0, 1, 1, 0, 1, 1
};

std::array<std::array<int, 8>, 8> givens = {
  0, 0, 2, 0, 0, 7, 0, 0,
  0, 4, 0, 0, 0, 0, 5, 0,
  0, 0, 0, 0, 0, 0, 0, 7,
  0, 0, 4, 0, 0, 0, 2, 0,
  0, 5, 0, 0, 0, 1, 0, 0,
  6, 0, 0, 0, 0, 0, 0, 0,
  0, 2, 0, 0, 0, 0, 1, 0,
  0, 0, 6, 0, 0, 3, 0, 0
};

class EgyptianWall : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  EgyptianWall(const SizeOptions& opt)
    : Script(opt),
      x(*this, 64, 1, 8) {

    Matrix<IntVarArray> m(x, 8, 8);
    IntVarArgs mainDiag{ m(0,0), m(1,1), m(2,2), m(3,3), m(4,4), m(5,5), m(6,6), m(7,7) };
    IntVarArgs antiDiag{ m(7,0), m(6,1), m(5,2), m(4,3), m(3,4), m(2,5), m(1,6), m(0,7) };

    //1) zadana cisla musi odpovidat
    for ( int row = 0; row < 8; ++row ) {
      for ( int col = 0; col < 8; ++col ) {
        if ( givens[row][col] != 0 ) {
          rel( *this, m(col,row) == givens[row][col] );
        }
      }
    }

    for ( int i = 0; i < 8; ++i ) {
      //2) cisla jsou ruzny v kazdym radku
      distinct( *this, m.row(i) );
      //3) cisla jsou ruzny v kazdym sloupci
      distinct( *this, m.col(i) );
    }
    //4) cisla jsou ruzny na hlavni diagonale
    distinct( *this, mainDiag );
    //5) cisla jsou ruzny na vedlejsi diagonale
    distinct( *this, antiDiag );
    //6) kdyz je bricks rovno 1, musi soucet zde a napravo byt lichy
    for ( int row = 0; row < 8; ++row ) {
      for ( int col = 0; col < 7; ++col ) {
        if ( bricks[row][col] == 1 ) {
          rel( *this, ( m(col,row) + m(col+1,row) ) %2 == 1 );
        }
      }
    }
    //7) v kazde dvojici diagonalne sousedicich poli musi byt ruzna cisla
    for ( int row = 0; row < 7; ++row ) {
      rel( *this, m(0,row) != m(1,row+1) );
      for ( int col = 1; col < 7; ++ col ) {
        rel( *this, m(col,row) != m(col+1,row+1) );
        rel( *this, m(col+1,row) != m(col,row+1) );
      }
      rel( *this, m(7,row) != m(6,row+1) );
    }

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for ( int r = 0; r < 8; ++r ) {
      for ( int c = 0; c < 8; ++c ) {
        os << x[r*8+c] << '\t';
      }
      os << '\n';
    }
  }
  /// Constructor for cloning \a s
  EgyptianWall(EgyptianWall& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new EgyptianWall(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Egyptian Wall (K&H, Logika 2022, 1. kolo, 5. uloha)");
  opt.parse(argc,argv);
  Script::run<EgyptianWall,DFS,SizeOptions>(opt);
}
