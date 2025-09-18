#pragma once

/*
This is a package domain, and all code should be included
in the package domain except for macro instructions.

`$pkg` is essentially a macro definition, similar to the
`crate` keyword in Rust, created to address possible symbol
conflicts between packages.
*/
namespace $pkg {
    // This is a function.
    int add(int a, int b);
}

// You can also use `$pkg` in this way to avoid excessive indentation.
#define PKG_BEGIN namespace $pkg {
#define PKG_END }

PKG_BEGIN

int sub(int a, int b);

PKG_END

