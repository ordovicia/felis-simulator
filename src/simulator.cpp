#include <cmath>
#include <iostream>
#include <sstream>
#include <ncurses.h>
#include "util.hpp"
#include "simulator.hpp"

Simulator::Simulator(const std::string& binfile)
{
    m_binfile.open(binfile, std::ios::binary);
    if (m_binfile.fail()) {
        throw std::runtime_error{"File " + binfile + " couldn't be opened"};
    }

    initInstruction();

    m_state_hist.emplace_front();
    m_state_iter = m_state_hist.begin();

    m_codes.reserve(CODE_INITIAL_SIZE);
    while (not m_binfile.eof()) {
        Instruction r;
        m_binfile.read(reinterpret_cast<char*>(&r), sizeof r);
        m_codes.emplace_back(r);
    }
}

void Simulator::run()
{
    bool run = false;

    while (true) {
        erase();
        printState(m_state_iter);
        printCode(m_state_iter);

        // Input command
        if (m_halt or not run) {
            addstr(">> ");
            refresh();

            char input[16];
            getnstr(input, 16);

            if (streq(input, "run") or streq(input, "r")) {
                if (run or m_halt) {
                    m_state_iter = m_state_hist.begin();
                    m_halt = false;
                    run = false;
                    continue;
                }
                run = true;
            } else if (streqn(input, "break", 5)) {
                sscanf(input + 5, "%d", &m_breakpoint);
                continue;
            } else if (streqn(input, "b", 1)) {
                sscanf(input + 1, "%d", &m_breakpoint);
                continue;
            } else if (streq(input, "step") or streq(input, "s")) {
                // break;
            } else if (streq(input, "prev") or streq(input, "p")) {
                if (m_halt)
                    if (m_state_iter != m_state_hist.begin())
                        m_state_iter--;
                if (m_state_iter != m_state_hist.begin())
                    m_state_iter--;

                run = false;
                m_halt = false;
                continue;
            } else if (streq(input, "quit") or streq(input, "q")) {
                return;
            } else if (streq(input, "help") or streq(input, "h")) {
                printHelp();
                getch();
                continue;
            } else {
                continue;
            }
        }

        if (not m_halt) {
            try {
                Instruction inst = m_codes.at(m_state_iter->pc / 4);  // fetch
                auto opcode = decodeOpCode(inst);
                auto new_state = exec(opcode, inst, m_state_iter);
                if (m_state_iter == std::prev(m_state_hist.end())) {
                    m_state_iter
                        = m_state_hist.insert(m_state_hist.end(), new_state);
                } else {
                    m_state_iter++;
                }

                m_dynamic_inst_cnt++;

                if (m_breakpoint > 0
                    and static_cast<uint32_t>(m_breakpoint) == m_state_iter->pc)
                    run = false;
            } catch (std::out_of_range e) {
                FAIL("Program counter out of range\n" << e.what());
            }
        }
    }
}

OpCode Simulator::decodeOpCode(Instruction inst)
{
    return static_cast<OpCode>(bitset(inst, 0, 6));
}

Simulator::State Simulator::exec(
    OpCode opcode, Instruction inst, StateIter state_iter)
{
    try {
        m_inst_cnt.at(opcode)++;
        return (m_inst_funcs.at(opcode))(inst, state_iter);
    } catch (std::out_of_range e) {
        FAIL("Invalid instruction code\n" << e.what());
    }
}

Simulator::OperandR Simulator::decodeR(Instruction inst)
{
    return OperandR{
        bitset(inst, 6, 11),
        bitset(inst, 11, 16),
        bitset(inst, 16, 21),
        bitset(inst, 21, 26),
        bitset(inst, 26, 32)};
}

Simulator::OperandI Simulator::decodeI(Instruction inst)
{
    return OperandI{
        bitset(inst, 6, 11),
        bitset(inst, 11, 16),
        bitset(inst, 16, 32)};
}

