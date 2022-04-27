#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

class Rectangle {
  private:
    std::size_t width_, height_;
  public:
    struct Delta {
      int row, column;
    };
    static constexpr std::array<Delta, 4> dirs{ 
      Delta{ -1, 0 }, Delta{ 0, 1 }, Delta{ 1, 0 }, Delta{ 0, -1 } };

    Rectangle() {}
    template<std::size_t width, std::size_t height>
    Rectangle( const std::array<std::array<int, width>, height>& givensArray ) : 
      width_{ width }, height_{ height } {}
    std::size_t getWidth() const { return width_; }
    std::size_t getHeight() const { return height_; }
    std::size_t getFieldsCount() const { return width_ * height_; }
    static constexpr std::size_t DirsCount = 4;
    std::size_t getNeighbor( std::size_t fieldIndex, std::size_t dirIndex ) const {
      std::size_t row = fieldIndex / width_, column = fieldIndex % width_;
      if ( dirIndex == 0 && row > 0 ) return fieldIndex - width_;
      if ( dirIndex == 1 && column < width_ - 1 ) return fieldIndex + 1;
      if ( dirIndex == 2 && row < height_ - 1 ) return fieldIndex + width_;
      if ( dirIndex == 3 && column > 0 ) return fieldIndex - 1;
      return -1;
    }
};

template<typename Topology>
class TopologyData;

template<>
class TopologyData<Rectangle> {
  private:
    //to remove
    std::size_t width_, height_;
    std::vector<int> values;
    Rectangle topology_;
  public:
    TopologyData() {}
    //nasledujici konstruktor by mel byt omezen tak, aby nesel zkompilovat, pokud Topology
    //  neni Rectangle
    template<std::size_t width, std::size_t height>
    TopologyData( const Rectangle& topology, const std::array<std::array<int, width>, height>& array ) 
      : width_{ width }, height_{ height }, values{}, topology_{ topology } {
        for ( auto i1 : array ) {
          for ( auto i2 : i1 ) {
            values.push_back( i2 );
          }
        }
    }

    int getValue( std::size_t fieldIndex ) const {
      return values[ fieldIndex ];
    }

    //to remove
    std::size_t getWidth() const { return width_; }
    std::size_t getHeight() const { return height_; }
    std::size_t getNeighbor( std::size_t fieldIndex, std::size_t dirIndex ) const { 
      return topology_.getNeighbor( fieldIndex, dirIndex );
    }
};

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

#include "tykadla.h"

int
main(int argc, char* argv[]) {
  SizeOptions opt("Tykadla (K&H, Logika 2022, 1. kolo, 8. uloha)");
  opt.parse(argc,argv);
  Script::run<Tykadla<Rectangle>,DFS,SizeOptions>(opt);
}
