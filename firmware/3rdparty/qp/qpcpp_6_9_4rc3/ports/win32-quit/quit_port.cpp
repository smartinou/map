/// @file
/// @brief QUIT (QP Unit Internal Test) port to Win32 with GNU or VisualC++
/// @cond
///***************************************************************************
/// Last updated for version 6.9.2
/// Last updated on  2021-02-12
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond
///
#include "quit.hpp"

#include <stdio.h>  // for printf_s_s()
#include <stdlib.h> // for exit()

#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Win32 API

namespace QP {

//Q_DEFINE_THIS_MODULE("quit_port")

// Global objects ============================================================
std::uint8_t volatile QF_intNest;

//............................................................................
static int l_test_count;

//............................................................................
void QUIT_fail_(char const *cond, char const *file, int line) {
    printf_s(" FAILED in %s:%d\n"
        "%s\n"
        "---------------------------------------------\n"
        "%d test(s)\n"
        "FAILED\n",
        file, line, cond, l_test_count);

#ifndef NDEBUG
    DebugBreak();
#endif

    exit(l_test_count);
}
//............................................................................
void TEST(char const *title) {
    if (l_test_count > 0) {
        printf_s(" PASSED\n");
    }
    printf_s("test: \"%s\" ...", title);
    ++l_test_count;
}

#ifdef Q_SPY
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
void QS::onReset(void) {
}
//............................................................................
void QS::onFlush(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {
    return (QSTimeCtr)0U;
}
//............................................................................
void QS::onCommand(uint8_t cmdId, uint32_t param1,
    uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif // Q_SPY

} // namespace QP

//............................................................................
int main(void) {
    printf_s("\n%s\n", "QP Unit Internal Testing -- QUIT");

    QP::onRunTests();

    if (QP::l_test_count > 0) {
        printf_s("%s\n", " PASSED");
    }
    printf_s("---------------------------------------------\n"
        "%d test(s)\nOK\n", QP::l_test_count);

    return 0; // success
}

/*==========================================================================*/
extern "C" Q_NORETURN Q_onAssert(char_t const * const module,
                                 int_t const location)
{
    QP::QUIT_fail_("Q_onAssert", module, location);
    for (;;) { // explicitly make it "noreturn
    }
}

/*==========================================================================*/

