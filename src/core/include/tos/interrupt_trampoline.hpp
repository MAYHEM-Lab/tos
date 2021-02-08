#pragma once

#include <tos/arch.hpp>
#include <tos/function_ref.hpp>
#include <tos/suspended_launch.hpp>
#include <tos/thread.hpp>
#include <tos/utility.hpp>

namespace tos {
/**
 * This class is used to setup a trampoline that can be used to transition
 * from a thread in interrupt context to another thread in normal mode.
 */
class interrupt_trampoline : public tos::non_copy_movable {
    enum class state
    {
        none,
        first,
        later
    } m_isr_state{};

public:
    /**
     * This function is used as the entry point for the trampoline.
     *
     * Upon entry, it immediately transitions to an interrupt context
     * using a platform dependent function. For instance, it could be an SVC
     * or waiting for a timer interrupt.
     */
    template<class InISR>
    void operator()(InISR& in_isr) {
        kern::disable_interrupts();
        in_isr([this](auto&&...) { on_svc(); });
        // The stack up until this point allows us to escape from an interrupt handler.
        // We'll keep that in m_stack and switch to m_tmp_stack to perform the
        // context switch and block_forever.
        arch::set_stack_ptr(reinterpret_cast<char*>(&m_tmp_stack));
        if (m_isr_state == state::first) {
            // We just saved the context, no need to return anywhere, we'll stay here
            this_thread::block_forever();
        }
        // At this point, we successfully escaped from an interrupt context to thread
        // context, we can safely switch to the requested thread.
        // The thread context that was interrupted will safely stay in a semi-interrupt
        // context.
        global::thread_state.current_thread = m_target;
        kern::disable_interrupts();
        global::should_enable = true;
        kern::switch_context(m_target->get_processor_state(),
                             tos::return_codes::scheduled);
    }

    template<class InISR>
    void setup(InISR& in_isr) {
        auto& t = suspended_launch(m_stack, std::ref(*this), in_isr);
        LOG("About to swap to trampoline setup");
        int_guard ig;
        kern::make_runnable(*self());
        swap_context(*self(), t, ig);
        kern::enable_interrupts();
    }

    void switch_to(tos::kern::tcb& target) {
        m_target = &target;
        swap_context(*self(), m_isr_tcb, int_ctx{});
    }

private:
    void on_svc() {
        if (save_context(m_isr_tcb, m_isr_ctx) == tos::return_codes::saved) {
            m_isr_state = state::first;
            return;
        }
        m_isr_state = state::later;
    }

    /**
     * Objects of this type is used as a thread control block for the
     * ISR context that we record. As the thread is started using the
     * regular tos::launch call, there's no need to implement any startup
     * code in this tcb.
     */
    class trampoline_tcb : public tos::kern::tcb {
    public:
        trampoline_tcb(tos::context& ctx)
            : tcb(ctx) {
        }
    };
    trampoline_tcb m_isr_tcb{tos::current_context()};

    /**
     * This context stores an execution context that's in an interrupt
     * handler.
     */
    tos::kern::processor_state m_isr_ctx;

    // Upon executing the trampoline, we'll return to this thread.
    tos::kern::tcb* m_target;
    // This stack maintains the interrupt return code
    tos::stack_storage<4096> m_stack;
    // This stack is used to execute block_forever and switch_context calls.
    tos::stack_storage<2048> m_tmp_stack;
};

template<class InISR>
auto make_interrupt_trampoline(InISR& isr) {
    auto trampoline = std::make_unique<interrupt_trampoline>();
    trampoline->setup(isr);
    return trampoline;
}
} // namespace tos