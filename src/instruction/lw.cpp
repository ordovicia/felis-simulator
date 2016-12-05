#include "simulator.hpp"
#include "util.hpp"

Simulator::State Simulator::lw(Instruction inst)
{
    auto new_state = *m_state_iter;
    new_state.memory_patch = MemoryPatch{};

    auto op = decodeI(inst);
    auto addr = (m_state_iter->reg.at(op.rs)
                    + static_cast<int32_t>(signExt(op.immediate, 16))) / 4;

    new_state.pc += 4;
    try {
        new_state.reg.at(op.rt) = m_memory.at(addr);
    } catch (std::out_of_range e) {
        FAIL("# Memory index out of range\n" << e.what());
    }

    return new_state;
}
