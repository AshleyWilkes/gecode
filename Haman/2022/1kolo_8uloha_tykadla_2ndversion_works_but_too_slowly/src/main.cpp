#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Tykadla
 *
 * Z každého políčka s číslem veďtě vodorovně nebo svisle jedno nebo více
 * tykadel.Číslo udává součet délek všech tykadel, přičemž políčkem s číslem 
 * se do tohoto součtu nepočítá. Tykadla se nesmějí křížit ani vzájemně
 * dotýkat, vedou přes středy políček a každé políčko bez čísla musí být 
 * obsazeno právě jedním tykadlem.
 *
 */

//std::array<std::array<int, 10>, 12> givensArrayReal = {
std::array<std::array<int, 10>, 12> givensArray = {
  -1, 11, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,  8, -1,
  -1, -1,  6, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,  6, -1, -1, -1,
  13, -1, -1, -1, -1, -1, -1, -1, -1,  9,
  -1,  8, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1,  6, -1, -1, -1,  2, -1, -1,
  -1, -1, -1, -1, -1,  5, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,  7, -1,
  -1, -1, -1, -1,  4, -1, -1, -1, -1, -1,
  -1, -1,  5, -1, -1, -1, -1, -1, -1,  7,
  -1, -1, -1, -1,  8, -1, -1, -1, -1, -1
};

std::array<std::array<int, 3>, 2> givensArrayTest = {
//std::array<std::array<int, 3>, 2> givensArray = {
  3, -1, -1,
  -1, 1, -1
};

//std::size_t givenW = 10, givenH = 12, numGivens = 15;
//std::size_t givenW = 3, givenH = 2, numGivens = 2;
/*std::vector<given> givens = {
  { 0, 1, 11 },//0
  { 1, 8,  8 },//1
  { 2, 2,  6 },
  { 3, 6,  6 },//3
  { 4, 0, 13 },
  { 4, 9,  9 },
  { 5, 1,  8 },//6
  { 6, 3,  6 },
  { 6, 7,  2 },
  { 7, 5,  5 },//9
  { 8, 8,  7 },
  { 9, 4,  4 },
  {10, 2,  5 },//12
  {10, 9,  7 },
  {11, 4,  8 }
};*/

class TykadloData {
  private:
    struct Delta {
      int row, column;
    };
    std::array<Delta, 4> dirs{ Delta{ -1, 0 }, Delta{ 0, 1 }, Delta{ 1, 0 }, Delta{ 0, -1 } };
    std::size_t index;
    int value, maxLength = 0;
    std::vector<std::size_t> fieldIndices;
  public:
    TykadloData() {}
    template<std::size_t width, std::size_t height>
    TykadloData( std::size_t numberIndex, std::size_t index_, std::size_t row, std::size_t column, 
        const std::array<std::array<int, width>, height>& givensArray ) : index{ index_ }, 
        value{ static_cast<int>( 5 * numberIndex + index + 1 ) } {
      Delta dir = dirs[ index ];
      std::size_t curRow = row + dir.row, curColumn = column + dir.column, distance = 1;
      while ( curRow < height && curColumn < width && givensArray[ curRow ][ curColumn ] == -1 ) {
        fieldIndices.push_back( curRow * width + curColumn );
        maxLength = distance;
        curRow += dir.row;
        curColumn += dir.column;
        distance++;
      }
      print();
    }
    void print() {
      std::cout << "TykadloData(" << value << ", " << index << ", " << maxLength << "): ";
      for ( auto i : fieldIndices ) {
        std::cout << i << ", ";
      }
      std::cout << '\n';
    }
    int getMaxLength() const { return maxLength; }
    std::vector<std::size_t> getFieldIndices() const { return fieldIndices; }
    Int::IntView getField( const IntVarArray& grid, std::size_t index ) const { 
      return grid[ fieldIndices[ index ] ]; 
    }
    int getValue() const { return value; }
    std::size_t getIndex() const { return index; }
};

class GivenNumberData {
  private:
    std::array<TykadloData, 4> tykadlaData;
    int maxLength = 0;
    std::size_t number = 0;
  public:
    template<std::size_t width, std::size_t height>
    GivenNumberData( std::size_t row, std::size_t column, std::size_t numberIndex,
        const std::array<std::array<int, width>, height>& givensArray ) {
      for ( std::size_t i = 0; i < 4; ++i ) {
        tykadlaData[ i ] = TykadloData{ numberIndex, i, row, column, givensArray };
        if ( maxLength < tykadlaData[ i ].getMaxLength() ) {
          maxLength = tykadlaData[ i ].getMaxLength();
        }
      }
      number = givensArray[ row ][ column ];
    }
    int getMaxLength() const { return maxLength; }
    std::array<TykadloData, 4> getTykadlaData() const { return tykadlaData; }
    std::size_t getNumber() const { return number; }
};

