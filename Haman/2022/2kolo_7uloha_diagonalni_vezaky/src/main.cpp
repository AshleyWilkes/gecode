#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Diagonal skyscrapers
 *
 * In each row and column of the board there are
 * 7 skyscrapers of different heights 1-7 and one
 * empty field. Givens for some row and column directions
 * specify sums of skyscrapers visible from that direction.
 * Some skyscrapers heights are also given.
 *
 */

std::array<std::array<int, 8>, 8> givens = {
  0, 0, 0, 0, 0, 0, 3, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 5, 0, 0, 7, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 4, 0,
  4, 0, 0, 0, 0, 2, 0, 0,
  0, 0, 0, 0, 4, 0, 0, 0,
  0, 0, 7, 0, 0, 0, 0, 5,
  0, 0, 0, 0, 2, 0, 0, 0
};

std::array<int, 8> rowFront = { 23, 10, 13, 7, 16, 15, 9, -1 };
std::array<int, 8> rowBack = { -1, 20, 14, 22, 13, 15, 18, -1 };
std::array<int, 8> columnTop = { -1, 11, 24, -1, 13, 13, 14, 7 };
std::array<int, 8> columnBottom = { 12, 16, 7, -1, 21, 7, 14, 22 };

class DiagonalSkyscrapers : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  DiagonalSkyscrapers(const SizeOptions& opt)
    : Script(opt),
      x(*this, 64, 0, 7) {
    Matrix<IntVarArray> m{ x, 8, 8 };

    //zadana cisla
    for ( int row = 0; row < 8; ++row ) {
      for ( int column = 0; column < 8; ++column ) {
        if ( givens[row][column] != 0 ) {
          rel( *this, m( column, row ) == givens[row][column] );
        }
      }
    }

    //soucty viditelnych + distinct row, col
    for ( int i = 0; i < 8; ++i ) {
      sumVisibles( m.row( i ), rowFront[ i ], rowBack[ i ] );
      sumVisibles( m.col( i ), columnTop[ i ], columnBottom[ i ] );
    }

    //cisla jsou ruzny na hlavni diagonale
    auto mainDiag = x.slice( 0, 9 );
    distinct( *this, mainDiag );
    //cisla jsou ruzny na vedlejsi diagonale
    auto antiDiag = x.slice( 7, 7, 8 );
    distinct( *this, antiDiag );

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  void sumVisibles( IntVarArgs fields, int directVisiblesSum, int backwardVisiblesSum ) {
    sumVisibles( fields, directVisiblesSum );
    distinct( *this, fields );

    sumVisibles( fields.slice( fields.size() - 1, -1 ), backwardVisiblesSum );
  }

  void sumVisibles( IntVarArgs fields, int visiblesSum ) {
    IntVarArgs max{ *this, fields.size(), 0, 7 };
    BoolVarArray visible{ *this, fields.size(), 0, 1 };
    IntVarArray contribution{ *this, fields.size(), 0, 7 };
    for ( std::size_t i = 0; i < fields.size(); ++i ) {
      Gecode::max( *this, fields.slice( 0, 1, i + 1 ), max[ i ] );
      rel( *this, max[ i ], IRT_EQ, fields[ i ], visible[ i ] );
      //visible => contribution = fields
      rel( *this, contribution[ i ], IRT_EQ, fields[ i ], imp( visible[ i ] ) );
      //visible <= contribution != 0
      rel( *this, contribution[ i ], IRT_NQ, 0, pmi( visible[ i ] ) );
    }
    if ( visiblesSum != -1 ) {
      linear( *this, contribution, IRT_EQ, visiblesSum );
    }
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m{ x, 8, 8 };
    os << m;
  }
  /// Constructor for cloning \a s
  DiagonalSkyscrapers(DiagonalSkyscrapers& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new DiagonalSkyscrapers(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("DiagonalSkyscrapers (K&H, Logika 2022, 2. kolo, 7. uloha)");
  opt.parse(argc,argv);
  Script::run<DiagonalSkyscrapers,DFS,SizeOptions>(opt);
}
