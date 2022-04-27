#pragma once

using namespace Gecode;

template<typename ModelT>
class MyScript : public Script {
  private:
    ModelT::ProperVariables variables;
  public:
    MyScript(const SizeOptions& opt) : Script( opt ) {
      ModelT::initVariables( *this );
      ModelT::addConstraints( *this );
      ModelT::addBranching( *this );
    }
    MyScript( MyScript& orig ) : Script( orig ) {
      ModelT::initVariables( *this );
      variables.update( *this, orig.variables );
    }

    virtual Space*
    copy(void) {
      return new MyScript(*this);
    }
    ModelT::ProperVariables& getVariables() { return variables; }
    const ModelT::ProperVariables& getVariables() const { return variables; }
    //requires requires (std::ostream& os, ModelT::ProperVariables x) { os << x; }
    void print(std::ostream& os) const override { os << variables << '\n'; }
};

template<typename Puzzle>
int mainRun( int argc, char* argv[] ) {
  SizeOptions opt(Puzzle::description);
  opt.parse(argc,argv);
  Script::run<MyScript<Puzzle>,DFS,SizeOptions>(opt);
  return 0;
}

#include "my_constraint.h"
#include "my_brancher.h"
