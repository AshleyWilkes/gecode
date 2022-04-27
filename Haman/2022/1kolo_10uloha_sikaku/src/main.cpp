#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <numeric>

using namespace Gecode;

/**
 * \brief %Example: Sikaku
 *
 * Rozdelte cely obrazec po naznacenych liniich 
 * na obdelniky a ctverce, ktere se vzajemne neprekryvaji.
 * V kazdem obdelniku a ctverci se musi nachazet prave
 * jedno cislo, ktere udava, z kolika policek se prislusny
 * obdelnik nebo ctverec sklada.
 *
 */

std::array<std::array<int, 11>, 11> givensArray {
  -1,  6, -1, -1, -1, -1, -1, -1,  7, -1, -1,
  -1, -1, -1,  3, -1, -1,  4, -1, -1, -1,  7,
  -1, -1, -1, -1, -1,  6, -1, -1,  2, -1, -1, 
  -1, -1,  9, -1, -1, -1, -1,  4, -1, -1, -1,
   5, -1, -1, -1,  4, -1,  6, -1, -1,  6, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1,  6, -1, -1, -1, -1, -1, -1, -1,  6, -1,
  -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1,
   4, -1, -1,  8, -1, -1, -1, -1, -1, -1,  6,
  -1, -1, -1, -1, 10, -1, -1, -1,  2, -1, -1,
  -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1
};

/*std::array<std::array<int, 3>, 3> givensArray {
   3, -1,  3,
  -1,  3, -1,
  -1, -1, -1
};*/

std::array<std::vector<std::pair<std::size_t, std::size_t>>, 9> sizeOptions {
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 2 }, { 2, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 3 }, { 3, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 4 }, { 2, 2 }, { 4, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 5 }, { 5, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 6 }, { 2, 3 }, { 3, 2 }, { 6, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 7 }, { 7, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 8 }, { 2, 4 }, { 4, 2 }, { 8, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 9 }, { 3, 3 }, { 9, 1 } },
  std::vector{ std::pair<std::size_t, std::size_t>{ 1, 10 }, { 2, 5 }, { 5, 2 }, { 10, 1 } }
};

class GivenNumberData {
  private:
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>> data;
    std::size_t first, last, number;
    template<std::size_t width, std::size_t height>
    bool canPlace( const std::array<std::array<int, width>, height>& givens, 
        int startRow, int startColumn, std::size_t rectangleHeight, std::size_t rectangleWidth, 
        std::size_t numberRow, std::size_t numberColumn ) {
      if ( startRow < 0 || startColumn < 0 || startRow + rectangleHeight > height
          || startColumn + rectangleWidth > width ) {
        return false;
      }
      for ( std::size_t r = startRow; r < startRow + rectangleHeight; ++r ) {
        for ( std::size_t c = startColumn; c < startColumn + rectangleWidth; ++c ) {
          if ( r != numberRow || c != numberColumn ) {
            if ( givens[ r ][ c] != -1 ) {
              return false;
            }
          }
        }
      }
      return true;
    }
  public:
    template<std::size_t width, std::size_t height>
    GivenNumberData( const std::array<std::array<int, width>, height>& givens, 
        std::size_t row, std::size_t column, std::size_t firstOptionValue ) {
      number = givens[ row ][ column ];
      for ( auto sizeOption : sizeOptions[ number - 2 ] ) {
        for ( int r = static_cast<int>( row ) - static_cast<int>( get<0>( sizeOption ) ) + 1; 
            r <= static_cast<int>( row ); ++r ) {
          for ( int c = static_cast<int>( column ) - static_cast<int>( get<1>( sizeOption ) ) + 1; 
              c <= static_cast<int>( column ); ++c ) {
            if ( canPlace( givens, r, c, get<0>( sizeOption ), get<1>( sizeOption ), row, column ) ) {
              data.push_back({ r, r + get<0>( sizeOption ) - 1, c, c + get<1>( sizeOption ) - 1 });
            }
          }
        }
      }
      first = firstOptionValue;
      last = first + data.size() - 1;
    }
    std::size_t getNumOptionValues() const { return data.size(); }
    std::size_t getFirstOptionValue() const { return first; }
    std::size_t getLastOptionValue() const { return last; }
    std::size_t getTopRow( std::size_t optionValue ) const { 
      return std::get<0>( data[ optionValue - first ] );
    }
    std::size_t getBottomRow( std::size_t optionValue ) const {
      return std::get<1>( data[ optionValue - first ] );
    }
    std::size_t getLeftColumn( std::size_t optionValue ) const {
      return std::get<2>( data[ optionValue - first ] );
    }
    std::size_t getRightColumn( std::size_t optionValue ) const {
      return std::get<3>( data[ optionValue - first ] );
    }
    std::size_t getNumber() const { return number; }
};

