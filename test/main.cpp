#include "../src/tagged_ptr.h"
#include "immintrin.h"
#include "ut.hpp"
#include "xmmintrin.h"

int main()
{
    using namespace boost::ut;
    using namespace tp;

    "test1"_test = []
    {
        int a = 0;
        tagged_ptr<int, bool, bool> p = &a;

        *p = 1;
        expect(a == 1);

        auto x1 = p.get<0>();
        auto x2 = p.get<1>();
        expect(!x1);
        expect(!x2);
        x1 = true;
        expect(x1);
        expect(!x2);
        expect(*p == 1);
        x2 = true;
        expect(x1);
        expect(x2);
        expect(*p == 1);
    };

    "test2"_test = []
    {
        int a = 0;
        enum class flags
        {
            a = 0,
            b = 1,
            c = 2,
            d = 3
        };

        tagged_ptr<int, tag_of<flags, 2>> p = &a;

        *p = 1;
        expect(a == 1);

        auto x = p.get<0>();
        expect(x == flags::a);
        x = flags::b;
        expect(x == flags::b);
        expect(*p == 1);
        x = flags::c;
        expect(x == flags::c);
        expect(*p == 1);
        x = flags::d;
        expect(x == flags::d);
    };

    "test high bits"_test = []
    {
        int a = 0;


        tagged_ptr<int, tag_of<int, 2>, uint16_t> p = &a;
        const int num = int(21314234214124);
        *p = num;
        expect(a == num);

        auto x = p.get<1>();
        x = 0xFFFF;
        expect(*p = num);
        expect(x == 0xFFFF);

        const auto [t1c, t2c] = p;

        auto [t1, t2] = p;




    };

    "test align"_test = []
    {
        const int scale = 1 << 15;
        std::array<tagged_ptr<int>, scale> ps;
        for (int i = 0; i < scale; i++)
        {
            ps[i] = new int();
        }
        for (int i = 0; i < scale; i++)
        {
            delete ps[i].get();
        }

        for (int i = 0; i < scale; i++)
        {
            ps[i] = (int*) _aligned_malloc(sizeof(int), alignof(int));
        }
        for (int i = 0; i < scale; i++)
        {
            _aligned_free(ps[i].get());
        }
    };

    "test align 128"_test = []
    {
        const int scale = 1 << 15;
        const int al = alignof(__m128);
        std::array<tagged_ptr<__m128>, scale> ps;
        for (int i = 0; i < scale; i++)
        {
            ps[i] = new __m128();
        }
        for (int i = 0; i < scale; i++)
        {
            delete ps[i].get();
        }

        for (int i = 0; i < scale; i++)
        {
            ps[i] = (__m128*) _aligned_malloc(sizeof(__m128), alignof(__m128));
        }
        for (int i = 0; i < scale; i++)
        {
            _aligned_free(ps[i].get());
        }

        //note that the default align is 16 on msvc
        for (int i = 0; i < scale; i++)
        {
            ps[i] = (__m128*) malloc(sizeof(__m128));
        }
        for (int i = 0; i < scale; i++)
        {
            free(ps[i].get());
        }
    };

    "test align 256"_test = []
    {
        const int scale = 1 << 15;
        const int al = alignof(__m256);
        std::array<tagged_ptr<__m256>, scale> ps;
        for (int i = 0; i < scale; i++)
        {
            ps[i] = new __m256();
        }
        for (int i = 0; i < scale; i++)
        {
            delete ps[i].get();
        }

        for (int i = 0; i < scale; i++)
        {
            ps[i] = (__m256*) _aligned_malloc(sizeof(__m256), alignof(__m256));
        }
        for (int i = 0; i < scale; i++)
        {
            _aligned_free(ps[i].get());
        }
        //this cannot pass the test as the 32 align is larger than default allocate align 16
        //        for (int i = 0; i < scale; i++) {
        //            ps[i] = (__m256*)malloc(sizeof (__m256));
        //        }
        //        for (int i = 0; i < scale; i++) {
        //            free(ps[i].get());
        //        }
    };


    return 0;
}
