#include "tykadlaHelpers.h"
#include "propagatorField.h"
#include "propagatorRoot.h"

using namespace Gecode;

Dir operator-( const Dir& dir ) { return { -dir.first, -dir.second }; }
std::pair<int, int> operator*( Dir dir, int distance ) { 
  return { dir.first * distance, dir.second * distance };
}
Coords operator+( Coords c1, std::pair<int, int> c2 ) {
  return { c1.row + c2.first > 0 ? c1.row + c2.first : 0, 
    c1.column + c2.second > 0 ? c1.column + c2.second : 0 };
}
bool operator<( const Coords& c1, const Coords& c2 ) {
  if ( c1.row == c2.row ) {
    return c1.column < c2.column;
  } else {
    return c1.row < c2.row;
  }
}

Gecode::ExecStatus postHelperOnly( Gecode::Home home, Coords coords, const TykadlaHelper& helper,
    Gecode::Int::IntView field ) {
  rel( home, field, IRT_EQ, helper.getIndex() );
  return Gecode::ES_OK;
}

Gecode::ExecStatus postHelpersOnly( Gecode::Home home, Coords coords, 
    std::map<Dir, std::pair<TykadlaHelper, std::size_t>>& helpers, Gecode::Int::IntView field ) {
  Gecode::IntArgs indices;
  for ( Dir dir : dirs ) {
    if ( helpers.contains( dir ) ) {
      indices << helpers[ dir ].first.getIndex();
    }
  }
  Gecode::IntSet set{ indices };
  dom( home, field, set );
  return Gecode::ES_OK;
}

//oba typy propagatoru mohou, zda se, byt NaryPropagator, je treba,
//  aby jako parametr konstruktoru dostavaly seznam svych poli
//  k subscribnuti dle popisu vyse. Jinak je v nich zajimava pouze
//  metoda propagate, jinak jsou velmi standardni.

void tykadlaCondition( Gecode::Home home, Coords coords, TykadlaHelpers& helpers, 
    Gecode::Int::IntView field, const Gecode::IntVarArgs& grid ) {
  auto coordsHelper = helpers.getHelper( coords );
  if ( coordsHelper ) {
    //propagatorRoot
    GECODE_ES_FAIL( PropagatorRoot<Gecode::Int::PC_INT_DOM>::post( home, *coordsHelper, grid ) );
    GECODE_ES_FAIL( PropagatorRoot<Gecode::Int::PC_INT_VAL>::post( home, *coordsHelper, grid ) );
    GECODE_ES_FAIL( postHelperOnly( home, coords, *coordsHelper, field ) );
  } else {
    //propagatorField
    auto coordsHelpers = helpers.getFieldHelpers( coords );
    GECODE_ES_FAIL( PropagatorField::post( home, coordsHelpers ) );
    GECODE_ES_FAIL( postHelpersOnly( home, coords, coordsHelpers, field ) );
  }
}
