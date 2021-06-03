#pragma once

#include <functional>

namespace GreenThread {
    struct Thread {
        void* rbx;
        void* rsp;
        void* rbp;
        void* r12;
        void* r13;
        void* r14;
        void* r15;

        void* topOfStack;
        void* botOfStack;

        std::function<void()> func;

        Thread* joinedBy;
        Thread* next;
        Thread* prev;

        bool finished;
    };

    void Yield();

    void Join(Thread* other);

    Thread* __InternalCreate(std::function<void()>&&);

    template<class F, class... Args>
    Thread* Create(F&& func, Args&&... args) {
        return __InternalCreate(std::bind(func, std::forward<Args>(args)...));
    }
}