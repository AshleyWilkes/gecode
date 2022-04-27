#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <set>

using namespace Gecode;

/**
 * \brief %Example: Spojena sudoku
 *
 * We have 5 interconnected geometric sudokus. 
 * 4 of them (labeled 1-4) are 6x6, the fifth
 * is 5x5. Sudokus 1 and 4 are also no-touch.
 *
 */

std::array<std::array<int, 6>, 6> areaIds1 = {
  1, 1, 1, 2, 2, 2,
  1, 1, 1, 2, 2, 2,
  3, 3, 3, 4, 4, 4,
  3, 3, 3, 4, 4, 4,
  5, 5, 5, 6, 6, 6,
  5, 5, 5, 6, 6, 6
};

std::array<std::array<int, 6>, 6> areaIds2 = {
  1, 1, 2, 3, 3, 3,
  1, 1, 2, 2, 3, 3,
  1, 1, 4, 2, 2, 3,
  5, 4, 4, 2, 6, 6,
  5, 5, 4, 4, 6, 6,
  5, 5, 5, 4, 6, 6
};

std::array<std::array<int, 5>, 5> areaIds3 = {
  1, 1, 1, 2, 2,
  1, 1, 3, 3, 2,
  4, 4, 3, 2, 2,
  4, 3, 3, 5, 5,
  4, 4, 5, 5, 5
};

std::array<std::array<int, 6>, 6> givens1 = {
  0, 0, 0, 5, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 3, 0, 0, 0, 1,
  4, 0, 0, 0, 0, 0,
  0, 0, 0, 6, 0, 0,
  0, 0, 1, 0, 0, 0
};

std::array<std::array<int, 6>, 6> givens2 = {
  0, 0, 0, 0, 0, 3,
  0, 0, 2, 0, 0, 4,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 3, 0, 0,
  0, 0, 0, 5, 0, 0,
  0, 0, 1, 0, 0, 0
};

std::array<std::array<int, 6>, 6> givens3 = {
  0, 0, 0, 4, 0, 0,
  0, 0, 3, 0, 0, 0,
  0, 0, 5, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  1, 0, 0, 2, 0, 0,
  5, 0, 0, 0, 0, 0
};

std::array<std::array<int, 6>, 6> givens4 = {
  0, 0, 0, 4, 0, 0,
  0, 0, 5, 0, 0, 0,
  0, 0, 0, 0, 0, 1,
  4, 0, 0, 0, 6, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 3, 0, 0, 0
};

class ConnectedSudokus : public Script {
private:
  IntVarArray s1, s2, s3, s4, s5;

public:
  /// Actual model
  ConnectedSudokus(const SizeOptions& opt)
    : Script(opt),
      s1(*this, 36, 1, 6),
      s2(*this, 36, 1, 6),
      s3(*this, 36, 1, 6),
      s4(*this, 36, 1, 6),
      s5(*this, 25, 1, 5) {

    makeSudoku( s1, areaIds1, givens1, true );
    makeSudoku( s2, areaIds2, givens2 );
    makeSudoku( s3, areaIds2, givens3 );
    makeSudoku( s4, areaIds1, givens4, true );
    makeSudoku( s5, areaIds3 );

    makeConnected( s1, 4, 4, s5, 0, 0 );
    makeConnected( s2, 4, 0, s5, 0, 3 );
    makeConnected( s3, 0, 4, s5, 3, 0 );
    makeConnected( s4, 0, 0, s5, 3, 3 );

    branch(*this, s1, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, s2, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, s3, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, s4, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, s5, INT_VAR_NONE(), INT_VAL_MIN());
  }

  template<std::size_t size>
  void makeSudoku( IntVarArray& vars, std::array<std::array<int, size>, size> areaIds,
      std::array<std::array<int, size>, size> givens = {}, bool noTouch = false ) {
    Matrix<IntVarArray> m{ vars, size, size };

    std::vector<IntVarArgs> areas( size );
    for ( int i = 0; i < size; ++i ) {
      for ( int j = 0; j < size; ++j ) {
        areas.at(areaIds[i][j] - 1) << m( j, i );
      }
    }

    //zadana cisla musi odpovidat
    for ( int row = 0; row < size; ++row ) {
      for ( int col = 0; col < size; ++col ) {
        if ( givens[row][col] != 0 ) {
          rel( *this, m(col,row) == givens[row][col] );
        }
      }
    }

    for ( int i = 0; i < size; ++i ) {
      //cisla jsou ruzny v kazdym radku
      distinct( *this, m.row(i) );
      //cisla jsou ruzny v kazdym sloupci
      distinct( *this, m.col(i) );
      //cisla jsou ruzny v kazdy zadany arey
      distinct( *this, areas[i] );
    }
    
    if ( noTouch ) {
      //stejna cisla nesousedi rohem
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
    }
  }

  void makeConnected( IntVarArray& s1, int r1, int c1, IntVarArray& s2, int r2, int c2 ) {
    //pro ctverec 2x2 plati, ze pole v s1[r1, c1] == s2[r2, c2]
    //s1 je 6x6, s2 je 5x5
    Matrix<IntVarArray> m1{ s1, 6, 6 }, m2{ s2, 5, 5 };
    for ( int row = 0; row < 2; ++row ) {
      for ( int column = 0; column < 2; ++column ) {
        rel( *this, m1( c1 + column, r1 + row ) == m2( c2 + column, r2 + row ) );
      }
    }
  }

  void print( std::ostream& os, const IntVarArray& vars, int size ) const {
    Matrix<IntVarArray> m{ vars, size, size };
    os << m << '\n';
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    print( os, s1, 6 );
    print( os, s2, 6 );
    print( os, s3, 6 );
    print( os, s4, 6 );
    print( os, s5, 5 );
  }
  /// Constructor for cloning \a s
  ConnectedSudokus(ConnectedSudokus& s) : Script(s) {
    s1.update(*this, s.s1);
    s2.update(*this, s.s2);
    s3.update(*this, s.s3);
    s4.update(*this, s.s4);
    s5.update(*this, s.s5);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new ConnectedSudokus(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("ConnectedSudokus (K&H, Logika 2022, 2. kolo, 5. uloha)");
  opt.parse(argc,argv);
  Script::run<ConnectedSudokus,DFS,SizeOptions>(opt);
}
