# GreenThreads
Simple Implementation of green threads (corutines) -- several functions can run simultaneously in the same thread.

## Usage
Use GreenThread::Create(func, params...) to create new green thread, GreenThread::Join to join another green thread and GreenThread::Yield to pass execution to next thread.
See example.cpp for example

## Compilation
```bash
g++ green-thread.cpp green-thread.S <yourCode> -std=c++17
```

Yes I shuld've done makefile but I spent a lot of time for this implementation already

## Warning
This code will only work on x86_64 architecture, on a system, that follows **System V AMD64 ABI** calling conventions (on Windows probably wouldn't work)
