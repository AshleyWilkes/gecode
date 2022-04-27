#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Blocks
 *
 * Place a number 1 up to 5 to each field
 * such that each area contains numbers 1
 * to the areas's size. No two fields with
 * the same number may share an edge or a corner.
 * One number is given.
 *
 */

std::array<std::array<int, 7>, 7> areaIds = {
   1,  2,  2,  3,  3,  4,  4,
   1,  5,  5,  3,  3,  4,  6,
   1,  5,  5,  3,  4,  4,  6,
   1,  5,  7,  8,  8,  8,  6,
   9,  9,  7,  8,  8, 10,  6,
   9,  9,  7, 11, 12, 10,  6,
  11, 11, 11, 11, 12, 10, 10
};

std::pair<int, int> given = { 12, 1 };

class Blocks : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  Blocks(const SizeOptions& opt)
    : Script(opt),
      x(*this, 49, 1, 5) {
    Matrix<IntVarArray> m{ x, 7, 7 };

    //zadane cislo
    rel( *this, x[ 12 ] == 1 );

    int numAreas = 12;
    int size = 7;
    std::vector<IntVarArgs> areas( numAreas );
    for ( int i = 0; i < size; ++i ) {
      for ( int j = 0; j < size; ++j ) {
        areas.at(areaIds[i][j] - 1) << m( j, i );
      }
    }

    //kazda area obsahuje prave cisla 1-velikostArey
    for ( auto area : areas ) {
      dom( *this, area, 1, area.size() );
      distinct( *this, area );
    }

    for ( int row = 0; row < size - 1; ++row ) {
      for ( int col = 0; col < size; ++col ) {
        IntVarArgs aR{ m(col,row), m(col,row+1) };
        IntVarArgs aC{ m(row,col), m(row+1,col) };

        //stejne cislice se nedotykaji stranou
        for ( int i = 1; i < 6; ++i ) {
          count( *this, aR, i, IRT_LE, 2 );
          count( *this, aC, i, IRT_LE, 2 );
        }
      }
    }

    //stejne cislice se nedotykaji rohem
    for ( int row = 0; row < size - 1; ++row ) {
      for ( int col = 0; col < size - 1; ++col ) {
        IntVarArgs aR{ m(col,row), m(col+1,row+1) };
        IntVarArgs aC{ m(row,col+1), m(row+1,col) };
        for ( int val = 1; val <= size; ++val ) {
          count( *this, aR, val, IRT_LE, 2 );
          count( *this, aC, val, IRT_LE, 2 );
        }
      }
    }

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m{ x, 7, 7 };
    os << m;
  }
  /// Constructor for cloning \a s
  Blocks(Blocks& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Blocks(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Blocks (K&H, Logika 2022, 2. kolo, 8. uloha)");
  opt.parse(argc,argv);
  Script::run<Blocks,DFS,SizeOptions>(opt);
}
