#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <map>
#include <set>

#include "my_script.h"

//nyni tedy nova verze. Necht se jedna o pokus o vytvoreni
//normalne pouzitelne "sablony", ktera umozni psat ulohy
//mnohem jednoduseji nez dosud, a to ulohy obecne bez ohledu
//na to, zda je lze snadno specifikovat pomoci Gecode
//constraintu cili nic.
//
//Predpokladam 3 classy a main. Classy:
//1) Neco, co je potomkem Script (mozna jen Space) a ma
//  to virtualni metody pro zadavani propagatoru a branchi.
//  Nejak chci specifikovat updatovatelne promenne, abych
//  je nemusel rucne updatovat: Classu je templatovana parametr 
//  packem, kde kazdy parametr z packu predstavuje jednu 
//  updatovatelnou promennou. Pro kazdy tento parametr pak 
//  existuje intem templatovana variable templata, jez odpovida
//  te updatovatelne promenne, prislusne danemu intu.
//2) Propagator (optional, zatim ne)
//3) Brancher (optional, zatim ne)
//4) Metoda main, ktera *POUZE* preposle argc a argv jine metode
//  templatovane prinejmensim na typ z bodu 1)

/**
 * \brief %Example: HexaArrows
 *
 * Place arrows into fields adjacent to the
 * board. Given numbers specify number of arrows
 * pointing to each field of the board.
 *
 */

//je to hexa 5,9; tj. plny hexagon, jehoz nejkratsi
// strana ma 5 poli a nejdelsi ma 9 poli, celkem
// tedy 61 poli ( 5 + 6 + 7 + 8 + 9 + 8 + 7 + 6 + 5 )

std::array<int, 61> givens = {
          2,  1,  1,  4,  1,
        4,  1,  2,  4,  3,  4,
      4,  2,  1,  4,  2,  5,  2,
    4,  2,  2,  3,  2,  4,  3,  4,
  3,  1,  1,  3,  0,  3,  1,  4,  2,
    1,  1,  3,  1,  2,  1,  3,  3,
      2,  4,  2,  4,  1,  4,  3,
        5,  3,  5,  3,  4,  4,
          1,  3,  1,  3,  1
};

class Hexa {
  public:
    enum class Dirs{ SW, W, NW };
    static constexpr std::initializer_list<Dirs> allDirs{ Dirs::SW, Dirs::W, Dirs::NW };
  private:
    int short_, long_;
    std::map<Dirs, std::vector<std::vector<int>>> fieldIds = {
      { Dirs::W, {
        { 0, 1, 2, 3, 4 },
        { 5, 6, 7, 8, 9, 10 },
        { 11, 12, 13, 14, 15, 16, 17 },
        { 18, 19, 20, 21, 22, 23, 24, 25 },
        { 26, 27, 28, 29, 30, 31, 32, 33, 34 },
        { 35, 36, 37, 38, 39, 40, 41, 42 },
        { 43, 44, 45, 46, 47, 48, 49 },
        { 50, 51, 52, 53, 54, 55 },
        { 56, 57, 58, 59, 60 }}},
      { Dirs::SW, {
        { 0, 5, 11, 18, 26 },
        { 1, 6, 12, 19, 27, 35 },
        { 2, 7, 13, 20, 28, 36, 43 },
        { 3, 8, 14, 21, 29, 37, 44, 50 },
        { 4, 9, 15, 22, 30, 38, 45, 51, 56 },
        { 10, 16, 23, 31, 39, 46, 52, 57 },
        { 17, 24, 32, 40, 47, 53, 58 },
        { 25, 33, 41, 48, 54, 59 },
        { 34, 42, 49, 55, 60 }}},
      { Dirs::NW, {
        { 26, 35, 43, 50, 56 },
        { 18, 27, 36, 44, 51, 57 },
        { 11, 19, 28, 37, 45, 52, 58 },
        { 5, 12, 20, 29, 38, 46, 53, 59 },
        { 0, 6, 13, 21, 30, 39, 47, 54, 60 },
        { 1, 7, 14, 22, 31, 40, 48, 55 },
        { 2, 8, 15, 23, 32, 41, 49 },
        { 3, 9, 16, 24, 33, 42 },
        { 4, 10, 17, 25, 34 }}}
    };

