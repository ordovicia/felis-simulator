#include "simulator.hpp"
#include "util.hpp"

Simulator::PreState Simulator::lwo(Instruction inst)
{
    auto op = decodeR(inst);

    auto pre_state = makePreGPRegState(op.rd);

    auto addr = (m_reg.at(op.rs) + m_reg.at(op.rt)) / 4;
    checkMemoryIndex(addr);

    m_reg.at(op.rd) = m_memory[addr];
    m_pc += 4;

    return pre_state;
}
