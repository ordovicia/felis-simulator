#include "simulator.hpp"
#include "util.hpp"

Simulator::PreState Simulator::lwc1(Instruction inst)
{
    auto op = decodeI(inst);

    auto pre_state = makePreFRegState(op.rt);

    auto addr = (m_reg.at(op.rs)
                    + static_cast<int32_t>(signExt(op.immediate, 16))) / 4;
    if (addr < 0 || addr >= static_cast<int32_t>(MEMORY_NUM))
        FAIL("# Error: Memory index out of range");

    m_freg.at(op.rt) = btof(m_memory.at(addr));
    m_pc += 4;

    return pre_state;
}
