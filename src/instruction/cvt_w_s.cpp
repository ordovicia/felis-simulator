#include "simulator.hpp"

Simulator::State Simulator::cvt_w_s(Instruction inst)
{
    auto new_state = *m_state_iter;

    auto op = decodeR(inst);

    new_state.pc += 4;
    new_state.reg.at(op.rd)
        = static_cast<uint32_t>(m_state_iter->freg.at(op.rs));

    return new_state;
}