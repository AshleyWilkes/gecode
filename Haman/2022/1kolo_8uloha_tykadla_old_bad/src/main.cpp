#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include "tykadlaHelpers.h"

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
  3, -1, -1,
  -1, 1, -1
};

std::size_t givenW = 10, givenH = 12, numGivens = 15;
//std::size_t givenW = 3, givenH = 2, numGivens = 2;
std::vector<given> givens = {
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
};

//jak na to:
//chci mit dva druhy propagatoru:
//1) je centrovan na policko s cislem, resi, zda se pro nektery (typicky nejdelsi)
//  smer da urcit, ze nejake tykadlo alespon urcite delky v nem musi lezet,
//  a/nebo, ze nejake tykadlo nemuze byt delsi nez urcita delka a tudiz 
//  do nekterych policek nemuze zasahnout.
//2) je centrovan na policko bez cisla, resi, zda se k nemu hypotechnicky
//  daji poslat tykadla z vice nez jednoho smeru, jinak receno, zda vidi
//  alespon 2 cisla, jez nejsou mensi nez jejich vzdalenost od nej
//
//neboli potrebuju pro kazde policko s cislem mit instanci nejake classy; 
//  ta jedina by mela menit hodnoty v gridu, konkretne sdelovat urcitym 
//  polickum, ze do nich zasahuje tykadlo z tohoto policka s cislem.
//  Propagatory obou typu posilaji sve dedukce gridu pomoci interface 
//  teto classy. Tuto classu potrebuju k tomu, aby si propagator 2. typu
//  umel vsimnout, ze v nekterem policku s cislem doslo v jinem smeru
//  ke zmene, ktera znamena, ze uz se z toho policka s cislem k tomuto
//  policku neda dojit. A taky se hodi k tomu, aby udrzovala info 
//  o tom, jak dlouhe tykadlo uz v kazdem smeru vede a na jakou delku
//  se muze nejvys prodlouzit. Tahle trida tedy jedina zpusobuje zmeny 
//  v gridu, ty zpusobi volani propagatoru a ty in turn zpusobi volani
//  metod teto tridy atd. ad infinitum, ad nauseam.

class Tykadla : public Script {
private:
  IntVarArray actualValues;
  TykadlaHelpers helpers;

public:
  /// Actual model
  Tykadla(const SizeOptions& opt)
    : Script(opt),
      actualValues(*this, givenW * givenH, 0, numGivens - 1),
      helpers(*this, actualValues, givensArray) {

    Matrix<IntVarArray> actualValuesM( actualValues, givenW, givenH );

    for ( std::size_t row = 0; row < givenH; ++row ) {
      for ( std::size_t col = 0; col < givenW; ++col ) {
        tykadlaCondition( *this, { row, col }, helpers, actualValuesM( col, row ), actualValues );
      }
    }

    for ( int i = 0; i < givens.size(); ++i ) {
      count( *this, actualValues, i, IRT_EQ, givens[ i ].length + 1 );
    }

    //branch(*this, actualValues, INT_VAR_NONE(), INT_VAL_MIN());
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
    helpers.update(*this, s.helpers);
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
