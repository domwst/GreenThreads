#include "green-thread.h"

#include <cassert>
#include <utility>
#include <sys/mman.h>
#include <sys/resource.h>

namespace {
    GreenThread::Thread *mainThread, *curThread;

    struct GreenThreadInit {
        GreenThreadInit() {
            mainThread = new GreenThread::Thread;
            curThread = mainThread;
            mainThread->joinedBy = nullptr;
            mainThread->next = mainThread->prev = mainThread;
            mainThread->finished = mainThread->waiting = false;
        }

        ~GreenThreadInit() {
            assert(mainThread->next == mainThread);
            assert(mainThread->prev == mainThread);
            delete mainThread;
        }
    } __GreenThreadInitObject;

    extern "C" void GreenThreadSwitch(GreenThread::Thread* cur, GreenThread::Thread* nxt);

    void InsertThreadAfter(GreenThread::Thread* ptr, GreenThread::Thread* toInsert) {
        toInsert->next = ptr->next;
        toInsert->prev = ptr;

        toInsert->next->prev = toInsert;
        ptr->next = toInsert;
    }

    void RemoveThread(GreenThread::Thread* toErase) {
        toErase->next->prev = toErase->prev;
        toErase->prev->next = toErase->next;
        toErase->next = toErase->prev = toErase;
    }

    extern "C" void GreenThreadFirstSwitch(GreenThread::Thread* cur, GreenThread::Thread* newTh);

    extern "C" void GreenThreadExecute(GreenThread::Thread* toExecute) {
        curThread = toExecute;
        toExecute->func();
        assert(curThread == toExecute);
        curThread->finished = true;
        if (curThread->joinedBy != nullptr) {
            assert(curThread->joinedBy->waiting);
            curThread->joinedBy->waiting = false;
        }
        GreenThread::Yield();
    }
}

void GreenThread::Yield() {
    GreenThread::Thread* cur = curThread;
    for (GreenThread::Thread* nxt = cur->next; nxt != cur; nxt = nxt->next) {
        if (!nxt->finished && !nxt->waiting) {
            GreenThreadSwitch(cur, nxt);
            curThread = cur;
            break;
        }
        assert(cur != nxt);
    }
}

void GreenThread::Join(GreenThread::Thread* other) {
    assert(other->joinedBy == nullptr);
    assert(other != mainThread);
    if (!other->finished) {
        other->joinedBy = curThread;
        curThread->waiting = true;
        GreenThread::Yield();
        assert(!curThread->waiting);
    }
    RemoveThread(other);
    munmap(other->botOfStack, (char*)other->topOfStack - (char*)other->botOfStack);
    delete other;
}

GreenThread::Thread* GreenThread::__InternalCreate(std::function<void()>&& func) {
    GreenThread::Thread* newThread = new GreenThread::Thread;
    newThread->joinedBy = nullptr;
    newThread->finished = newThread->waiting = false;
    newThread->func = std::move(func);

    size_t stackLen;
    {
        rlimit lim;
        getrlimit(RLIMIT_STACK, &lim);
        stackLen = lim.rlim_cur;
    }
    newThread->botOfStack = mmap(nullptr, stackLen, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(newThread->botOfStack != MAP_FAILED);
    newThread->topOfStack = (char*)newThread->botOfStack + stackLen;

    GreenThread::Thread* cur = curThread;
    InsertThreadAfter(cur, newThread);
    GreenThreadFirstSwitch(cur, newThread);
    curThread = cur;
    return newThread;
}
