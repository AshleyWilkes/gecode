#pragma once
#include <set>

//propagatorField spolupracuje s up-to 4 helpery, na kazdou stranu
//  s max jednim; ma bejt subscribovan u vsech poli, ktera lezi
//  mezi nim a nekterym helperem; 
//  dela uvahu jedineho druhu: 
//    pokud ve trech smerech je policko s cislem dal nez jeho maximum, 
//      pak ve ctvrtem smeru musi byt min vzdalenost sem

class PropagatorField : public Gecode::NaryPropagator<Gecode::Int::IntView, Gecode::Int::PC_INT_DOM> {
  private:
    std::map<Dir, std::pair<TykadlaHelper, std::size_t>> helpers;
  public:
    PropagatorField( Gecode::Home home, std::map<Dir, std::pair<TykadlaHelper, std::size_t>> helpers_,
        Gecode::ViewArray<Gecode::Int::IntView> fields ) :
      Gecode::NaryPropagator<Gecode::Int::IntView, Gecode::Int::PC_INT_DOM>( home, fields ), 
      helpers{ helpers_ } {}

    PropagatorField( Gecode::Home home, PropagatorField& orig ) :
      Gecode::NaryPropagator<Gecode::Int::IntView, Gecode::Int::PC_INT_DOM>( home, orig ), 
      helpers{ orig.helpers } {
        for ( Dir dir : dirs ) {
          helpers[ dir ].first.update( home, orig.helpers[ dir ].first );
        }
    }

    Gecode::Actor* copy( Gecode::Space& home ) override {
      return new (home) PropagatorField( home, *this );
    }

    Gecode::ExecStatus propagate( Gecode::Space& home, const Gecode::ModEventDelta& med ) override {
      std::set<Dir> visibles;
      for ( Dir dir : dirs ) {
        if ( helpers.contains( dir ) && 
            helpers[ dir ].first.getDirsMinMax()[ -dir ].second >= helpers[ dir ].second ) {
          visibles.insert( dir );
        }
      }
      if ( visibles.empty() ) {
        return Gecode::ES_FAILED;
      }
      if ( visibles.size() == 1 ) {
        Dir dir = *( visibles.begin() );
        return helpers[ dir ].first.updateMin( home, -dir, helpers[ dir ].second );
      }
      return Gecode::ES_FIX;
    }

    static Gecode::ExecStatus post( Gecode::Home home, 
        std::map<Dir, std::pair<TykadlaHelper, std::size_t>> helpers ) {
      Gecode::IntVarArgs temp;
      for ( Dir dir : dirs ) {
        Gecode::ViewArray<Gecode::Int::IntView> dirFields = helpers[ dir ].first.getFields( -dir );
        for ( auto dirField : dirFields ) {
          temp << dirField;
        }
      }
      Gecode::ViewArray<Gecode::Int::IntView> fields = { home, temp };
      (void) new (home) PropagatorField( home, helpers, fields );
      return Gecode::ES_OK;
    }
};
