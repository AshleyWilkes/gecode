#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: Magic hexagon
 *
 * The task is to place numbers 1-10 into hexagonal grid of size 3-5
 * so that each straight "row" in the grid contains exactly 2 numbers.
 * The sum of 2 numbers in each straight "row" must be distinct from
 * all other sums and must fall into interval 4-18. Sum of the sums
 * are given.
 *
 * \ingroup Example
 *
 */
class MagicHexagon : public Script {
private:
  IntVarArray x, sums;

public:
  MagicHexagon(const SizeOptions& opt)
    : Script(opt),
      x(*this, 19, 0, 10),
      sums(*this, 15, 4, 18) {

    std::vector<IntVarArgs> dirs {
      { x[0], x[1], x[2] },               // 0
      { x[3], x[4], x[5], x[6] },         // 1
      { x[7], x[8], x[9], x[10], x[11] }, // 2
      { x[12], x[13], x[14], x[15] },     // 3
      { x[16], x[17], x[18] },            // 4

      { x[7], x[12], x[16] },             // 5
      { x[3], x[8], x[13], x[17] },       // 6
      { x[0], x[4], x[9], x[14], x[18] }, // 7
      { x[1], x[5], x[10], x[15] },       // 8
      { x[2], x[6], x[11] },              // 9

      { x[0], x[3], x[7] },               //10
      { x[1], x[4], x[8], x[12] },        //11
      { x[2], x[5], x[9], x[13], x[16] }, //12
      { x[6], x[10], x[14], x[17] },      //13
      { x[11], x[15], x[18] }             //14
    };

    //1) v kazdem dirsu jsou 2 nenulova cisla
    for ( const auto& dir : dirs ) {
      count( *this, dir, 0, IRT_EQ, dir.size() - 2 );
    }
    //2) kazde nenulove cislo je jine
    distinct( *this, x, 0 );
    //3) v kazdem dirsu je soucet 4-18
    for ( int i = 0; i < 15; ++i ) {
      linear( *this, dirs[i], IRT_EQ, sums[i] );
    }
    //4) kazdy soucet je jiny
    distinct( *this, sums );
    //5) jsou zadane soucty:
    //    0 - 12
    rel( *this, sums[0], IRT_EQ, 12 );
    //    2 - 9
    rel( *this, sums[2], IRT_EQ, 9 );
    //    4 - 10
    rel( *this, sums[4], IRT_EQ, 10 );
    //    5 - 13
    rel( *this, sums[5], IRT_EQ, 13 );
    //    7 - 6
    rel( *this, sums[7], IRT_EQ, 6 );
    //    9 - 4
    rel( *this, sums[9], IRT_EQ, 4 );
    //    10 - 7
    rel( *this, sums[10], IRT_EQ, 7 );
    //    12 - 15
    rel( *this, sums[12], IRT_EQ, 15 );
    //    14 - 5
    rel( *this, sums[14], IRT_EQ, 5 );

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t\t" << x[0] << "\t\t" << x[1] << "\t\t" << x[2] << '\n';
    os << "\t" << x[3] << "\t\t" << x[4] << "\t\t" << x[5] << "\t\t" << x[6] << '\n';
    os << x[7] << "\t\t" << x[8] << "\t\t" << x[9] << "\t\t" << x[10] << "\t\t" << x[11] << '\n';
    os << "\t" << x[12] << "\t\t" << x[13] << "\t\t" << x[14] << "\t\t" << x[15] << '\n';
    os << "\t\t" << x[16] << "\t\t" << x[17] << "\t\t" << x[18] << '\n';
  }
  /// Constructor for cloning \a s
  MagicHexagon(MagicHexagon& s) : Script(s) {
      x.update(*this, s.x);
      sums.update(*this, s.sums);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new MagicHexagon(*this);
  }

};

int
main(int argc, char* argv[]) {
  SizeOptions opt("Magic hexagon (K&H, Logika 2022, 1. kolo, 2. uloha)");
  opt.parse(argc,argv);
  Script::run<MagicHexagon,DFS,SizeOptions>(opt);
}