class Givens {
  private:
    std::size_t width, height;
    std::vector<GivenNumberData> numbersData;
    std::vector<IntArgs> doms;
  public:
    Givens() {}
    template<std::size_t width_, std::size_t height_>
    Givens( const std::array<std::array<int, width_>, height_>& givensArray ) : 
        width{ width_ }, height{ height_ } {
      doms.resize( width * height );
      for ( std::size_t row = 0; row < height; ++row ) {
        for ( std::size_t column = 0; column < width; ++column ) {
          if ( givensArray[ row ][ column ] != -1 ) {
            std::size_t index = numbersData.size();
            doms[ row * width + column ] << 5 * index;
            GivenNumberData newNumberData{ row, column, index, givensArray };
            numbersData.push_back( newNumberData );

            for ( auto tykadloData : newNumberData.getTykadlaData() ) {
              for ( auto fieldIndex : tykadloData.getFieldIndices() ) {
                doms[ fieldIndex ] << tykadloData.getValue();
              }
            }
          }
        }
      }
      
    }
    std::size_t getWidth() const { return width; }
    std::size_t getHeight() const { return height; }
    std::size_t getNumbersCount() const { return numbersData.size(); }
    IntArgs getDom( std::size_t row, std::size_t column ) const { return doms.at( row * width + column ); }
    std::vector<GivenNumberData> getNumbersData() const { return numbersData; }
};

//jak na to:
//jednak mit IntVarArray pro celej grid, to je jasny
//dvak ale resit kazdy jednotlivy tykadlo zvlast
//tzn. hodnoty v gridu necht jsou [0, 5*pocetPolicekSCislem)
//pro 1. policko s cislem:
//  0 necht je hodnota v tom policku
//  1-4 necht jsou hodnoty pro jednotlive smery (1 - N, 2 - E, 3 - S, 4 - W)
//pro kazde tykadlo mit pomocne pole BoolVarArray tak dlouhe, jak 
//  hypotechnicky muze tykadlo v danem smeru byt
//boolVar[ i + 1 ] == bolVar[ i ] && prislusne policko obsahuje prislusnou hodnotu
//dale mit pro kazde tykadlo pomocny IntVar, oznacujici jeho delku

class Tykadla : public Script {
private:
  Givens givens;
  IntVarArray actualValues;
  std::size_t givenW, givenH;
public:
  /// Actual model
  Tykadla(const SizeOptions& opt)
      : Script(opt),
      givens{ givensArray },
      actualValues(*this, givens.getWidth() * givens.getHeight(), 0, 5 * givens.getNumbersCount() - 1),
      givenW{ givens.getWidth() }, givenH{ givens.getHeight() } {

    Matrix<IntVarArray> actualValuesM( actualValues, givens.getWidth(), givens.getHeight() );
    //trace( *this, actualValues );

    for ( std::size_t row = 0; row < givens.getHeight(); ++row ) {
      for ( std::size_t column = 0; column < givens.getWidth(); ++column ) {
        //std::cout << "dom: row=" << row << ", column=" << column << ", dom: " << givens.getDom( row, column ) << '\n';
        dom( *this, actualValuesM( column, row ), IntSet{ givens.getDom( row, column ) } );
      }
    }

    for ( const auto& givenNumberData : givens.getNumbersData() ) {
      std::cout << givenNumberData.getNumber() << ", " << givenNumberData.getMaxLength() << '\n';
      IntVarArray tykadlaLength{ *this, 4, 0, givenNumberData.getMaxLength() };
      //trace( *this, tykadlaLength );
      for ( const auto& tykadloData : givenNumberData.getTykadlaData() ) {
        if ( tykadloData.getMaxLength() > 0 ) {
          BoolVarArray tykadloBools{ *this, tykadloData.getMaxLength(), 0, 1 };
          //trace( *this, tykadloBools );
          std::cout << "First: " << tykadloData.getValue() << "\n";
          rel( *this, ( tykadloData.getField( actualValues, 0 ) == tykadloData.getValue() ) 
              == tykadloBools[ 0 ] );
          for ( int i = 1; i < tykadloData.getMaxLength(); ++i ) {
            std::cout << "Next (" << i << ")\n";
            rel( *this, ( ( tykadloData.getField( actualValues, i ) == tykadloData.getValue() ) 
                  && tykadloBools[ i - 1 ] ) == tykadloBools[ i ] );
          }
          std::cout << "linear\n";
          linear( *this, tykadloBools, IRT_EQ, tykadlaLength[ tykadloData.getIndex() ] );
        } else {
          std::cout << "no tykadlo\n";
          rel( *this, tykadlaLength[ tykadloData.getIndex() ], IRT_EQ, 0 );
        }
      }
      std::cout << "total: " << givenNumberData.getNumber() << '\n';
      linear( *this, tykadlaLength, IRT_EQ, givenNumberData.getNumber() );
    }

    branch(*this, actualValues, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> actualValuesM( actualValues, givenW, givenH );
    os << actualValuesM << '\n';
  }
  /// Constructor for cloning \a s
  Tykadla(Tykadla& s) : Script(s) {
    actualValues.update(*this, s.actualValues);
    givenW = s.givenW;
    givenH = s.givenH;
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Tykadla(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Tykadla (K&H, Logika 2022, 1. kolo, 8. uloha)");
  opt.parse(argc,argv);
  Script::run<Tykadla,DFS,SizeOptions>(opt);
}
