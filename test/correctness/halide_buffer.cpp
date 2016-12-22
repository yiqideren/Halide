// Don't include Halide.h: it is not necessary for this test.
#include "HalideBuffer.h"

using namespace Halide::Runtime;

template<typename T1, typename T2>
void check_equal_shape(const Buffer<T1> &a, const Buffer<T2> &b) {
    if (a.dimensions() != b.dimensions()) abort();
    for (int i = 0; i < a.dimensions(); i++) {
        if (a.dim(i).min() != b.dim(i).min() ||
            a.dim(i).extent() != b.dim(i).extent()) {
            abort();
        }
    }
}

template<typename T1, typename T2>
void check_equal(const Buffer<T1> &a, const Buffer<T2> &b) {
    check_equal_shape(a, b);
    a.for_each_element([&](const int *pos) {
        if (a(pos) != b(pos)) {
            printf("Mismatch: %f vs %f at %d %d %d\n",
                   (float)(a(pos)), (float)(b(pos)), pos[0], pos[1], pos[2]);
            abort();
        }
    });
}



int main(int argc, char **argv) {
    {
        // Check copying a buffer
        Buffer<float> a(100, 3, 80), b(120, 80, 3);

        // Mess with the memory layout to make it more interesting
        a.transpose(1, 2);

        a.fill(1.0f);
        b.for_each_element([&](int x, int y, int c) {
            b(x, y, c) = x + 100.0f * y + 100000.0f * c;
        });

        check_equal(a, a.copy());

        // Check copying from one subregion to another (with different memory layout)
        Buffer<float> a_window = a.make_alias();
        a_window.crop(0, 20, 20);
        a_window.crop(1, 50, 10);
        Buffer<const float> b_window = b.make_alias();
        b_window.crop(0, 20, 20);
        b_window.crop(1, 50, 10);
        a_window.copy_from(b);

        check_equal(a_window, b_window);

        // You don't actually have to crop a.
        a.fill(1.0f);
        a.copy_from(b_window);
        check_equal(a_window, b_window);

        // The buffers can have dynamic type
        Buffer<void> &a_void(a.as<void>());
        Buffer<const void> &b_void_window(b_window.as<const void>());
        a.fill(1.0f);
        a_void.copy_from(b_void_window);
        check_equal(a_window, b_window);
    }

    {
        // Check make a Buffer reference from a Buffer of a different type
        Buffer<float, 4> a(100, 80);
        auto &b = a.as<const float, 3>(); // statically safe
        auto &c = b.as<const void, 3>();  // statically safe
        auto &d = c.as<const float, 2>(); // does runtime checks of actual type.
        auto &e = a.as<void, 3>();        // statically safe
        auto &f = e.as<float, 2>();       // runtime checks
        (void)d;
        (void)f;
    }

    {
        // Check moving a buffer around
        Buffer<float> a(100, 80, 3);
        a.for_each_element([&](int x, int y, int c) {
            a(x, y, c) = x + 100.0f * y + 100000.0f * c;
        });
        Buffer<float> b = a.copy();
        b.set_min(123, 456, 2);
        b.translate({-123, -456, -2});
        check_equal(a, b);
    }

    {
        // Check lifting a function over scalars to a function over entire buffers.
        const int W = 5, H = 4, C = 3;
        Buffer<float> a(W, H, C);
        Buffer<float> b = Buffer<float>::make_interleaved(W, H, C);
        int counter = 0;
        b.for_each_value([&](float &b) {
            counter += 1;
            b = counter;
        });
        a.for_each_value([&](float &a, float b) {
            a = 2*b;
        }, b);

        if (counter != W * H * C) {
            printf("for_each_value didn't hit every element\n");
            return -1;
        }

        a.for_each_element([&](int x, int y, int c) {
            // The original for_each_value iterated over b, which is
            // interleaved, so we expect the counter to count up c
            // fastest.
            float correct_b = 1 + c + C * (x + W * y);
            float correct_a = correct_b * 2;
            if (b(x, y, c) != correct_b) {
                printf("b(%d, %d, %d) = %f instead of %f\n",
                       x, y, c, b(x, y, c), correct_b);
                abort();
            }
            if (a(x, y, c) != correct_a) {
                printf("a(%d, %d, %d) = %f instead of %f\n",
                       x, y, c, a(x, y, c), correct_a);
                abort();
            }
        });
    }

    printf("Success!\n");
    return 0;
}