Simulator::OperandJ Simulator::decodeJ(Instruction inst)
{
    return OperandJ{
        bitset(inst, 6, 11),
        bitset(inst, 11, 32)};
}

void Simulator::printBitset(uint32_t bits, int begin, int end, bool endl)
{
    for (int b = begin; b < end; b++) {
        uint32_t bit = (bits << b) >> 31;
        printw("%d", bit);
    }

    if (endl)
        addch('\n');

    refresh();
}

void Simulator::printState(StateIter state_iter) const
{
    int cwidth, cheight;
    getmaxyx(stdscr, cheight, cwidth);
    (void)cheight;
    bool col8 = (cwidth > 14 * 8 + 3 * 7);

    if (col8)
        addstr("============== + ============== + "
               "============== + ============== + "
               "============== + ============== + "
               "============== + ==============\n");
    else
        addstr("============== + ============== + "
               "============== + ==============\n");

    {  // Register
        auto reg = state_iter->reg;

        for (size_t i = 0; i < REG_SIZE; i++) {
            printw("r%-2d = %08x", i, reg.at(i));
            if ((col8 and i % 8 == 7) or (not col8 and i % 4 == 3))
                addch('\n');
            else
                addstr(" | ");
        }
    }

    if (col8)
        addstr("-------------- + -------------- + "
               "-------------- + -------------- + "
               "-------------- + -------------- + "
               "-------------- + --------------\n");
    else
        addstr("-------------- + -------------- + "
               "-------------- + --------------\n");

    {  // Floating point register
        auto freg = state_iter->freg;

        union FloatBit {
            float f;
            uint32_t b;
        };

        for (size_t i = 0; i < FREG_SIZE; i++) {
            FloatBit fb{freg.at(i)};
            uint32_t b = fb.b;
            printw("f%-2d = %08x", i, b);
            if ((col8 and i % 8 == 7) or (not col8 and i % 4 == 3))
                addch('\n');
            else
                addstr(" | ");
        }
    }

    if (col8)
        addstr("-------------- + -------------- + "
               "-------------- + -------------- + "
               "-------------- + -------------- + "
               "-------------- + --------------\n");
    else
        addstr("-------------- + -------------- + "
               "-------------- + --------------\n");

    printw("hi  = %08x | lo  = %08x | bp  = %8d |\n",
        state_iter->hi, state_iter->lo, m_breakpoint);

    if (col8)
        addstr("============== + ============== + "
               "============== + ============== + "
               "============== + ============== + "
               "============== + ==============\n");
    else
        addstr("============== + ============== + "
               "============== + ==============\n");

    refresh();
}

void Simulator::printCode(StateIter state) const
{
    int cwidth, cheight;
    getmaxyx(stdscr, cheight, cwidth);
    bool col8 = (cwidth > 14 * 8 + 3 * 7);
    int row_width = (cheight - (col8 ? 8 : 16) - 12) / 2;

    int pc4 = state->pc / 4;
    int min_code_idx
        = std::min(pc4 + row_width, static_cast<int>(m_codes.size()));

    for (int c = pc4 - row_width; c < pc4 + row_width; c++) {
        if (c < 0 or c >= min_code_idx) {
            addstr("      |\n");
            continue;
        }

        if (c == pc4)
            attrset(COLOR_PAIR(0) | A_REVERSE);

        printw("%5d | ", c * 4);
        printBitset(m_codes.at(c), 0, 32, true);

        attrset(COLOR_PAIR(0));
    }

    if (col8)
        addstr("============================================="
               "============================================="
               "===========================================\n");
    else
        addstr("================================="
               "================================\n");

    refresh();
}

void Simulator::printHelp()
{
    addstr("run|r: run to the 'halt' or reset\n"
           "(break|b)[int]: set breakpoint\n"
           "step|s: next instruction, "
           "prev|p: rewind to previous instruction\n"
           "quit|q, help|h\n");
    refresh();
}