    int dirLength( int dirIndex ) {
      return dirIndex < long_ - short_ ? short_ + dirIndex : long_ + short_ - dirIndex - 1;
    }
  public:
    Hexa( int shortRow, int longRow ) : short_{ shortRow }, long_{ longRow } {}
    //vrati indexy policek, ktera lezi v prislusnem smeru
    std::vector<int> getDir( Dirs dir, int dirIndex ) {
      return fieldIds[ dir ][ dirIndex ];
    }
    /*std::set<int> getDir( Dirs dir, int dirIndex ) {
      std::set<int> result;
      //urcit pocet policek v danem smeru [dir, dirIndex]
      int fieldCount = dirLength( dirIndex );
      for ( int i = 0; i < fieldCount; ++i ) {
        result.insert( fieldIds[ dir ][ dirIndex ][ i ] );
      }
      return result;
    }*/
};

constexpr std::initializer_list<Hexa::Dirs> Hexa::allDirs;

class HexaArrowsOld : public Script {
private:
  BoolVarArray b;
  std::vector<BoolVarArray> fieldsBools;

public:
  /// Actual model
  HexaArrowsOld(const SizeOptions& opt)
    : Script(opt),

      //0 - 3 je horni rada sipek zleva doprava a pak dale po smeru hodinovych rucicek
      //hodnota 0 v policku znamena sipka ve smeru pohledu z policka doprava, hodnota 1
      //doleva
      b(*this, 24, 0, 1),
      //kazdy bool s indexem i znamena "miri na me sipka ve smeru i?"
      //smery jsou usporadane SW, NE, W, E, NW, SE
      fieldsBools(61, BoolVarArray(*this, 6, 0, 1)) {


    for ( int i = 0; i < 61; ++i ) {
      fieldsBools[ i ] = BoolVarArray(*this, 6, 0, 1);
      branch( *this, fieldsBools[i], BOOL_VAR_NONE(), BOOL_VAL_MIN() );
    }
    for ( int i = 0; i < 1; ++i ) {
      linear( *this, fieldsBools[i], IRT_EQ, givens[i] );
      //linear( *this, fieldsBools[i], IRT_EQ, 1 );
    }

    makeDirs(Hexa::Dirs::SW, 0, 4, 0, true );
    //makeDirs(Hexa::Dirs::SW, 1, 3, 5, false );
    //makeDirs(Hexa::Dirs::W, 1, 5, 0, true );
    //makeDirs(Hexa::Dirs::W, 2, 4, 5, false );
    //makeDirs(Hexa::Dirs::NW, 2, 0, 0, true );
    //makeDirs(Hexa::Dirs::NW, 3, 5, 5, false );

    //branch( *this, b, BOOL_VAR_NONE(), BOOL_VAL_MIN() );
  }

  void makeDirs( Hexa::Dirs dir, int areaWithRightArrowIndex, int areaWithLeftArrowIndex, int dirDelta, bool boolDir ) {
    for ( int i = 0; i < 4; ++i ) {
      makeDir(dir, i + dirDelta, 4 * areaWithRightArrowIndex + i, 4 * areaWithLeftArrowIndex + 3 - i, boolDir );
    }
  }

