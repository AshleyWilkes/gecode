#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include "interconnected.h"

using namespace Gecode;

/**
 * \brief %Example: Hitori
 *
 * Blacken some fields in the grid so no unblackened
 * number is repeated in any row or column. No 2 blackened
 * fields may touch vertically or horizontally. All unblackened
 * fields must be interconnected vertically and horizontally.
 *
 */

std::array<std::array<int, 9>, 9> givens = {
  6, 1, 4, 3, 7, 6, 9, 6, 2,
  4, 5, 2, 8, 2, 7, 8, 1, 7,
  7, 2, 1, 5, 4, 9, 3, 5, 8,
  6, 3, 5, 9, 7, 4, 8, 9, 7,
  1, 7, 8, 2, 6, 3, 1, 4, 8,
  6, 4, 1, 3, 5, 7, 2, 7, 1,
  5, 1, 2, 4, 8, 6, 4, 5, 3,
  3, 8, 7, 5, 1, 2, 8, 9, 4,
  9, 7, 8, 4, 9, 4, 1, 8, 2
};

class Hitori : public Script {
private:
  IntVarArray actualValues;

public:
  /// Actual model
  Hitori(const SizeOptions& opt)
    : Script(opt),
      actualValues(*this, 81, 0, 9) {

    Matrix<IntVarArray> actualValuesM( actualValues, 9, 9 );
    //1) actualValue je rovno 0 anebo given
    for ( int row = 0; row < 9; ++row ) {
      for ( int col = 0; col < 9; ++col ) {
        IntArgs a({ 0, givens[row][col] });
        IntSet s(a);
        dom( *this, actualValuesM(col,row), s );
      }
    }
    //2) dve pole, kde actualValue je rovno 0, se nesmeji dotykat stranou
    for ( int row = 0; row < 8; ++row ) {
      for ( int col = 0; col < 9; ++col ) {
        IntVarArgs aR{ actualValuesM(col,row), actualValuesM(col,row+1) };
        IntVarArgs aC{ actualValuesM(row,col), actualValuesM(row+1,col) };
        count( *this, aR, 0, IRT_LE, 2 );
        count( *this, aC, 0, IRT_LE, 2 );
      }
    }
    //3) actualValues se neopakuji (az na 0) v radku ani sloupci
    for ( int i = 0; i < 9; ++i ) {
      distinct( *this, actualValuesM.row(i), 0);
      distinct( *this, actualValuesM.col(i), 0);
    }
    //4) pole, kde actualValues neni 0, musi byt propojena
    interconnected( *this, actualValues, 9, 0 );

    branch(*this, actualValues, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for ( int r = 0; r < 9; ++r ) {
      for ( int c = 0; c < 9; ++c ) {
        os << actualValues[r*9+c] << '\t';
      }
      os << '\n';
    }
  }
  /// Constructor for cloning \a s
  Hitori(Hitori& s) : Script(s) {
    actualValues.update(*this, s.actualValues);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Hitori(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Hitori (K&H, Logika 2022, 1. kolo, 7. uloha)");
  opt.parse(argc,argv);
  Script::run<Hitori,DFS,SizeOptions>(opt);
}
