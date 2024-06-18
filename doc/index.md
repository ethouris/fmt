# A modern formatting library

## Overview

**{fmt}** is an open-source formatting library providing a fast and safe
alternative to C stdio and C++ iostreams.

This version is a forked version of **{fmt}**, which provides
additionally the formatter tags and facilities for sequenced formatting
in order to have an almost perfect drop-in replacement for iostreams and
any printing and logging solutions based on iostreams - [see
below](#tagged).

## String-based format API {#format-api-intro}

The string-based format API is similar in spirit to the C `printf`
family of function, although it is safer, simpler and several times
[faster](https://www.zverovich.net/2020/06/13/fast-int-to-string-revisited.html)
than common standard library implementations. The [format string
syntax](syntax.rst) is similar to the one used by
[str.format](https://docs.python.org/3/library/stdtypes.html#str.format)
in Python.

## Tagged on-demand format API {#tagged}

This API consists of two main elements:

1.  Variadic functions that glue together the formatted versions of all
    subsequent arguments.
2.  Named tags, similar to iostream manipulators, for formatting the
    value.

For example:

``` c++
ffprint(cout, "Name: ", name, " age: ", age, " serial: ",
              ffmt(serial, width(10), fillzero), " status: ", st, "\n");
```

## Safety in the string-based format API

Errors in format strings, which are a common source of vulnerabilities in C,
are **reported at compile time**. For example:

```cpp
fmt::format("{:d}", "I am not a number");
```

will give a compile-time error because `d` is not a valid format specifier for
strings. APIs like [`fmt::format`](api/#format) **prevent buffer overflow
errors** via automatic memory management.

[→ Learn more](api#compile-time-checks)

## Extensibility

Formatting of most **standard types**, including all containers, dates, and
times is **supported out-of-the-box**. For example:

```cpp
fmt::print("{}", std::vector{1, 2, 3});
```

prints the vector in a JSON-like format:

```
[1, 2, 3]
```

You can **make your own types formattable** and even make compile-time checks
work for them.

[→ Learn more](api#udt)

## Performance

{fmt} can be anywhere from **tens of percent to 20-30 times faster** than
iostreams and `sprintf`, especially for numeric formatting. [![](perf.svg)
](https://github.com/fmtlib/fmt?tab=readme-ov-file#benchmarks)The library
**minimizes dynamic memory allocations** and can optionally [compile format
strings](api#compile-api) to optimal code.

## Fast compilation

The library makes extensive use of **type erasure** to achieve fast
compilation. `fmt/base.h` provides a subset of the API with **minimal include
dependencies** and enough functionality to replace all uses of `*printf`.

Code using {fmt} is usually several times faster to compile than the equivalent
iostreams code, and while `printf` compiles faster still, the gap is narrowing.

[→ Learn more](https://github.com/fmtlib/fmt?tab=readme-ov-file#compile-time-and-code-bloat)

## Small binary footprint

Type erasure is also used to prevent template bloat, resulting in **compact
per-call binary code**. For example, a call to `fmt::print` with a single
argument is just [a few instructions](https://godbolt.org/g/TZU4KF), comparable
to `printf` despite adding runtime safety, and much smaller than the equivalent
iostreams code.

The library itself has small binary footprint and some components such as
floating-point formatting can be disabled to make it even smaller for
resource-constrained devices.

## Portability

{fmt} has a **small self-contained codebase** with the core consisting of just
three headers and no external dependencies.

The library is highly portable and requires only a minimal **subset of C++11**
features which are available in GCC 4.8, Clang 3.4, MSVC 19.0 (2015) and later.
Newer compiler and standard library features are used if available, and enable
additional functionality.

An extra part contained in `sfmt.h` include file is added for projects that
cannot upgrade to C++11, but would like to make any temporary solition that
can allow to easily transit to {fmt} later.

Where possible, the output of formatting functions is **consistent across
platforms**.