  void makeDir( Hexa::Dirs dir, int dirIndex, int fieldWithRightArrowIndex, int fieldWithLeftArrowIndex, bool boolDir ) {
    //zdehle dir a dirIndex popisuji region v givens 
    //fieldWithRightArrowIndex popisuje pole se sipkou, ktere se pocita, pokud sipka miri vpravo
    //fieldWithLeftArrowIndex popisuje pole se sipkou, ktere se pocita, pokud sipka miri vlevo
    Hexa topology{ 5, 9 };
    for ( int fieldIndex : topology.getDir( dir, dirIndex ) ) {
      //kazde policko ma svoji sestici boolu
      auto& fieldBools = fieldsBools[ fieldIndex ];
      int boolIndex = ( dir == Hexa::Dirs::SW ) ? 0 : ( dir == Hexa::Dirs::W ? 2 : 4 );
      
      std::cout << dirIndex << ", " << fieldIndex << ":: " << fieldWithRightArrowIndex << ", " << ( boolIndex + boolDir ? 0 : 1 ) << '\n';
      std::cout << dirIndex << ", " << fieldIndex << ":: " << fieldWithLeftArrowIndex << ", " << ( boolIndex + boolDir ? 1 : 0 )  << '\n';
      rel( *this, b[ fieldWithRightArrowIndex ], IRT_EQ, 0, fieldBools[ boolIndex + ( boolDir ? 0 : 1 ) ] );
      rel( *this, b[ fieldWithLeftArrowIndex ], IRT_EQ, 1, fieldBools[ boolIndex + ( boolDir ? 1 : 0 ) ] );
      //zde chci propojit vhodny z onech sesti boolu s tim, ze ve fieldWithRightArrowIndex je 0
      //a jiny z onech sesti s tim, ze ve fieldWithLeftArrowIndex je 1
    }
  }

  int printLine( std::ostream& os, int numTabs, int firstIndex, int numFields ) const {
    for ( int i = 0; i < 2 * numTabs; ++i ) {
      os << ' ';
    }
    for ( int i = 0; i < numFields; ++i ) {
      //os << givens[firstIndex + i ] << "   ";
      auto bools = fieldsBools[ firstIndex + i ];
      int count = 0;
      for ( int i = 0; i < 6; ++i ) {
        if ( bools[i].val() ) ++count;
      }
      os << count << "   ";
    }
    os << '\n';
    return numFields;
  }

  void printHexa( std::ostream& os, int shortLineSize, int longLineSize ) const {
    os << b << "\n\n";
    for ( int i = 0; i < 61; ++i ) {
      os << fieldsBools[i] << '\n';
    }
    int index = 0;
    for ( int i = shortLineSize; i < longLineSize; ++i ) {
      int numTabs = longLineSize - i;
      index += printLine( os, numTabs, index, i );
    }
    for ( int i = longLineSize; i >= shortLineSize; --i ) {
      int numTabs = longLineSize - i;
      index += printLine( os, numTabs, index, i );
    }
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    //toto by se melo prepsat do Hexa<IntVarArray> m{ x, 5, 5 }
    //necht je hexa defaultne orientovane tak, ze nektere
    //  hrany jsou svisle (as opposed to nektere hrany vodorovne)
    //  pak totiz dava smysl kreslit hexa tak, ze mezi kazdymi
    //  dvema v radku sousedicimi hodnotami je jeden \t
    //  a jedna se tedy vlastne o vetsi obdelnik
    //cili mych 5, 5 rika: prvni radek ma 5 poli
    //  nejdelsi radek ma 5+5-1 poli
    //printLine potrebuje vedet, kolik tabu na zacatku a kolik poli
    //  ma vypsat a taky ktere prvni pole k vypsani
    printHexa( os, 5, 9 );
  }
  /// Constructor for cloning \a s
  HexaArrowsOld(HexaArrowsOld& s) : Script(s), 
      fieldsBools(61, BoolVarArray(*this, 6, 0, 1)) {
    b.update(*this, s.b);
    for ( int i = 0; i < s.fieldsBools.size(); ++i ) {
      fieldsBools[i].update( *this, s.fieldsBools[i] );
    }
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new HexaArrowsOld(*this);
  }

};

