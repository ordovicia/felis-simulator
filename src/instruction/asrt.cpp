#include "simulator.hpp"
#include <ncurses.h>

Simulator::State Simulator::asrt(Instruction inst)
{
    auto new_state = *m_state_iter;
    new_state.memory_patch = MemoryPatch{};

    auto op = decodeI(inst);

    if (m_state_iter->reg.at(op.rs) != static_cast<int32_t>(op.immediate)) {
        printw("Assertion failed\n");
        refresh();
        getch();

        endwin();
        std::exit(1);
    }

    new_state.pc += 4;

    return new_state;
}
