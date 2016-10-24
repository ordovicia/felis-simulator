#include "simulator.hpp"
#include <cmath>

Simulator::State Simulator::abs_s(Instruction inst)
{
    auto new_state = *m_state_iter;

    auto op = decodeR(inst);

    new_state.pc += 4;
    new_state.freg.at(op.rd) = std::abs(m_state_iter->freg.at(op.rs));

    return new_state;
}