std::map<std::pair<int, int>, std::pair<Hexa::Dirs, int>> boolToDir = {
  { { 0, 0 }, { Hexa::Dirs::SW, 0 } },
  { { 0, 1 }, { Hexa::Dirs::NW, 5 } },
  { { 1, 0 }, { Hexa::Dirs::SW, 1 } },
  { { 1, 1 }, { Hexa::Dirs::NW, 6 } },
  { { 2, 0 }, { Hexa::Dirs::SW, 2 } },
  { { 2, 1 }, { Hexa::Dirs::NW, 7 } },
  { { 3, 0 }, { Hexa::Dirs::SW, 3 } },
  { { 3, 1 }, { Hexa::Dirs::NW, 8 } },
  { { 4, 0 }, { Hexa::Dirs::W, 0 } },
  { { 4, 1 }, { Hexa::Dirs::SW, 5 } },
  { { 5, 0 }, { Hexa::Dirs::W, 1 } },
  { { 5, 1 }, { Hexa::Dirs::SW, 6 } },
  { { 6, 0 }, { Hexa::Dirs::W, 2 } },
  { { 6, 1 }, { Hexa::Dirs::SW, 7 } },
  { { 7, 0 }, { Hexa::Dirs::W, 3 } },
  { { 7, 1 }, { Hexa::Dirs::SW, 8 } },
  { { 8, 0 }, { Hexa::Dirs::NW, 8 } },
  { { 8, 1 }, { Hexa::Dirs::W, 5 } },
  { { 9, 0 }, { Hexa::Dirs::NW, 7 } },
  { { 9, 1 }, { Hexa::Dirs::W, 6 } },
  { { 10, 0 }, { Hexa::Dirs::NW, 6 } },
  { { 10, 1 }, { Hexa::Dirs::W, 7 } },
  { { 11, 0 }, { Hexa::Dirs::NW, 5 } },
  { { 11, 1 }, { Hexa::Dirs::W, 8 } },
  { { 12, 0 }, { Hexa::Dirs::SW, 8 } },
  { { 12, 1 }, { Hexa::Dirs::NW, 3 } },
  { { 13, 0 }, { Hexa::Dirs::SW, 7 } },
  { { 13, 1 }, { Hexa::Dirs::NW, 2 } },
  { { 14, 0 }, { Hexa::Dirs::SW, 6 } },
  { { 14, 1 }, { Hexa::Dirs::NW, 1 } },
  { { 15, 0 }, { Hexa::Dirs::SW, 5 } },
  { { 15, 1 }, { Hexa::Dirs::NW, 0 } },
  { { 16, 0 }, { Hexa::Dirs::W, 8 } },
  { { 16, 1 }, { Hexa::Dirs::SW, 3 } },
  { { 17, 0 }, { Hexa::Dirs::W, 7 } },
  { { 17, 1 }, { Hexa::Dirs::SW, 2 } },
  { { 18, 0 }, { Hexa::Dirs::W, 6 } },
  { { 18, 1 }, { Hexa::Dirs::SW, 1 } },
  { { 19, 0 }, { Hexa::Dirs::W, 5 } },
  { { 19, 1 }, { Hexa::Dirs::SW, 0 } },
  { { 20, 0 }, { Hexa::Dirs::NW, 0 } },
  { { 20, 1 }, { Hexa::Dirs::W, 3 } },
  { { 21, 0 }, { Hexa::Dirs::NW, 1 } },
  { { 21, 1 }, { Hexa::Dirs::W, 2 } },
  { { 22, 0 }, { Hexa::Dirs::NW, 2 } },
  { { 22, 1 }, { Hexa::Dirs::W, 1 } },
  { { 23, 0 }, { Hexa::Dirs::NW, 3 } },
  { { 23, 1 }, { Hexa::Dirs::W, 0 } }
};

