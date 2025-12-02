#pragma once

#include <memory>

// Backport of `std::make_unique` for the Xbox 360 toolchain.
//
// The Xenon compiler provides std::unique_ptr but does not support:
//   - `std::make_unique` (introduced in C++14)
//   - variadic templates (required by the real make_unique)
template <typename T> inline std::unique_ptr<T> make_unique()
{
    return std::unique_ptr<T>(new T());
}
