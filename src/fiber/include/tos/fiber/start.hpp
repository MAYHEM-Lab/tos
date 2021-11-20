#pragma once

#include "tos/stack_storage.hpp"
#include <cstring>
#include <tos/compiler.hpp>
#include <tos/fiber/basic_fiber.hpp>

namespace tos::fiber {
namespace detail {
[[noreturn]] inline void fiber_start(void* fib_ptr) {
    auto cur_fib = static_cast<basic_fiber*>(fib_ptr);
    cur_fib->start();
    TOS_UNREACHABLE();
}

template<class FibT, class... Args>
auto setup_stack(span<uint8_t> stack, Args&&... args) {
    auto res = new (stack.end() - sizeof(FibT)) FibT{std::forward<Args>(args)..., stack};
    stack = stack.slice(0, stack.size() - sizeof(FibT));

    auto ctx_ptr = new (stack.end() - sizeof(processor_context)) processor_context;
    stack = stack.slice(0, stack.size() - sizeof(processor_context));
    res->set_processor_state(*ctx_ptr);

    start(*ctx_ptr, &fiber_start, res, stack.end());

    return res;
}
} // namespace detail

struct non_owning {
    template<class StartFn>
    struct fib : basic_fiber {
        NO_INLINE [[noreturn]] void start() {
            m_fn(*this);
            suspend();
            TOS_UNREACHABLE();
        }

        fib(StartFn&& fn, span<uint8_t> stack)
            : m_fn{std::forward<StartFn>(fn)}
            , m_full_stack(stack) {
        }

        StartFn m_fn;
        span<uint8_t> m_full_stack;
    };

    template<class StartFn>
    static auto start(span<uint8_t> stack, StartFn&& fn) {
        return detail::setup_stack<fib<StartFn>>(stack, std::forward<StartFn>(fn));
    }
};

struct owning {
    template<class StartFn>
    struct fib : basic_fiber {
        NO_INLINE [[noreturn]] void start() {
            m_fn(*this);
            suspend();
            TOS_UNREACHABLE();
        }

        fib(StartFn&& fn, span<uint8_t> stack)
            : m_fn{std::forward<StartFn>(fn)}
            , m_full_stack(stack) {
        }

        ~fib() {
            delete[] m_full_stack.data();
        }

        StartFn m_fn;
        span<uint8_t> m_full_stack;
    };

    template<class StartFn>
    static fib<StartFn>* start(stack_size_t stack_sz, StartFn&& fn) {
        auto stack = new (std::nothrow) uint8_t[stack_sz.sz];
        if (!stack) {
            return nullptr;
        }
        return detail::setup_stack<fib<StartFn>>(span(stack, stack_sz.sz),
                                                 std::forward<StartFn>(fn));
    }
};
} // namespace tos::fiber