std::map<Hexa::Dirs, std::vector<std::vector<int>>> fieldIds = {
  { Hexa::Dirs::W, {
    { 0, 1, 2, 3, 4 },
    { 5, 6, 7, 8, 9, 10 },
    { 11, 12, 13, 14, 15, 16, 17 },
    { 18, 19, 20, 21, 22, 23, 24, 25 },
    { 26, 27, 28, 29, 30, 31, 32, 33, 34 },
    { 35, 36, 37, 38, 39, 40, 41, 42 },
    { 43, 44, 45, 46, 47, 48, 49 },
    { 50, 51, 52, 53, 54, 55 },
    { 56, 57, 58, 59, 60 }}},
  { Hexa::Dirs::SW, {
    { 0, 5, 11, 18, 26 },
    { 1, 6, 12, 19, 27, 35 },
    { 2, 7, 13, 20, 28, 36, 43 },
    { 3, 8, 14, 21, 29, 37, 44, 50 },
    { 4, 9, 15, 22, 30, 38, 45, 51, 56 },
    { 10, 16, 23, 31, 39, 46, 52, 57 },
    { 17, 24, 32, 40, 47, 53, 58 },
    { 25, 33, 41, 48, 54, 59 },
    { 34, 42, 49, 55, 60 }}},
  { Hexa::Dirs::NW, {
    { 26, 35, 43, 50, 56 },
    { 18, 27, 36, 44, 51, 57 },
    { 11, 19, 28, 37, 45, 52, 58 },
    { 5, 12, 20, 29, 38, 46, 53, 59 },
    { 0, 6, 13, 21, 30, 39, 47, 54, 60 },
    { 1, 7, 14, 22, 31, 40, 48, 55 },
    { 2, 8, 15, 23, 32, 41, 49 },
    { 3, 9, 16, 24, 33, 42 },
    { 4, 10, 17, 25, 34 }}}
};
class HexaArrows {
  class HexaConstraint {
    public:
      using Variables = ViewArray<Int::BoolView>;
      static constexpr PropCond propagatorCondition = Int::PC_BOOL_VAL;
      static ExecStatus propagate( MyScript<HexaArrows>& home, const ModEventDelta& med ) {
        const HexaArrows::ProperVariables& status = home.getVariables();
        std::array<int, 61> numbers{};
        for ( int i = 0; i < 24; ++i ) {
          if ( status[ i ].assigned() ) {
            auto [ dir, dirIndex ] = boolToDir[{ i, status[i].val() }];
            for ( int fieldIndex : fieldIds[ dir ][ dirIndex ] ) {
              numbers[ fieldIndex ]++;
            }
          }
        }
        for ( int i = 0; i < 61; ++i ) {
          if ( numbers[ i ] > givens[ i ] ) {
            return ES_FAILED;
          }
        }
        return ES_FIX;
      }
  };

  class HexaBrancher {
    private:
      class HexaChoice : public Choice {
        public:
          int index_;
          HexaChoice( const MyBrancher<HexaBrancher, HexaArrows>& brancher, int index ) 
            : Choice{ brancher, 2 }, index_{ index } {}
          void archive( Archive& archive ) const {}
          int getIndex() const { return index_; }
      };
    public:
      using ChoiceT = HexaChoice;
      static bool status( const MyScript<HexaArrows>& home ) {
        return ! home.getVariables().assigned();
      }

      static const Choice* choice( MyBrancher<HexaBrancher, HexaArrows>& brancher, 
          MyScript<HexaArrows>& home ) {
        for ( int i = 0; i < 24; ++i ) {
          if ( ! home.getVariables()[ i ].assigned() ) {
            return new HexaChoice( brancher, i );
          }
        }
        GECODE_NEVER;
        return nullptr;
      }

      static ExecStatus commit( MyScript<HexaArrows>& home, const HexaChoice& choice, uint a ) {
        auto& variables = home.getVariables();
        Int::BoolView view{ variables[ choice.getIndex() ] };
        return me_failed( view.eq( home, (int) a ) ) ? ES_FAILED : ES_OK;
      }
  };
  public:
    using ProperVariables = BoolVarArray;
    static constexpr char description[] = "HexaArrows (K&H, Logika 2022, 2. kolo, 11. uloha)";
    static void initVariables( MyScript<HexaArrows>& model ) {
      model.getVariables() = BoolVarArray( model, 24, 0, 1 );
    }
    static void addConstraints( MyScript<HexaArrows>& model ) {
      postConstraint<HexaConstraint>( model );
    }
    static void addBranching( MyScript<HexaArrows>& model ) {
      postBrancher<HexaBrancher>( model );
    }
};

int
main(int argc, char* argv[]) {
  mainRun<HexaArrows>( argc, argv );
}
