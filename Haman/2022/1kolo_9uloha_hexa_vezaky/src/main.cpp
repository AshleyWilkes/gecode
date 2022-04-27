#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Hexa vezaky
 *
 * Do prazdnych policek zapiste po jedne cislice
 * 1 - 7, ktere predstavuji pocet podlazi jednotlivych
 * domu. V kazde rade vsech tri smeru musi byt vsechny
 * cislice ruzne, pricemz cislice 1 a 7 se v obrazci
 * vyskytuji sestkrat a ze zbyvajicich kazda petkrat.
 * Cislice uvedene u obvodu sestiuhelniku udavaji pocet
 * viditelnych domu v danem smeru. Nizsi dum za vyssim
 * videt neni.
 *
 */

class HexaSkyscrapers : public Script {
private:
  IntVarArray x;

  //BoolVarArray visible;
  //IntVarArray max;
  void numVisibles( IntVarArgs fields, std::size_t visibles ) {
    BoolVarArray visible{ *this, fields.size(), 0, 1 };
    IntVarArgs max{ *this, fields.size(), 1, 7 };
    for ( std::size_t i = 0; i < fields.size(); ++i ) {
      Gecode::max( *this, fields.slice( 0, 1, i + 1 ), max[ i ] );
      rel( *this, max[ i ], IRT_EQ, fields[ i ], visible[ i ] );
    }
    linear( *this, visible, IRT_EQ, visibles );
  }

  void numVisibles( std::size_t dirIndex, std::size_t directVisibles, std::size_t backwardVisibles ) {
    std::vector<IntVarArgs> dirs {
      { x[0], x[1], x[2], x[3] },                         // 0
      { x[4], x[5], x[6], x[7], x[8] },                   // 1
      { x[9], x[10], x[11], x[12], x[13], x[14] },        // 2
      { x[15], x[16], x[17], x[18], x[19], x[20], x[21] },// 3
      { x[22], x[23], x[24], x[25], x[26], x[27] },       // 4
      { x[28], x[29], x[30], x[31], x[32] },              // 5
      { x[33], x[34], x[35], x[36] },                     // 6

      { x[15], x[22], x[28], x[33] },                     // 7
      { x[9], x[16], x[23], x[29], x[34] },               // 8
      { x[4], x[10], x[17], x[24], x[30], x[35] },        // 9
      { x[0], x[5], x[11], x[18], x[25], x[31], x[36] },  //10
      { x[1], x[6], x[12], x[19], x[26], x[32] },         //11
      { x[2], x[7], x[13], x[20], x[27] },                //12
      { x[3], x[8], x[14], x[21] },                       //13

      { x[0], x[4], x[9], x[15] },                        //14
      { x[1], x[5], x[10], x[16], x[22] },                //15
      { x[2], x[6], x[11], x[17], x[23], x[28] },         //16
      { x[3], x[7], x[12], x[18], x[24], x[29], x[33] },  //17
      { x[8], x[13], x[19], x[25], x[30], x[34] },        //18
      { x[14], x[20], x[26], x[31], x[35] },              //19
      { x[21], x[27], x[32], x[36] }                      //20
    };


    IntVarArgs direct = dirs[ dirIndex ];
    numVisibles( direct, directVisibles );
    distinct( *this, direct );

    numVisibles( direct.slice( direct.size() - 1, -1 ), backwardVisibles );
  }
public:
  /// Actual model
  HexaSkyscrapers(const SizeOptions& opt)
      : Script(opt),
      x( *this, 37, 1, 7 ) {
      /*x( *this, 4, 1, 4 ),
      visible{ *this, x.size(), 0, 1 },
      max{ *this, x.size(), 1, 4 } {*/
    /*for ( std::size_t i = 0; i < x.size(); ++i ) {
      Gecode::max( *this, x.slice( 0, 1, i + 1 ), max[ i ] );
      rel( *this, max[ i ], IRT_EQ, x[ i ], visible[ i ] );
    }
    linear( *this, visible, IRT_EQ, 3 );*/

    //zadana cisla
    rel( *this, x[ 12 ] == 6 );
    rel( *this, x[ 17 ] == 4 );
    rel( *this, x[ 25 ] == 5 );

    //1, 7 sestkrat, ostatni petkrat
    count( *this, x, 1, IRT_EQ, 6 );
    count( *this, x, 2, IRT_EQ, 5 );
    count( *this, x, 3, IRT_EQ, 5 );
    count( *this, x, 4, IRT_EQ, 5 );
    count( *this, x, 5, IRT_EQ, 5 );
    count( *this, x, 6, IRT_EQ, 5 );
    count( *this, x, 7, IRT_EQ, 6 );

    //pocty viditelnych
    numVisibles( 0, 2, 2 );
    numVisibles( 1, 1, 4 );
    numVisibles( 2, 4, 1 );
    numVisibles( 3, 4, 3 );
    numVisibles( 4, 2, 4 );
    numVisibles( 5, 4, 1 );
    numVisibles( 6, 2, 2 );

    numVisibles( 7, 2, 2 );
    numVisibles( 8, 3, 1 );
    numVisibles( 9, 1, 3 );
    numVisibles( 10, 2, 2 );
    numVisibles( 11, 4, 1 );
    numVisibles( 12, 1, 3 );
    numVisibles( 13, 3, 2 );

    numVisibles( 14, 2, 3 );
    numVisibles( 15, 2, 2 );
    numVisibles( 16, 1, 3 );
    numVisibles( 17, 4, 3 );
    numVisibles( 18, 5, 1 );
    numVisibles( 19, 1, 4 );
    numVisibles( 20, 2, 2 );
    
    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t\t\t" << x[0] << "\t\t" << x[1] << "\t\t" << x[2] << "\t\t" << x[3] << '\n';
    os << "\t\t" << x[4] << "\t\t" << x[5] << "\t\t" << x[6] << "\t\t" << x[7] << "\t\t" << x[8] << '\n';
    os << "\t" << x[9] << "\t\t" << x[10] << "\t\t" << x[11] << "\t\t" << x[12] << "\t\t" << x[13] << "\t\t" << x[14] << '\n';
    os << x[15] << "\t\t" << x[16] << "\t\t" << x[17] << "\t\t" << x[18] << "\t\t" << x[19] << "\t\t" << x[20] << "\t\t" << x[21] << '\n';
    os << "\t" << x[22] << "\t\t" << x[23] << "\t\t" << x[24] << "\t\t" << x[25] << "\t\t" << x[26] << "\t\t" << x[27] << '\n';
    os << "\t\t" << x[28] << "\t\t" << x[29] << "\t\t" << x[30] << "\t\t" << x[31] << "\t\t" << x[32] << '\n';
    os << "\t\t\t" << x[33] << "\t\t" << x[34] << "\t\t" << x[35] << "\t\t" << x[36] << '\n';
  }
  /// Constructor for cloning \a s
  HexaSkyscrapers(HexaSkyscrapers& s) : Script(s) {
    x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new HexaSkyscrapers(*this);
  }

};


int
main(int argc, char* argv[]) {
  SizeOptions opt("Hexa vezaky (K&H, Logika 2022, 1. kolo, 9. uloha)");
  opt.parse(argc,argv);
  Script::run<HexaSkyscrapers,DFS,SizeOptions>(opt);
}
