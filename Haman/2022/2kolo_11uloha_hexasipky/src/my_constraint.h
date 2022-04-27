#pragma once

template<typename ConstraintT, typename PuzzleT>
class MyConstraint : public Propagator {
  private:
    ConstraintT::Variables variables_;
  public:
    MyConstraint( Home home, ConstraintT::Variables& variables ) : 
      Propagator( home ), variables_{ variables } {
        variables_.subscribe( home, *this, ConstraintT::propagatorCondition );
    }

    MyConstraint( Home home, MyConstraint& orig ) : Propagator( home, orig ) {
      variables_.update( home, orig.variables_ );
    }

    Actor* copy( Space& home ) override {
      return new (home) MyConstraint( home, *this );
    }

    void reschedule( Space& home ) override {
      variables_.reschedule( home, *this, ConstraintT::propagatorCondition );
    }

    ExecStatus propagate( Space& home, const ModEventDelta& med ) override {
      return ConstraintT::propagate( static_cast<MyScript<PuzzleT>&>( home ), med );
    }

    PropCost cost( const Space& home, const ModEventDelta& med ) const override {
      //TODO:: konstanty cist z ConstraintT
      return PropCost::crazy(PropCost::HI, 81);
    }
    
    std::size_t dispose( Space& home ) override {
      variables_.cancel( home, *this, ConstraintT::propagatorCondition );
      (void) Propagator::dispose( home );
      return sizeof(*this);
    }

    static ExecStatus post( Home home, ConstraintT::Variables& variables ) {
      (void) new (home) MyConstraint( home, variables );
      return ES_OK;
    }
};

template<typename ConstraintT, typename ModelT>
void postConstraint( MyScript<ModelT>& home ) {
  BoolVarArgs vars = home.getVariables();
  ViewArray<Int::BoolView> gridView{ home, vars };
  using Constraint = MyConstraint<ConstraintT, ModelT>;
  GECODE_ES_FAIL(Constraint::post(home, gridView));
}
