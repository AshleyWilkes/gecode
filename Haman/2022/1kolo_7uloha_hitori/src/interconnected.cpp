#include "interconnected.h"

#include <iostream>
#include <queue>

using namespace Gecode;

/*class Interconnected : public Propagator {
  private:
    ViewArray<Int::IntView> gridView;
    int width, value;

    std::vector<int> neighbors( int index ) {
      std::vector<int> result;
      int row = index / 9;
      int col = index % 9;
      if ( row > 0 ) {
        result.push_back( index - 9 );
      }
      if ( row < 8 ) {
        result.push_back( index + 9 );
      }
      if ( col > 0 ) {
        result.push_back( index - 1 );
      }
      if ( col < 8 ) {
        result.push_back( index + 1 );
      }
      return result;
    }

    bool isInterconnected() {
      std::cout << "Zdar jak cyp form propagator!\n";
      std::vector<int> q;
      std::vector<bool> processedStatus( gridView.size(), false );
      //najdi prvni index pole, ktery neni roven value, dej ho do q, vsechny predchozi processni
      int index = 0;
      while ( index < 81 ) {
        if ( gridView[index].val() == value ) {
          processedStatus[index] = true;
          ++index;
        } else {
          q.push_back( index );
          break;
        }
      }
      //while q neni prazdna
      while ( ! q.empty() ) {
        //popni back jako pracovni index
        index = q.back();
        q.pop_back();
        //  pro kazdy ze ctyr smeru over
        for ( int neighIndex : neighbors( index ) ) {
          //kdyz je processed, tak nic
          //kdyz neni processed, tak
          if ( processedStatus[ neighIndex ] == false ) {
            //kdyz hodnota je value, tak dat processed
            if ( gridView[ neighIndex ].val() == value ) {
              processedStatus[ neighIndex ] = true;
            } else {//kdyz hodnota neni value, pushnout do q
              q.push_back( neighIndex );
            }
          }
        }
        processedStatus[ index ] = true;
      }
      index = 0;
      while ( index < 81 ) {
        if ( processedStatus[index++] == false ) {
          return false;
        }
      }
      return true;
    }
  public:
    //patterned
    Interconnected( Home home, const IntVarArgs& grid, int width_, int value_ ) 
      : Propagator( home ), gridView( home, grid ), width( width_ ), value( value_ ) {
        gridView.subscribe( home, *this, Int::PC_INT_VAL );
      }

    //patterned
    Interconnected( Home home, Interconnected& orig ) : 
      Propagator( home, orig ), width( orig.width ), value( orig.value ) {
      gridView.update( home, orig.gridView );
    }

    Actor* copy( Space& home ) override {
      return new (home) Interconnected( home, *this );
    }

    //patterned
    void reschedule( Space& home ) override {
      gridView.reschedule( home, *this, Int::PC_INT_VAL );
    }

    ExecStatus propagate( Space& home, const ModEventDelta& med ) override {
      if ( ! gridView.assigned() ) {
        return ES_FIX;
      }

      if ( isInterconnected() ) {
        return home.ES_SUBSUMED( *this );
      } else {
        return ES_FAILED;
      }
    }

    //patterned
    PropCost cost( const Space& home, const ModEventDelta& med ) const override {
      return PropCost::crazy(PropCost::HI, 81);
    }
    
    //patterned
    std::size_t dispose( Space& home ) override {
      gridView.cancel( home, *this, Int::PC_INT_VAL );
      (void) Propagator::dispose( home );
      return sizeof(*this);
    }

    static ExecStatus post( Home home, const IntVarArgs& grid, int width, int value ) {
      (void) new (home) Interconnected( home, grid, width, value );
      return ES_OK;
    }
};*/

class Interconnected : public NaryPropagator<Int::IntView, Int::PC_INT_VAL> {
  private:
    int width, value;

    std::vector<int> neighbors( int index ) {
      std::vector<int> result;
      int row = index / 9;
      int col = index % 9;
      if ( row > 0 ) {
        result.push_back( index - 9 );
      }
      if ( row < 8 ) {
        result.push_back( index + 9 );
      }
      if ( col > 0 ) {
        result.push_back( index - 1 );
      }
      if ( col < 8 ) {
        result.push_back( index + 1 );
      }
      return result;
    }

    bool isInterconnected() {
      std::cout << "Zdar jak cyp form propagator!\n";
      std::vector<int> q;
      std::vector<bool> processedStatus( x.size(), false );
      //najdi prvni index pole, ktery neni roven value, dej ho do q, vsechny predchozi processni
      int index = std::distance( x.begin(), 
          std::find_if( x.begin(), x.end(), 
            [&value = value]( const auto& v ) { return v.val() != value; } ) );
      q.push_back( index );
      for ( int i = 0; i < index; ++i ) {
        processedStatus[i] = true;
      }
 
      //while q neni prazdna
      while ( ! q.empty() ) {
        //popni back jako pracovni index
        int index = q.back();
        q.pop_back();
        //pro kazdy ze ctyr smeru
        for ( int neighIndex : neighbors( index ) ) {
          //kdyz je processed, tak nic
          //kdyz neni processed, tak
          if ( processedStatus[ neighIndex ] == false ) {
            //kdyz hodnota je value, tak dat processed
            if ( x[ neighIndex ].val() == value ) {
              processedStatus[ neighIndex ] = true;
            } else {//kdyz hodnota neni value, pushnout do q
              q.push_back( neighIndex );
            }
          }
        }
        processedStatus[ index ] = true;
      }

      //vrat bool, zda byla zprocessovana vsechna pole
      return std::find( processedStatus.begin(), processedStatus.end(), false ) == processedStatus.end();
    }
  public:
    Interconnected( Home home, ViewArray<Int::IntView>& gridView, int width_, int value_ ) :
      NaryPropagator<Int::IntView, Int::PC_INT_VAL>( home, gridView ),
      width( width_ ), value( value_ ) {}

    Interconnected( Home home, Interconnected& orig ) :
      NaryPropagator<Int::IntView, Int::PC_INT_VAL>( home, orig ),
      width( orig.width ), value( orig.value ) {}

    Actor* copy( Space& home ) override {
      return new (home) Interconnected( home, *this );
    }

    ExecStatus propagate( Space& home, const ModEventDelta& med ) override {
      if ( ! x.assigned() ) {
        return ES_FIX;
      }

      if ( isInterconnected() ) {
        return home.ES_SUBSUMED( *this );
      } else {
        return ES_FAILED;
      }
    }

    static ExecStatus post( Home home, ViewArray<Int::IntView>& gridView, int width, int value ) {
      (void) new (home) Interconnected( home, gridView, width, value );
      return ES_OK;
    }
};

void interconnected( Home home, const IntVarArgs& grid, int width, int value ) {
  ViewArray<Int::IntView> gridView{ home, grid };
  GECODE_ES_FAIL(Interconnected::post(home, gridView, width, value));
}
