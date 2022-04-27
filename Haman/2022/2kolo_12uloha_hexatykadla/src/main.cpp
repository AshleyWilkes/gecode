#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <map>

using namespace Gecode;

class Rectangle;

class Hexa {
  public:
    enum class Dirs{ NW, NE, E, SE, SW, W };
  private:
    std::map<std::size_t, std::array<int, 6>> neighbors = {
      { 0, { -1, -1, 1, 8, 7, -1 }},//0
      { 1, { -1, -1, 2, 9, 8, 0 }},//1
      { 2, { -1, -1, 3, 10, 9, 1 }},//2
      { 3, { -1, -1, 4, 11, 10, 2 }},//3
      { 4, { -1, -1, 5, 12, 11, 3 }},//4
      { 5, { -1, -1, 6, 13, 12, 4 }},//5
      { 6, { -1, -1, -1, 14, 13, 5 }},//6
      { 7, { -1, 0, 8, 15, -1, -1 }},//7
      { 8, { 0, 1, 9, 16, 15, 7 }},//8
      { 9, { 1, 2, 10, 17, 16, 8 }},//9
      { 10, { 2, 3, 11, 18, 17, 9 }},//10
      { 11, { 3, 4, 12, 19, 18, 10 }},//11
      { 12, { 4, 5, 13, 20, 19, 11 }},//12
      { 13, { 5, 6, 14, 21, 20, 12 }},//13
      { 14, { 6, -1, -1, -1, 21, 13 }},//14
      { 15, { 7, 8, 16, 23, 22, -1 }},//15
      { 16, { 8, 9, 17, 24, 23, 15 }},//16
      { 17, { 9, 10, 18, 25, 24, 16 }},//17
      { 18, { 10, 11, 19, 26, 25, 17 }},//18
      { 19, { 11, 12, 20, 27, 26, 18 }},//19
      { 20, { 12, 13, 21, 28, 27, 19 }},//20
      { 21, { 13, 14, -1, 29, 28, 20 }},//21
      { 22, { -1, 15, 23, 30, -1, -1 }},//22
      { 23, { 15, 16, 24, 31, 30, 22 }},//23
      { 24, { 16, 17, 25, 32, 31, 23 }},//24
      { 25, { 17, 18, 26, 33, 32, 24 }},//25
      { 26, { 18, 19, 27, 34, 33, 25 }},//26
      { 27, { 19, 20, 28, 35, 34, 26 }},//27
      { 28, { 20, 21, 29, 36, 35, 27 }},//28
      { 29, { 21, -1, -1, -1, 36, 28 }},//29
      { 30, { 22, 23, 31, 38, 37, -1 }},//30
      { 31, { 23, 24, 32, 39, 38, 30 }},//31
      { 32, { 24, 25, 33, 40, 39, 31 }},//32
      { 33, { 25, 26, 34, 41, 40, 32 }},//33
      { 34, { 26, 27, 35, 42, 41, 33 }},//34
      { 35, { 27, 28, 36, 43, 42, 34 }},//35
      { 36, { 28, 29, -1, 44, 43, 35 }},//36
      { 37, { -1, 30, 38, 45, -1, -1 }},//37
      { 38, { 30, 31, 39, 46, 45, 37 }},//38
      { 39, { 31, 32, 40, 47, 46, 38 }},//39
      { 40, { 32, 33, 41, 48, 47, 39 }},//40
      { 41, { 33, 34, 42, 49, 48, 40 }},//41
      { 42, { 34, 35, 43, 50, 49, 41 }},//42
      { 43, { 35, 36, 44, 51, 50, 42 }},//43
      { 44, { 36, -1, -1, -1, 51, 43 }},//44
      { 45, { 37, 38, 46, -1, -1, -1 }},//45
      { 46, { 38, 39, 47, -1, -1, 45 }},//46
      { 47, { 39, 40, 48, -1, -1, 46 }},//47
      { 48, { 40, 41, 49, -1, -1, 47 }},//48
      { 49, { 41, 42, 50, -1, -1, 48 }},//49
      { 50, { 42, 43, 51, -1, -1, 49 }},//50
      { 51, { 43, 44, -1, -1, -1, 50 }}//51
    };
  public:
    Hexa() {}
    Hexa( const std:: array<int, 52>& givensArray ) {}
    std::size_t getFieldsCount() const { return 52; }
    static constexpr std::size_t DirsCount = 6;
    std::size_t getNeighbor( std::size_t fieldIndex, std::size_t dirIndex ) const {
      return neighbors.at( fieldIndex )[ dirIndex ];
    }
    void print( std::ostream& os, const IntVarArray& actualValues ) {
      os << "\t" << actualValues[0] << "\t\t" << actualValues[1] << "\t\t" << actualValues[2] << "\t\t" << actualValues[3] << "\t\t" << actualValues[4] << "\t\t" << actualValues[5] << "\t\t" << actualValues[6] << '\n';
      os << actualValues[7] << "\t\t" << actualValues[8] << "\t\t" << actualValues[9] << "\t\t" << actualValues[10] << "\t\t" << actualValues[11] << "\t\t" << actualValues[12] << "\t\t" << actualValues[13] << "\t\t" << actualValues[14] << '\n';
      os << "\t" << actualValues[15] << "\t\t" << actualValues[16] << "\t\t" << actualValues[17] << "\t\t" << actualValues[18] << "\t\t" << actualValues[19] << "\t\t" << actualValues[20] << "\t\t" << actualValues[21] << '\n';
      os << actualValues[22] << "\t\t" << actualValues[23] << "\t\t" << actualValues[24] << "\t\t" << actualValues[25] << "\t\t" << actualValues[26] << "\t\t" << actualValues[27] << "\t\t" << actualValues[28] << "\t\t" << actualValues[29] << '\n';
      os << "\t" << actualValues[30] << "\t\t" << actualValues[31] << "\t\t" << actualValues[32] << "\t\t" << actualValues[33] << "\t\t" << actualValues[34] << "\t\t" << actualValues[35] << "\t\t" << actualValues[36] << '\n';
      os << actualValues[37] << "\t\t" << actualValues[38] << "\t\t" << actualValues[39] << "\t\t" << actualValues[40] << "\t\t" << actualValues[41] << "\t\t" << actualValues[42] << "\t\t" << actualValues[43] << "\t\t" << actualValues[44] << '\n';
      os << "\t" << actualValues[45] << "\t\t" << actualValues[46] << "\t\t" << actualValues[47] << "\t\t" << actualValues[48] << "\t\t" << actualValues[49] << "\t\t" << actualValues[50] << "\t\t" << actualValues[51] << '\n';
    }
};

