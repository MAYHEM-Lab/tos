//
// Created by Mehmet Fatih BAKIR on 28/03/2018.
//

#include <avr/io.h>
#include <avr/wdt.h>

#include <tos_arch.hpp>
#include <tos/arch.hpp>

#include <avr/power.h>
#include <avr/sleep.h>
#include <tos/compiler.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/delay.hpp>
#include <util/delay.h>

namespace tos
{
    void delay_ms(std::chrono::milliseconds d)
    {
        ::_delay_ms(d.count());
    }

    void delay_us(std::chrono::microseconds d)
    {
        ::_delay_us(d.count());
    }
} // namespace tos


static void power_down(int mode)
{
    set_sleep_mode(mode);
    sleep_enable();
    sleep_bod_disable();
    sleep_cpu();
    sleep_disable();
}

extern void tos_main();

int TOS_EXPORT TOS_MAIN NORETURN main()
{
    tos::kern::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        if (res == tos::exit_reason::restart) {
            tos_force_reset();
        }
        if (res == tos::exit_reason::power_down) {
            power_down(SLEEP_MODE_PWR_DOWN);
        }
        if (res == tos::exit_reason::idle) power_down(SLEEP_MODE_IDLE);
    }

    __builtin_unreachable();
}