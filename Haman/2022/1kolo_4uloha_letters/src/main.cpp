#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Letters
 *
 * Place letters A, B, C, D to the grid so that each is exactly once
 * in every row and column. For some rows and columns the first
 * letter visible in a direction is given.
 *
 */

std::array<int, 6> rowFront = { -1, 3, 1, 3, -1, -1 };
std::array<int, 6> rowBack = { 4, 1, 4, -1, 1, -1 };
std::array<int, 6> columnTop = { 3, -1, -1, -1, -1, -1 };
std::array<int, 6> columnBottom = { -1, 3, -1, 1, 4, -1 };
class Letters : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  Letters(const SizeOptions& opt)
    : Script(opt),
      x(*this, 36, 0, 4) {

    Matrix<IntVarArray> m(x, 6, 6);

    for ( int i = 0; i < 6; ++i ) {
      //1) pismena jsou ruzny v kazdym radku
      distinct( *this, m.row(i), 0 );
      //2) v kazdym radku jsou 2 nuly
      count( *this, m.row(i), 0, IRT_EQ, 2 );
      //3) pismena jsou ruzny v kazdym sloupci
      distinct( *this, m.col(i), 0 );
      //4) v kazdym sloupci jsou 2 nuly
      count( *this, m.col(i), 0, IRT_EQ, 2 );
      //5) rowFront je prvni viditelne v radku zleva, kdyz neni -1
      if ( rowFront[i] != -1 ) {
        rel( *this, m(0,i) == rowFront[i] || ( m(0,i) == 0 && m(1,i) == rowFront[i] ) 
            || (m(0,i) == 0 && m(1,i) == 0 && m(2,i) == rowFront[i] ) );
      }
      //6) rowBack je prvni viditelne v radku zprava, kdyz neni -1
      if ( rowBack[i] != -1 ) {
        rel( *this, m(5,i) == rowBack[i] || ( m(5,i) == 0 && m(4,i) == rowBack[i] ) 
            || (m(5,i) == 0 && m(4,i) == 0 && m(3,i) == rowBack[i] ) );
      }
      //7) columnTop je prvni viditelne v sloupci seshora, kdyz neni -1
      if ( columnTop[i] != -1 ) {
        rel( *this, m(i,0) == columnTop[i] || ( m(i,0) == 0 && m(i,1) == columnTop[i] ) 
            || (m(i,0) == 0 && m(i,1) == 0 && m(i,2) == columnTop[i] ) );
      }
      //8) columnBottom je prvni viditelne v sloupci zespodu, kdyz neni -1
      if ( columnBottom[i] != -1 ) {
        rel( *this, m(i,5) == columnBottom[i] || ( m(i,5) == 0 && m(i,4) == columnBottom[i] ) 
            || (m(i,5) == 0 && m(i,4) == 0 && m(i,3) == columnBottom[i] ) );
      }
    }

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for ( int r = 0; r < 6; ++r ) {
      for ( int c = 0; c < 6; ++c ) {
        os << x[r*6+c] << '\t';
      }
      os << '\n';
    }
  }
  /// Constructor for cloning \a s
  Letters(Letters& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Letters(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Letters (K&H, Logika 2022, 1. kolo, 4. uloha)");
  opt.parse(argc,argv);
  Script::run<Letters,DFS,SizeOptions>(opt);
}
