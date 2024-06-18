# Lightweight compat on-demand formatting API {#lightweight-compat-api}

This API provides a similar functionality to the tagged on-demand
formatting API on top of `FILE*` type objects and using `snprintf` for
formatting.

## Lightweight snprintf wrapper

The following function is used for on-demand formatting:

``` c++
template<class Value>
fmt::internal::form_memory_buffer<>
     fmt::sfmt(Value val, const char* format = NULL);

template<class Value>
fmt::internal::form_memory_buffer<>
     fmt::sfmt(Value val, const fmt::sfmc& config);
```

There is also a version that returns this value as a string:

``` c++
template<class Value>
std::string fmt::sfmts(Value val, const char* format = NULL);

template<class Value>
std::string fmt::sfmts(Value val, const fmt::sfmc& config);
```

## String specification based formatting

Here `format` is what you would use in the `%` tag in printf, except the
`%` character itself, and also with further limitations:

1.  Special format flags `#'. +-` and width/precision specification
    should be used normally, but only if they apply to the value type.

2.  **DO NOT** use any type-specific modifiers (`l`, `ll`, `h` or `L`,
    as well as `j`, `z` or `t`) or any `PRI*` macros. Note that `j`, `z`
    and `t` refer to library-defined types that are typedefs to any
    short/long signed/unsigned integers, and are only provided for
    convenience for printf.

3.  A special string `<!!!>` will be printed after the formatted value,
    if the formatting string contained any wrong characters.

4.  Wrong characters depend on the type. For example, you cannot use `#`
    for strings, `x` for `double` or `u` for `int`. As a special
    exception, `x/X` and `o` are allowed for `int`, despite that the
    value will be actually displayed as unsigned. You can\'t enforce
    unsigned integer form for `int` or `char` by using `u` tag though;
    instead simply use `sfmt<unsigned>(...)`.

5.  The printf family is using the last character as both the
    presentation specifier and type specifier. As type is detected
    in C++, this flag is optional, and the defaults are: `g` for
    floating-point types, `i` for signed integer types and `u` for
    unsigned integer types.

Note also that all rules for formatting are determined by the syntax
used by the `printf` function family and differ to those used by the
{fmt} library.

## Structure based formatting

Additionally, if you prefer, you may specify the formatting flags using
a special structure named `sfmc`. This is intended to be used as a
temporary object together with methods changing the default values to
desired ones:

`fmt::sfmt(value, fmt::sfmc().width(10).precision(0).alt())`

The `sfmc` structure provides settings that will be then used to craft
the format string passed then to the `snprintf` call. The following flag
modifier methods are provided:

-   `alt` - Alternative representation (`#`)
-   `left` - Align to left (`-`)
-   `right` - Align to right (default)
-   `width` - Set the width (with parameter)
-   `precision` - Set the precision (with parameter)
-   `dec` - Decimal (default for integers)
-   `hex` - Hexadecimal with lowercase letters
-   `oct` - Octal
-   `uhex` - Hexadecimal with uppercase letters
-   `general` - Scientific or fixed as per precision (default)
-   `ugeneral` - Like general, with uppercase if scientific
-   `fhex` - Floating-point hexadecimal
-   `ufhex` - Floating-point hexadecimal (uppercase)
-   `exp/scientific` - Scientific presentation for floating-point
-   `uexp/uscientific` - Scientific presentation with uppercase E/INF/NAN
-   `fixed` - Fixed-point presentation for floating-point
-   `nopos` - Do not prefix positive numbers (default)
-   `posspace` - Prefix positive numbers with space
-   `posplus` - Prefix positive numbers with plus
-   `fillzero` - Fill width-padding with 0

## Iostream-style FILE wrappers

For convenience of using these functions, there are also some special
stream classes provided, which define `operator<<` in the style of
iostream for sending data to it. This is required if you want to use any
software that must be C++03-compatible, that is, where variadic
functions are not available. The following classes are available:

-   `fmt::ostdiostream`: a simple wrapper for `FILE*` intended to be
    used mainly for `stdout` or as a temporary wrapper to some existing
    `FILE` object

-   `fmt::ofilestream`: a managed wrapper for `FILE*`

-   `fmt::obufstream`: a buffering string builder, a replacement for
    `std::ostringstream`

All these classes have defined `operator<<`, which writes directly the
passed value in case of `std::string` or
`fmt::internal::form_memory_buffer` (you can also provide your own
overloads, just note that `obufstream` and `ostdiostream` are
unrelated), and for all other values the `fmt::sfmt` function is used
intermediately. This function should be used explicitly, if you want to
use any nonstandard formatting:

``` c++
ostdiostream sout(stdout);

sout << "The value is " << sfmt(val, "e")
     << " (around " << sfmt(val, ".08f") << ")\n";
```

The `ostdiostream` class is a simple wrapper around `FILE*` providing
the `operator<<` definition:

``` c++
class ostdiostream;

ostdiostream::ostdiostream(FILE* file);
```

Additionally, for writing to files that have to be opened and closed by
the user, there is a convenience wrapper:

``` c++
class ofilestream;

ofilestream::ofilestream();
ofilestream::ofilestream(const std::string& filename, const std::string& mode);
void ofilestream::open(const std::string& filename, const std::string& mode);
bool ofilestream::good(); // returns false if internal FILE* is NULL
void ofilestream::attach(FILE* existing_file);
FILE* ofilestream::detach();   // Sets internal FILE* to NULL, returns previous value
int ofilestream::close();      // Calls ``fclose`` and returns its result
ofilestream::~ofilestream(); // Closes the file
```

The default constructor constructs a NULL-initialized file, which shall
not be used. The `open` method and the constructor with filename and
mode simply forward to `std::fopen` and don\'t check the result; you
should do it yourself by calling `good()` afterwards. If you use some
other function to open a file than `fopen` to create the `FILE*` stream
(and it should still be closed by `fclose`), you can also use
`attach()`. The use of `detach` can prevent the file from being closed
in this class\'s destructor.

And the `obufstream` is though of as string builder:

``` c++
class obufstream;

size_t size() const; // a total size of a possibly fragmented internal buffer
std::string str() const; // creates a solid std::string from the internal buffer
void copy_to(OutputContainer& out) const; // copies contents using std::back_inserter
```

This class is thought of as a replacement for `std::ostringstream`.
