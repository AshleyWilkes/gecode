#pragma once
#include <gecode/driver.hh>
#include <map>
#include <optional>

struct given {
  std::size_t row, column, length;
};

struct Coords {
  std::size_t row, column;
};

using Dir = std::pair<int, int>;
static Dir N{ -1, 0 }, E{ 0, 1 }, S{ 1, 0 }, W{ 0, -1 };
static std::array<Dir, 4> dirs{ N, E, S, W };
Dir operator-( const Dir& dir ); 
std::pair<int, int> operator*( Dir dir, int distance ); 
Coords operator+( Coords c1, std::pair<int, int> c2 ); 
bool operator<( const Coords& c1, const Coords& c2 );

class Tykadla;

class TykadlaHelper {
  //toto ma byt classa pro jedno policko s cislem;
  //zna svoje policka v kazdem smeru a upravuje jim pripustne hodnoty
  //zna svuj total a svoje min a max v kazdem smeru; inicializuji
  //  se min = 0, max = vzdalenost od kraje nebo nejblizsiho 
  //  jineho pole s cislem nebo hodnota given.length; 
  //  min znamena jiz jiste tim smerem vedouci tykadlo, max 
  //  znamena na jakou delku se timto smerem vedouci tykadlo 
  //  da nejvic natahnout
  //poskytuje metody pro zjisteni a pro upravu min a max
  //  getMin(), updateMin()
  //  updateMin() si overi, zda argument se nesnazi minimum zmensit,
  //    v tom pripade neudela nic
  //    pokud se ale updateMin() snazi minimum zvetsit nad aktualni
  //      maximum, je to error
  //  updateMax() si overi, zda se argument nesnazi maximum zvetsit,
  //    v tom pripade neudela nic
  //    pokud se ale updateMax() snazi maximum zmensit pod aktualni
  //      minimum, je to error
  private:
    struct DirData {
      std::size_t min, max;
      Gecode::ViewArray<Gecode::Int::IntView> fields;
      void update( Gecode::Home home, DirData& orig ) {
        min = orig.min;
        max = orig.max;
        fields.update( home, orig.fields );
      }
    };
    std::size_t index, total;
    Gecode::ViewArray<Gecode::Int::IntView> allFields;
    std::map<Dir, DirData> data;
  public:
    TykadlaHelper(){}
    TykadlaHelper( std::size_t index_, std::size_t total_, 
        std::map<Dir, Gecode::ViewArray<Gecode::Int::IntView>>& dirFields,
        Gecode::ViewArray<Gecode::Int::IntView>& allFields_ )
        : index{ index_ }, total{ total_ }, allFields{ allFields_ } {
      for ( const auto& dir : dirs ) {
        data.insert( { dir, 
            { 0, static_cast<std::size_t>( dirFields[ dir ].size() ), dirFields[ dir ] } } );
      }
      //std::cout << "Create: ";
      //print();
    }
    TykadlaHelper( Gecode::Home home, TykadlaHelper& orig ) : index{ orig.index }, total{ orig.total },
        allFields{ home, orig.allFields }, data{ orig.data } {
      for ( Dir dir : dirs ) {
        data[ dir ].update( home, orig.data[ dir ] );
      }
      //std::cout << "Copy: "; 
      //print();
    }
    void print() { 
      std::cout << index << " (" << total << ") = ";
      for ( Dir dir : dirs ) {
        std::cout << "[" << dir.first << "," << dir.second << ": " << data[ dir ].min << "/" << data[ dir ].max << "], ";
      }
      std::cout << '\n';
    }
    Gecode::ExecStatus updateWithGrid( Gecode::Home home ) {
      //ocekovat maxy; max nesmi byt vetsi nez pocet policek v prislusnem smeru, do nichz lze stale
      //  umistit helper.index; podle tohoto kriteria upraveny max nesmi byt ovsem mensi nez min
      for ( Dir dir : dirs ) {
        if ( data.contains( dir ) ) {
          for ( size_t distance = 1; distance < data[ dir ].max; ++distance ) {
            if ( ! data[ dir ].fields[ distance ].in( static_cast<int>( index ) ) ) {
              updateMax( home, dir, distance );
              break;
            }
          }
        }
      }
      return Gecode::ES_OK;
    }
    void update( Gecode::Home home, TykadlaHelper& orig ) {
      index = orig.index;
      total = orig.total;
      allFields.update( home, orig.allFields );
      for ( const auto& dirData : orig.data ) {
        Dir dir = dirData.first;
        data.insert( { dir, { 0, 0, {} } } );
        data[ dir ].update( home, orig.data[ dir ] );
      }
    }
    std::size_t getIndex() const {
      return index;
    }
    std::size_t getTotal() const {
      return total;
    }
    std::map<Dir, std::pair<std::size_t, std::size_t>> getDirsMinMax() {
      std::map<Dir, std::pair<std::size_t, std::size_t>> result;
      for ( Dir dir : dirs ) {
        result.insert({ dir, { data[ dir ].min, data[ dir ].max } });
      }
      return result;
    }
    Gecode::ExecStatus updateMin( Gecode::Home home, Dir dir, std::size_t min ) {
      //std::cout << "index: " << index << " min: " << min << ", data[ dir ].max " << data[ dir ].max << '\n';
      if ( min > data[ dir ].max ) {
        //throw 1;
        return Gecode::ES_FAILED;
      }
      data[ dir ].min = std::max( min, data[ dir ].min );
      for ( std::size_t i = 0; i < data[ dir ].min; ++i ) {
        GECODE_ME_CHECK( data[ dir ].fields[ i ].eq( home, static_cast<int>( index ) ) );
      }
      return Gecode::ES_OK;
    }
    Gecode::ExecStatus updateMax( Gecode::Home home, Dir dir, std::size_t max ) {
      //std::cout << "index: " << index << " max: " << max << ", data[ dir ].min " << data[ dir ].min << '\n';
      if ( max < data[ dir ].min ) {
        //throw 2;
        return Gecode::ES_FAILED;
      }
      data[ dir ].max = std::min( max, data[ dir ].max );
      for ( std::size_t i = data[ dir ].max; i < data[ dir ].fields.size(); ++i ) {
        //rel( home, data[ dir ].fields[ i ], Gecode::IRT_NQ, index );
        GECODE_ME_CHECK( data[ dir ].fields[ i ].nq( home, static_cast<int>( index ) ) );
      }
      return Gecode::ES_OK;
    }
    Gecode::ViewArray<Gecode::Int::IntView> getFields( Dir dir ) {
      return data[ dir ].fields;
    }
    Gecode::ViewArray<Gecode::Int::IntView> getFields() {
      return allFields;
    }
};