class Givens {
  private:
    std::size_t width, height;
    std::vector<GivenNumberData> data;
  public:
    Givens() {}
    template<std::size_t width_, std::size_t height_>
    Givens( const std::array<std::array<int, width_>, height_>& givens ) : 
        width{ width_ }, height{ height_ } {
      for ( std::size_t row = 0; row < height; ++row ) {
        for ( std::size_t column = 0; column < width; ++column ) {
          if ( givens[ row ][ column ] != -1 ) {
            data.push_back({ givens, row, column, getNumOptions() });
          }
        }
      }
    }
    std::size_t getHeight() const{ return height; }
    std::size_t getWidth() const{ return width; }

    std::size_t getNumOptions() const { return std::accumulate( data.begin(), data.end(), 0,
        []( std::size_t cur, const GivenNumberData& datum ) 
        { return cur + datum.getNumOptionValues(); } ); }
    std::vector<GivenNumberData> getGivenNumbersData() const { return data; }
};

class Sikaku : public Script {
private:
  Givens givens;
  IntVarArray x;

public:
  /// Actual model
  Sikaku(const SizeOptions& opt)
      : Script(opt),
      givens{ givensArray },
      x( *this, givens.getWidth() * givens.getHeight(), 1, givens.getNumOptions() ) {
    //pro kazde zadane cislo existuje givens.getNumOptions( numberIndex ) a 
    //  givens.getNumFirstOptionValue( numberIndex ) a givens.getNumLastOptionValue( numberIndex )
    //pro kazdou hodnotu z jimi urceneho intervalu je treba:
    //1) pripustit hodnotu prave do policek, kterych se tyka
    //2) mit bool variablu, zda prave tato hodnota je pro zadane cislo pouzita
    //3) omezit count hodnoty na 0 nebo prislusny pocet poli, reified by the bool var
    //4) omezit count pres bool vars pro dane zadane cislo na 1

    Matrix<IntVarArray> m{ x, static_cast<int>( givens.getWidth() ), 
      static_cast<int>( givens.getHeight() ) };
    for ( GivenNumberData givenNumber : givens.getGivenNumbersData() ) {
      BoolVarArray used{ *this, static_cast<int>( givenNumber.getNumOptionValues() ), 0, 1 };
      IntVarArray counts{ *this, static_cast<int>( givenNumber.getNumOptionValues() ), 
        IntSet({ 0, static_cast<int>( givenNumber.getNumber() ) }) };
      linear( *this, used, IRT_EQ, 1 );
      for ( std::size_t optionValue = givenNumber.getFirstOptionValue(); 
          optionValue <= givenNumber.getLastOptionValue(); ++optionValue ) {
        for ( std::size_t row = 0; row < givens.getHeight(); ++row ) {
          for ( std::size_t column = 0; column < givens.getWidth(); ++column ) {
            if ( row < givenNumber.getTopRow( optionValue ) 
                || row > givenNumber.getBottomRow( optionValue ) 
                || column < givenNumber.getLeftColumn( optionValue ) 
                || column > givenNumber.getRightColumn( optionValue ) ) {
              rel( *this, m( column, row ) != ( optionValue + 1) );
            }
          }
        }
        count( *this, x, optionValue + 1, IRT_EQ, 
            counts[ optionValue - givenNumber.getFirstOptionValue() ] );
        rel( *this, counts[ optionValue - givenNumber.getFirstOptionValue() ], IRT_NQ, 0, 
            used[ optionValue - givenNumber.getFirstOptionValue() ] );
        linear( *this, used, IRT_EQ, 1 );
      }
    }
    
    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> matrix( x, 11, 11 );
    os << "This is the solution!!!!\n";
    os << matrix << '\n';
  }
  /// Constructor for cloning \a s
  Sikaku(Sikaku& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Sikaku(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Sikaku (K&H, Logika 2022, 1. kolo, 10. uloha)");
  opt.parse(argc,argv);
  Script::run<Sikaku,DFS,SizeOptions>(opt);
}
