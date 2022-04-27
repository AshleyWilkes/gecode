#pragma once

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

//this is first try; I don't want to do anything but check for the condition
//being fulfilled when the grid is completely filled
//hence the const Matrix, though who knows whether this is possible and can work
void interconnected( Gecode::Home home, 
    const Gecode::IntVarArgs& grid, int width, int value );