class TykadlaHelpers {
  private:
    std::vector<TykadlaHelper> helpers;
    std::map<Coords, std::size_t> rootHelpers;
    struct FieldHelperData{
      std::size_t index, distance;
    };
    std::map<Coords, std::map<Dir, FieldHelperData>> fieldHelpers;
  public:
    TykadlaHelpers() {}

    template<std::size_t width, std::size_t height>
    TykadlaHelpers( Gecode::Home home, const Gecode::IntVarArray& grid, 
        const std::array<std::array<int, width>, height>& givens ){
      //1. kolo: prochazet rows, cols
      for ( std::size_t row = 0; row < height; ++row ) {
        for ( std::size_t column = 0; column < width; ++column ) {
          //pro kazdou kombinaci se podivat, jestli v givens je -1
          if ( givens[row][column] == -1 ) {
            //pokud ano, zalozit polozku ve fieldHelpers s prazdnou mapou
            fieldHelpers.insert( { { row, column }, {} } );
          } else {
            //pokud ne, zalozit polozku v rootHelpers s patricnym indexem
            rootHelpers.insert( { { row, column }, rootHelpers.size() } );
          }
        }
      }

      helpers.resize( rootHelpers.size() );
      //2. kolo: prochazet polozky v rootHelpers
      for ( const auto& rootHelper : rootHelpers ) {
        Coords coords = rootHelper.first;
        std::map<Dir, Gecode::ViewArray<Gecode::Int::IntView>> fieldViews;
        Gecode::VarArgArray<Gecode::IntVar> allFieldVars;
        Gecode::ViewArray<Gecode::Int::IntView> allFieldViews;
        for ( const Dir& dir : dirs ) {
          Gecode::VarArgArray<Gecode::IntVar> fieldVars;
          //prochazet v kazdem ze 4 smeru policka tak dlouho, dokud nenarazim na okraj 
          //  gridu nebo jine policko s cislem, nikoli vsak dele, nez urcuje cislo 
          //  v tomto policku
          for ( std::size_t distance = 1; distance <= givens[coords.row][coords.column]; ++distance ) {
            Coords currentCoords = coords + dir * distance;
            //std::cout << coords.row << ", " << coords.column << ", " << distance << ", " << dir.first << ", " << dir.second << "      ";
            //std::cout << currentCoords.row << ", " << currentCoords.column << "\n";
            if ( currentCoords.row < 0 || currentCoords.row >= height 
                || currentCoords.column < 0 || currentCoords.column >= width
                || givens[currentCoords.row][currentCoords.column] != -1 ) {
              /*std::cout << "break: " << (currentCoords.row < 0) << ", " << 
                (currentCoords.row >= height) << ", " << (currentCoords.column < 0) 
                << ", " << (currentCoords.column >= width) << ", " << 
                (givens[currentCoords.row][currentCoords.column] != -1) << 
                ", " << givens[currentCoords.row][currentCoords.column] << '\n';*/
              break;
            }
            //kazde projite policko ulozit do fieldVars pro inicializaci TykadlaHelperu
            fieldVars << grid[ currentCoords.row * width + currentCoords.column];
            allFieldVars << grid[ currentCoords.row * width + currentCoords.column];
            //projitemu policku pridat do mapy prislusne info o tomto policku s cislem
            fieldHelpers[ currentCoords ][ -dir ] = { rootHelper.second, distance };
          }
          //std::cout << '\n';
          fieldViews[ dir ] = { home, fieldVars };
        }
        //zalozit TykadlaHelper v helpers 
        allFieldViews = { home, allFieldVars };
        helpers[ rootHelper.second ] = { rootHelper.second,
          static_cast<std::size_t>( givens[ coords.row ][ coords.column ] ), 
          fieldViews, allFieldViews };
      }
    } 

