#pragma once
#include <numeric>

//propagatorRoot ma spolupracovat s jedinym helperem a ma bejt 
//  subscribovan u vsech poli, do nichz by z nej tykadlo mohlo 
//  zasahnout, tedy na kazdou stranu helper.getMax() ihned po 
//  inicializaci (unsubscribe ted neresme)
//  zavolan dela uvahy dvou druhu: 
//    a) soucet maxu ve trech smerech je mensi nez total, pak 
//        ve ctvrtem smeru musi byt min
//    b) soucet minu ve trech smerech plus max ve ctvrtem smeru
//        je vetsi nez total, pak v tom ctvrtem smeru musi byt
//        mensi max

template<int PC>
class PropagatorRoot : public Gecode::NaryPropagator<Gecode::Int::IntView, PC> {
  private:
    TykadlaHelper helper;
  public:
    PropagatorRoot( Gecode::Home home, TykadlaHelper helper_, 
        Gecode::ViewArray<Gecode::Int::IntView> fields ) :
      Gecode::NaryPropagator<Gecode::Int::IntView, PC>( home, fields ), 
      helper{ helper_ } {}

    PropagatorRoot( Gecode::Home home, PropagatorRoot& orig ) :
      Gecode::NaryPropagator<Gecode::Int::IntView, PC>( home, orig ), 
      helper{ home, orig.helper } {}

    Gecode::Actor* copy( Gecode::Space& home ) override {
      return new (home) PropagatorRoot( home, *this );
    }

    Gecode::ExecStatus propagate( Gecode::Space& home, const Gecode::ModEventDelta& med ) override {
      //std::cout << "propagate " << helper.getIndex() << "\n";
      if ( helper.updateWithGrid( home ) == Gecode::ES_FAILED ) {
        return Gecode::ES_FAILED;
      }
      std::size_t total = helper.getTotal();
      std::map<Dir, std::pair<std::size_t, std::size_t>> dirsMinMax = helper.getDirsMinMax();

      //a) soucet maxu ve trech smerech je mensi nez total, pak ve ctvrtem smeru musi byt min
      std::size_t sumMax = std::accumulate( dirsMinMax.begin(), dirsMinMax.end(), std::size_t{ 0 },
          []( std::size_t a, auto b ) { return a + b.second.second; } );
      for ( Dir dir : dirs ) {
        std::size_t sumMaxWithoutDir = sumMax - dirsMinMax[ dir ].second;
        //std::cout << sumMax << ", " << sumMaxWithoutDir << ", " << total << '\n';
        if ( sumMaxWithoutDir < total ) {
          if ( helper.updateMin( home, dir, total - sumMaxWithoutDir ) == Gecode::ES_FAILED ) {
            return Gecode::ES_FAILED;
          }
        }
      }

      //b) soucet minu ve trech smerech plus max ve ctvrtem smeru je vetsi nez total, pak 
      //  v tom ctvrtem smeru musi byt mensi max
      std::size_t sumMin = std::accumulate( dirsMinMax.begin(), dirsMinMax.end(), std::size_t{ 0 },
          []( std::size_t a, auto b ) { return a + b.second.first; } );
      for ( Dir dir : dirs ) {
        std::size_t sumMinWithoutDir = sumMin - dirsMinMax[ dir ].first;
        std::size_t sumMinPlusMaxInDir = sumMinWithoutDir + dirsMinMax[ dir ].second;
        if ( sumMinPlusMaxInDir > total ) {
          if ( helper.updateMax( home, dir, total - sumMinWithoutDir ) == Gecode::ES_FAILED ) {
            return Gecode::ES_FAILED;
          }
        }
      }

      //c) pokud jsou v kazdem smeru min a max totozne, pak soucet techto hodnot musi byt roven
      //  totalu; pokud zaroven zadne jine pole neobsahuje index, jsem SUBSUMED, jinak FAILED;
      dirsMinMax = helper.getDirsMinMax();
      for ( Dir dir : dirs ) {
        if ( dirsMinMax[ dir ].first != dirsMinMax[ dir ].second ) {
          return Gecode::ES_NOFIX;
        }
      }
      sumMax = std::accumulate( dirsMinMax.begin(), dirsMinMax.end(), std::size_t{ 0 },
          []( std::size_t a, auto b ) { return a + b.second.second; } );
      if ( sumMax == total ) {
        for ( Dir dir : dirs ) {
          if ( helper.updateMax( home, dir, dirsMinMax[ dir ].second ) == Gecode::ES_FAILED ) {
            return Gecode::ES_FAILED;
          }
        }
        std::cout << "Subsumed index: " << helper.getIndex() << ", total: " << helper.getTotal() << '\n';
        return home.ES_SUBSUMED( *this );
      } else {
        return Gecode::ES_FAILED;
      }
    }

    static Gecode::ExecStatus post( Gecode::Home home, TykadlaHelper helper, 
        const Gecode::IntVarArgs& grid ) {
      //Gecode::ViewArray<Gecode::Int::IntView> fields = helper.getFields();
      //(void) new (home) PropagatorRoot( home, helper, fields );

      Gecode::ViewArray<Gecode::Int::IntView> gridView{ home, grid };
      (void) new (home) PropagatorRoot( home, helper, gridView );
      return Gecode::ES_OK;
    }
};