template<typename Topology>
class TopologyData;

template<>
class TopologyData<Hexa> {
  private:
    Hexa topology_;
    std::array<int, 52> values;
  public:
    TopologyData() {}
    TopologyData( const Hexa& topology, const std::array<int, 52>& array ) : topology_{ topology }, values{ array } {}
    int getValue( std::size_t fieldIndex ) const {
      return values[ fieldIndex ];
    }
    std::size_t getNeighbor( std::size_t fieldIndex, std::size_t dirIndex ) const { 
      return topology_.getNeighbor( fieldIndex, dirIndex );
    }
};

/**
 * \brief %Example: HexaTykadla
 *
 * Z každého políčka s číslem vyhchází v přímém směru
 * sudý počet tykadel. Součet jejich délek je dán číslem
 * v políčku, příčemž políčko s číslem se do tohoto
 * součtu nepočítá. Každé prázdné políčko obsahuje část
 * právě jednoho tykadla. Tykadla lze vést v každém
 * ze šesti směrů.
 *
 */


std::array<int, 52> fieldIdxs = {
     0,  1,  2,  3,  4,  5,  6,
   7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21,
  22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36,
  37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51
};

std::array<int, 52> givensArray = {
    -1,  8, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,  7, -1,
    -1, -1, -1, -1,  7, -1, -1,
  -1,  9, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1,  4, -1,  3,
    -1,  7, -1, -1, -1, -1, -1
};

#include "../../1kolo_8uloha_tykadla/src/tykadla.h"

int
main(int argc, char* argv[]) {
  SizeOptions opt("HexaTykadla (K&H, Logika 2022, 2. kolo, 12. uloha)");
  opt.parse(argc,argv);
  Script::run<Tykadla<Hexa>,DFS,SizeOptions>(opt);
}
