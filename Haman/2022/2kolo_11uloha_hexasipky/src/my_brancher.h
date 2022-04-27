#pragma once

template<typename BrancherT, typename PuzzleT>
class MyBrancher : public Brancher {
  public:
    MyBrancher( Home home ) : Brancher( home ) {}
    MyBrancher( Home home, MyBrancher& orig ) : Brancher( home, orig ) {}

    Actor* copy( Space& home ) override {
      return new (home) MyBrancher( home, *this );
    }

    bool status( const Space& home ) const override {
      return BrancherT::status( static_cast<const MyScript<PuzzleT>&>( home ) );
    }
    
    const Choice* choice( Space& home ) override {
      return BrancherT::choice( *this, static_cast<MyScript<PuzzleT>&>( home ) );
    }

    const Choice* choice( const Space& home, Archive& archive ) override {
      return nullptr;
    }

    ExecStatus commit( Space& home, const Choice& choice, uint a ) override {
      return BrancherT::commit( static_cast<MyScript<PuzzleT>&>( home ), 
          static_cast<const BrancherT::ChoiceT&>( choice ), a );
    }

    static void post( Home home ) {
      (void) new (home) MyBrancher( home );
    }
};

template<typename BrancherT, typename ModelT>
void postBrancher( MyScript<ModelT>& home ) {
  if ( home.failed() ) return;
  MyBrancher<BrancherT, ModelT>::post(home);
}
