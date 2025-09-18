#pragma once

#ifdef _MSC_VER
#   define DLLEXPORT __declspec(dllexport)
#else
#   define DLLEXPORT
#endif

/*
This is a package domain, and all code should be included
in the package domain except for macro instructions.

`$pkg` is essentially a macro definition, similar to the
`crate` keyword in Rust, created to address possible symbol
conflicts between packages.
*/
namespace $pkg {
    DLLEXPORT int add(int a, int b);
}