    void update( Gecode::Home home, TykadlaHelpers& orig ) {
      for ( auto& origHelper : orig.helpers ) {
        TykadlaHelper helper = origHelper;
        helper.update( home, origHelper );
        helpers.push_back( helper );
      }
      rootHelpers = orig.rootHelpers;
      fieldHelpers = orig.fieldHelpers;
    }
    //lze zavolat s libovolnymi validnimi souradnicemi, jedna-li se o pole s cislem,
    //  vrati to prislusneho helpera, jinak none
    std::optional<TykadlaHelper> getHelper( const Coords& coords ) { 
      if ( rootHelpers.contains( coords ) ) {
        return helpers[ rootHelpers[ coords ] ];
      } else {
        return {}; 
      }
    }

    //lze zavolat pouze se souradnicemi pole bez cisla, jinak error
    //vrati to ctverici, kde pro kazdy smer je bud Helper, ktery je v tomto
    //  smeru policku nejbliz, anebo je tam none, pokud v danem smeru nelezi
    //  zadne policko s cislem
    std::map<Dir, std::pair<TykadlaHelper, std::size_t>> getFieldHelpers( const Coords& coords ) const { 
      std::map<Dir, std::pair<TykadlaHelper, std::size_t>> result;
      for ( const auto& fieldHelper : fieldHelpers.at( coords ) ) {
        result.insert( { fieldHelper.first, 
            { helpers[fieldHelper.second.index], fieldHelper.second.distance } } );
      }
      return result; 
    }
};

void tykadlaCondition( Gecode::Home home, Coords coords, TykadlaHelpers& helpers, 
    Gecode::Int::IntView field, const Gecode::IntVarArgs& grid );
