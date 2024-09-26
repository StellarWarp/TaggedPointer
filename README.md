# Tagged Pointer

a lightweight tagged pointer library for C++20

provide a tuple-like tagged pointer


```c++
#include "../src/tagged_ptr.h"
#include "ut.hpp"

using namespace boost::ut;

suite sample = [] {

    "sample1"_test = [] {
        int a = 0;
        //low bits: 2 utilize int alignment of 4
        tp::tagged_ptr<int, bool, bool> p = &a;

        //bool,bool
        const auto [b1,b2] = p;

        //bool ref, bool ref
        auto& [b1_ref,b2_ref] = p;
        b1_ref = true;
        b2_ref = true;
    };

    "sample2"_test = [] {

        enum class Flag {
            None = 0,
            A = 1 << 0,
            B = 1 << 1
        };

        int a = 0;
        tp::tagged_ptr<int, tp::tag_of<Flag,2>> p = &a;

        auto& [b1] = p;
        b1 = Flag::A;

    };

    "sample3"_test = [] {

        int a = 0;
        // low bits: 2
        // platform dependent: x86_64 high bits reserve: 16
        tp::tagged_ptr<int, bool, bool, uint16_t> p = &a;

        const auto [b1,b2,i] = p;

    };

    "sample4"_test = [] {
        // platform dependent: x86_64 high bits reserve: 16
        tp::tagged_ptr<void, uint16_t> p;

        const auto [i] = p;

    };


};
```

