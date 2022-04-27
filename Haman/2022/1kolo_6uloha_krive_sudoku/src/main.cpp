#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Tredoku
 *
 * Do prazdnych policek obrazce doplnte cislice 1 az 9 tak,
 * aby byly vsechny ruzne v kazdem ohranicenem kosoctverci
 * 3x3 policka, ve trech lomenych radach sesti policek a
 * v patnacti lomenych radach deviti policek, ktera spolu
 * sousedi stranou.
 *
 */

std::array<int, 81> givensArray {
  //0
  -1, -1,  3,
   6, -1, -1,
  -1, -1,  8,
  //1
  -1, -1,  9,
  -1,  2, -1,
  -1, -1,  1,
  //2
  -1,  7, -1,
   9,  8,  6,
   1, -1,  3,
  //3
  -1,  2, -1,
  -1, -1,  5,
   6, -1, -1,
  //4
   9, -1, -1,
   7, -1, -1,
  -1,  5, -1,
  //5
  -1,  8,  6,
   1, -1, -1,
   4,  7, -1,
  //6
  -1,  5,  3,
  -1,  6, -1,
   8, -1,  7,
  //7
   2, -1,  6,
  -1,  9, -1,
  -1,  3, -1,
  //8
  -1,  1, -1,
  -1, -1,  4,
  -1, -1,  9
};

std::array<std::array<int, 4>, 1> sixFielders {
  5, 0,
  6, 1
};

std::array<std::array<int, 6>, 5> nineFielders {
  0, 0,
  1, 0,
  3, 0,

  0, 1,
  4, 0,
  7, 1,

  1, 1,
  4, 1,
  5, 1,

  2, 1,
  3, 1,
  8, 1,

  6, 0,
  7, 0,
  8, 0
};

class Tredoku : public Script {
private:
  IntVarArray x;

  IntVarArgs& addFields( std::size_t squareId, std::size_t dirId, std::size_t rowId, IntVarArgs& args ) {
    std::array<std::array<std::array<int, 3>, 3>, 2> delta {
      0, 1, 2,
      3, 4, 5,
      6, 7, 8,

      0, 3, 6,
      1, 4, 7,
      2, 5, 8
    };
    for ( int i = 0; i < 3; ++i ) {
      std::cout << squareId * 9 + delta[ dirId ][ rowId ][ i ] << " ";
      args << x[ squareId * 9 + delta[ dirId ][ rowId ][ i ] ];
    }
    return args;
  }
public:
  /// Actual model
  Tredoku(const SizeOptions& opt)
      : Script(opt),
      x( *this, 81, 1, 9 ) {
    //je treba dat dohromady IntVarArgsy s policky pro kazdy smer a vsechno to nasrvat distinctu
    //1) jednotlive kosoctverce jako celky
    for ( std::size_t i = 0; i < 9; ++i ) {
      IntVarArgs slice = x.slice( i * 9, 1, 9 );
      distinct( *this, slice );
    }
    /*//2) sixFielders
    for ( auto sixFielder : sixFielders ) {
      IntVarArgs slice;
      for ( int rowId = 0; rowId < 3; ++rowId ) {
        for ( std::size_t i = 0; i < sixFielder.size() / 2; ++i ) {
          addFields( sixFielder[ 2 * i ], sixFielder[ 2 * i + 1], rowId, slice );
        }
        std::cout << '\n';
        distinct( *this, slice );
      }
    }
    //3) nineFielders
    for ( auto nineFielder : nineFielders ) {
      IntVarArgs slice;
      for ( int rowId = 0; rowId < 3; ++rowId ) {
        for ( std::size_t i = 0; i < nineFielder.size() / 2; ++i ) {
          addFields( nineFielder[ 2 * i ], nineFielder[ 2 * i + 1], rowId, slice );
        }
        std::cout << '\n';
        distinct( *this, slice );
      }
    }*/

    std::vector<std::vector<int>> regions = {
      { 45, 46, 47, 56, 59, 62 },
      { 48, 49, 50, 55, 58, 61 },
      { 51, 52, 53, 54, 57, 60 },

      { 0, 1, 2, 9, 10, 11, 27, 28, 29 },
      { 3, 4, 5, 12, 13, 14, 30, 31, 32 },
      { 6, 7, 8, 15, 16, 17, 33, 34, 35 },

      { 0, 3, 6, 42, 43, 44, 63, 66, 69 },
      { 1, 4, 7, 39, 40, 41, 64, 67, 70 },
      { 2, 5, 8, 36, 37, 38, 65, 68, 71 },

      { 9, 12, 15, 36, 39, 42, 45, 48, 51 },
      { 10, 13, 16, 37, 40, 43, 46, 49, 52 },
      { 11, 14, 17, 38, 41, 44, 47, 50, 53 },

      { 18, 21, 24, 27, 30, 33, 72, 75, 78 },
      { 19, 22, 25, 28, 31, 34, 73, 76, 79 },
      { 20, 23, 26, 29, 32, 35, 74, 77, 80 },

      { 54, 55, 56, 63, 64, 65, 72, 73, 74 },
      { 57, 58, 59, 66, 67, 68, 75, 76, 77 },
      { 60, 61, 62, 69, 70, 71, 78, 79, 80 }
    };

    for ( auto region : regions ) {
      IntVarArgs slice;
      for ( int i : region ) {
        slice << x[ i ];
      }
      distinct( *this, slice );
    }

    for ( int i = 0; i < 81; ++i ) {
      if ( givensArray[ i ] != -1 ) {
        rel( *this, x[ i ], IRT_EQ, givensArray[ i ] );
      }
    }
    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << x << '\n';
  }
  /// Constructor for cloning \a s
  Tredoku(Tredoku& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Tredoku(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Tredoku (K&H, Logika 2022, 1. kolo, 9. uloha)");
  opt.parse(argc,argv);
  Script::run<Tredoku,DFS,SizeOptions>(opt);
}
