#include "simulator.hpp"

Simulator::State Simulator::sub_s(Instruction inst)
{
    auto new_state = *m_state_iter;

    auto op = decodeR(inst);

    new_state.pc += 4;
    new_state.freg.at(op.rd)
        = m_state_iter->freg.at(op.rs) - m_state_iter->freg.at(op.rt);

    return new_state;
}