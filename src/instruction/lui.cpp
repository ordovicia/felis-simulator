#include "simulator.hpp"

Simulator::State Simulator::lui(Instruction inst, StateIter state_iter)
{
    auto now_state = *state_iter;
    auto new_state = now_state;

    auto op = decodeI(inst);

    new_state.pc += 4;
    new_state.reg.at(op.rt) = op.immediate << 16;

    return new_state;
}
