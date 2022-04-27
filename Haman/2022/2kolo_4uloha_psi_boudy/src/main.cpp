#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <set>

using namespace Gecode;

/**
 * \brief %Example: Psi boudy
 *
 * On a 10x10 board there are 20 dog kennels. Place dogs to
 * some empty fields such that 1 dog belongs to each of the 
 * kennels. The field with the dog and the field with the
 * kennel must share an edge. No 2 fields occupied with dogs
 * may share an edge or a corner. Given numbers specify number
 * of dogs for each row and column of the board.
 *
 */

std::array<std::array<int, 10>, 10> kennels = {
  1, 0, 0, 1, 1, 0, 0, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 1, 0, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 1, 0, 0, 0, 1, 0, 1,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
  1, 0, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 1, 0, 0
};

std::array<int, 10> rows = { 3, 1, 3, 1, 3, 1, 3, 1, 3, 1 };
std::array<int, 10> columns = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
class Kennels {
  private:
    std::array<std::array<int, 10>, 10> values;
  public:
    Kennels() {
      int index = 1;
      for ( int row = 0; row < 10; ++row ) {
        for ( int column = 0; column < 10; ++column ) {
          values[row][column] = kennels[row][column] ? index++ : 0;
        }
      }
    }
    IntSet neighbors( int row, int column ) {
      std::set<int> result{ 0 };
      if ( row > 0 ) result.insert(values[row-1][column]);
      if ( row < 9 ) result.insert(values[row+1][column]);
      if ( column > 0 ) result.insert(values[row][column-1]);
      if ( column < 9 ) result.insert(values[row][column+1]);
      return IntSet{ std::vector<int>( result.begin(), result.end() ) };
    }
};
class DogKennels : public Script {
private:
  IntVarArray x;

public:
  /// Actual model
  DogKennels(const SizeOptions& opt)
    : Script(opt),
      //0 je policko bez psa
      //1-20 jsou indexy psu
      x(*this, 100, 0, 20) {

    Matrix<IntVarArray> m{ x, 10, 10 };
    Kennels kennels_;

    //v obrazci je kazde cislo prave jednou,
    for ( int i = 1; i < 21; ++i ) {
      count( *this, x, i, IRT_EQ, 1 );
    }

    //a to v policku sousedicim s prislusnou boudou
    for ( int row = 0; row < 10; ++row ) {
      for ( int column = 0; column < 10; ++column ) {
        dom( *this, m( column, row ), kennels_.neighbors( row, column ) );
      }
    }

    //v kazdem radku a sloupci je dan pocet nenulovych hodnot
    for ( int row = 0; row < 10; ++row ) {
      count( *this, m.row( row ), 0, IRT_EQ, 10 - rows[ row ] );
    }

    for ( int column = 0; column < 10; ++column ) {
      count( *this, m.col( column ), 0, IRT_EQ, 10 - columns[ column ] );
    }

    //dve nenulove hodnoty spolu nesousedi stranou
    for ( int row = 0; row < 8; ++row ) {
      for ( int col = 0; col < 9; ++col ) {
        IntVarArgs aR{ m(col,row), m(col,row+1) };
        IntVarArgs aC{ m(row,col), m(row+1,col) };
        count( *this, aR, 0, IRT_GR, 0 );
        count( *this, aC, 0, IRT_GR, 0 );
      }
    }

    //dve nenulove hodnoty spolu nesousedi rohem
    for ( int row = 0; row < 8; ++row ) {
      for ( int col = 0; col < 8; ++col ) {
        IntVarArgs aR{ m(col,row), m(col+1,row+1) };
        IntVarArgs aC{ m(row,col+1), m(row+1,col) };
        count( *this, aR, 0, IRT_GR, 0 );
        count( *this, aC, 0, IRT_GR, 0 );
      }
    }

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m(x, 10, 10);
    os << m;
  }
  /// Constructor for cloning \a s
  DogKennels(DogKennels& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new DogKennels(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("DogKennels (K&H, Logika 2022, 2. kolo, 4. uloha)");
  opt.parse(argc,argv);
  Script::run<DogKennels,DFS,SizeOptions>(opt);
}
