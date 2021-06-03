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
            mainThread->finished = false;
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

    void RemoveThread(GreenThread::Thread* toRemove) {
        toRemove->next->prev = toRemove->prev;
        toRemove->prev->next = toRemove->next;
    }

    extern "C" void GreenThreadFirstSwitch(GreenThread::Thread* cur, GreenThread::Thread* newTh);

    extern "C" void GreenThreadExecute(GreenThread::Thread* toExecute) {
        curThread = toExecute;
        toExecute->func();
        assert(curThread == toExecute);
        curThread->finished = true;
        if (curThread->joinedBy != nullptr) {
            InsertThreadAfter(curThread, curThread->joinedBy);
        }
        assert(curThread->next != curThread);
        RemoveThread(curThread);
        GreenThread::Yield();
    }
}

void GreenThread::Yield() {
    GreenThread::Thread* cur = curThread;
    GreenThreadSwitch(cur, cur->next);
    curThread = cur;
}

void GreenThread::Join(GreenThread::Thread* other) {
    assert(other->joinedBy == nullptr);
    assert(other != mainThread);
    if (!other->finished) {
        assert(curThread->next != curThread);
        other->joinedBy = curThread;
        RemoveThread(curThread);
        GreenThread::Yield();
        assert(curThread->next->prev == curThread);
        assert(curThread->prev->next == curThread);
        assert(other->finished);
    }
    munmap(other->botOfStack, (char*)other->topOfStack - (char*)other->botOfStack);
    delete other;
}

GreenThread::Thread* GreenThread::__InternalCreate(std::function<void()>&& func) {
    GreenThread::Thread* newThread = new GreenThread::Thread;
    newThread->joinedBy = nullptr;
    newThread->finished = false;
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
    InsertThreadAfter(cur->prev, newThread);
    GreenThreadFirstSwitch(cur, newThread);
    curThread = cur;
    return newThread;
}
