#include <iostream>
#include "green-thread.h"

using namespace std;

void f(int l, int r) {
    for (int i = l; i < r; ++i) {
        cout << i << endl;
        GreenThread::Yield();
    }
}

int main() {
    const int l1 = 0, r1 = 10;
    const int l2 = 20, r2 = 25;
    const int l3 = 80, r3 = 80;
    auto t1 = GreenThread::Create(f, l1, r1);
    auto t2 = GreenThread::Create(f, l2, r2);
    auto t3 = GreenThread::Create(f, l3, r3);
    GreenThread::Join(t1);
    GreenThread::Join(t2);
    GreenThread::Join(t3);
}
