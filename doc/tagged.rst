.. _tagged-formatting-api:

**********
Tagged API
**********

The use of tagged API is based on two simple rules:

1. Your goal is to create a single form (let's simplify it to a string)
   that contains characters of the formatted values, one by one.
2. If a value is of any string-like type, it is simply copied as is to
   the output.
3. For any other type, the value is passed through the ``ffmt`` function,
   which will apply a type-dependent default formatting and return a string-like
   type so that it is taken just like in point 2 above.
4. If you want to apply any non-default formatting for a particular value,
   use the ``ffmt`` function explicitly, and pass the value and the desired
   formatters.

Example: let's say, you have the following variables:

.. code:: c++

   std::string name;
   size_t size;
   std::chrono::milliseconds duration;

When you try to print them this way:

.. code:: c++

   fmt::ffprint(cout, "Received entity: ", name, " size=", size, " lasts up to ", duration, "\n");

Then it is automatically turned into this form:

.. code:: c++

   fmt::ffprint(cout, "Received entity: ", name, " size=", fmt::ffmt(size), " lasts up to ", fmt::ffmt(duration), "\n");

Note here that the string-typed ``name`` is copied as is, while ``size`` is passed
through ``fmt::ffmt`` with no formatting tags (so, formatted with default settings),
which turns it into ``fmt::basic_memory_buffer``, which in turn is copied as is, too.
If you want to apply any non-default formatting for ``size``, for example, with width
6 and filled with zeros in front, then you should use
``fmt::ffmt(size, fmt::width(6), fmt::fillzero)``.

Or ``fmt::ffmt(size, "06")``, if you prefer that way.


Motivation: why tagged API and variadic functions
=================================================

Ok, if you are interested in a long version, `here it is
<https://sektorvanskijlen.wordpress.com/2023/11/27/format-string-considered-not-exactly-that-harmless/>`_.

Short version: My professional experience confirms that the way of formatting using
C++'s ostream is in some ways superior to string-based formatting specification. The
only bad design decision there was that format settings were provided as a stream
state, but beside this it was highly advantageous over ``printf``:

1. You just specify subsequent items to be printed in the order of specification.
This is something that was used everywhere, until C language came with its ``printf``,
already since BASIC:

.. code:: basic

   PRINT "origin="; left; ","; bottom; " dimensions="; (right-left); " x "; (top-bottom)

And the same you have today in many languages, notably Python's ``print`` function
or Javascript's ``console.log`` (some offer also interpolation, but it's not possible
in C++). C++ simply offered the same thing, although due to the limitations of the
first C++ standard and the lack of variadic functions (provided only in C++11),
they had to use the operator overloading:

.. code:: c++

   cout << "origin=" << left << "," << bottom << " dimensions="
        << (right-left) << " x " << (top-bottom);

2. The use of language-provided manipulator names, even if need more effort to write,
are clearer to people who are maintaining the code.

The only reason why ``printf`` was created was to fit in the C language limitations.
And the problem isn't in the need of explicit type specification in the format
string, but the format string itself. It can be argued, of course, that the format
string makes things clear as posing a formatting scheme, but this is true only if
you use named tags, with having still the format specification close to the value
being formatted.

So my first approach was to expand on the original C++ standard's idea, just solve
somehow the problem with state-based formatting settings. The solution was actually
quite simple - `provide the wrapper function for nondefault formatting
<https://gist.github.com/ethouris/2b431e1086c2197f516e609b1b4bf023>`_:

.. code:: c++

   cout << "origin=" << fmt(left, std::left, std::setw(20))
        << "," << fmt(bottom, std::right, std::setw(20)) << " dimensions="
        << (right-left) << " x " << (top-bottom);

Of course, in the above snippet you can find a "Print" function, which turns the
above into a function call.

This solution had one small problem, however: the performance is even poorer
than using plain iostream (what this ``fmt`` call actually does is to save the
ios flags, apply changes through manipulators, send the value to the stream, and
then restore the original flags).

So, in order to take advantage of the {fmt}'s performance, as well as its formatting
facilities, while implementing this above idea, the best way was to add this to the
{fmt} library. In the beginning I have even tried to negotiate with the {fmt}'s authors
that they expose the settings structure and provide the direct access to the writer
function, but they didn't seem interested. So I decided to use it on my own.


Motivation: why forking {fmt}
=============================

I'd like to simply explain, why I have used this library as a base for providing
something that looks otherwise completely unrelated: a value-collecting formatter
that almost completely discards the idea of a string-based formatter. Why haven't I
written it just anew, but forked the {fmt} library instead? There are several reasons:

1. The {fmt} library, beside the API using the string-based formatter, provides
   also important internal facilities for formatting, mainly based on the formatter
   structure.
2. There are also several other facilities worth reusing, like the formatting
   specification provided using direct string characters, just for a single value.
3. The {fmt} library has proven to be of high performance, beating the standard
   C++ iostream library by an order of magnitude.
4. And, last but not least, the library is distributed with a very liberal license,
   with only a requirement to leave the LICENSE file intact.

My initial idea was to just make a derivative work for {fmt}, with the hope to be
able to do the same with the C++20 standard library, but this required to have a
direct access to the structure known as ``format_specs``, as well as a direct
access to a function that performs the formatting using the specs. That's not only
not a part of the public API of {fmt} (and despite my request, people from the {fmt}
library team were not interested in exposing it), but additionally in the C++20
standard library version they use some mangled names, so even if publicly accessible,
such a library wouldn't be portable, even across multiple versions of the same
compiler.

So finally I resolved to forking the {fmt} library, even if this means that I would
still have to follow upgrades of {fmt} and the code can't be shared with C++20 standard
library.

This has actually proven to be moreover the better idea because both format string
and named tags are available and you can freely mix together all these facilities and
find they way that fits you best. For example, if you agree with me that the named
tags are the only useful in the format string, then this:

.. code:: c++

fmt::print("Hello, {name}! The answer is {number:04}. Goodbye, {name}.",
           fmt::arg("name", "World"), fmt::arg("number", 42));

can be also able to be written as:

.. code:: c++

fmt::print("Hello, {name}! The answer is {number}. Goodbye, {name}.",
           fmt::arg("name", "World"),
           fmt::arg("number", fmt::ffmt(42, "04")));


Value collector functions
=========================

There are several functions that collect multiple values to glue them
together, using different destinations:

* ``ffprint``: formats the values and prints them on the Stream (``std::ostream`` or ``FILE*``).

.. code:: c++

    void fmt::ffprint(Stream sout, T&&... args);

* ``ffwrite``: formats the values and writes the formatted text into the character container.

.. code:: c++

   void fmt::ffwrite(Container c, T&&... args);

* ``ffcat``: format the values into a string and return it (use ``wffcat`` for ``std::wstring``)

.. code:: c++

   std::string fmt::ffcat(T&&... args);

In all these functions you just pass values to be formatted and added to the
resulting string.


Direct formatting functions
===========================

Use these functions to apply formatting to a single value.

In these functions you pass a single value ``v`` to be formatted and optionally
any number of formatting tags. These tags can be the named tags, or a string with
formatting specification, the same as the one used in the string-based formatting.
See below for the information about tags.

* ``ffmt``: formats the single value and returns the result as ``fmt::basic_memory_buffer``

.. code:: c++

   fmt::basic_memory_buffer<char> ffmt(Value v, Args... formatters);
   fmt::basic_memory_buffer<wchar_t> wffmt(Value v, Args... formatters);

* ``ffmto``: formats the single value and writes it through the iterator

.. code:: c++

   OutIter ffmto(OutIter oi, Value v, Args... formatters);
   OutIter wffmto(OutIter oi, Value v, Args... formatters);

* ``ffmts``: formats the single value and returns the result as a string

.. code:: c++

   std::string ffmts(Value v, Args... formatters);
   std::wstring wffmts(Value v, Args... formatters);

Note that in ostream interface for this library there was added the
``operator<<`` version for ``fmt::basic_memory_buffer``. Therefore you can also
use ``ffmt`` function together with ostream directly. Here is the above
example rewritten:

.. code:: c++

   cout << "origin=" << fmt::ffmt(left, fmt::left, fmt::width(20))
        << "," << fmt::ffmt(bottom, fmt::right, fmt::width(20)) << " dimensions="
        << (right-left) << " x " << (top-bottom);


Formatting tags
===============

The trick for formatting tags is to prepare the structure of the type named
``fmt::format_specs``. This structure describes the formatting configuration.
The idea of the formatting tags is to change the default values into the
desired ones, then this structure is passed to a function that is expected
to format the value according to the rules.

Among the named tags, there's also a possibility to use a string with formatting
specification. For example:

.. code:: c++

   ffprint(cout, "The value is ", ffmt(val, scientific),
                 " (around ", ffmt(val, fixed, precision(8)), ")\n");

can be also written as:

.. code:: c++

   ffprint(cout, "The value is ", ffmt(val, "e"),
                 " (around ", ffmt(val, ".08f"), ")\n");

The named tags are designed to be very similar to the iostream's manipulators,
but there are important differences:

1. There's no ``uppercase`` tag. Instead there are tags with uppercase
variants, where case may matter, and they have just simply added ``u``
in front.

2. It was chosen that tags applying specific setting value (not just boolean
presence) do not use the ``set`` prefix (so there are ``width`` and ``precision``
tags, not ``setw`` and ``setprecision``). NOTE THAT IT IS CONSIDERED to add
aliases with similar names.

3. There's no formatter tag for a boolean value (such as ``std::boolalpha``).
CONSIDERED is adding a special facility to allow a user create their own
boolean value interpreters with provided some predefined values. Actually the
simplest way for an application is to create an array such as ``const char*
truefalse[2] = {"false", "true"};`` and then you can simply use
``truefalse[val]`` to make ``val`` printed as boolalpha.

4. Note also that formatting is adjusted to the features of the {fmt} library,
which are sometimes different to the one from the standard C++ library. For
example, in {fmt} there's no formatting known as ``std::internal``, as well
as the width specification is the exact, not minimum width.

By using the tags, you should take care that tags you are using make sense
and are consistent. Some of the tags may mean different things, but will
result in setting the same config entry, or the setting is interpreted
differently depending on the value type. Also next tags may override the
setting of the previous one. This includes also tag combinations using a
string-specified tags and named tags.

The following tags are provided:

Alignment tags:

* right (default)
* left
* center
* width(N)

When there's a padding required (the , value is aligned right or left. For center
the same padding is added in half on both right and left. This corresponds
to the {fmt} format markers ``<``, ``>`` and ``^``. The ``width`` tag is
parametrized and provides the number of characters that the value should take
(if the value is shorter, it uses padding with a fill character).

Sign tags:

* showneg (default)
* showpos
* showspace

The default ``showneg`` means that only the negative number is prefixed by
a minus sign (which is always the case anyway), but the positive numbers
are not prefixed. With ``showpos`` it is prefixed by a plus sign and with
``showspace`` with a space.

Alternative form tags:

* showbase
* showpoint
* falt

Actually all these tags set exactly the same boolean setting to true, which 
is false by default, and there exist also their counterparts with ``no``
prefix, which simply do nothing. This flag changes things depending on
the value type:

1. For integer values, it applies the prefix: ``0x`` for hex, ``0b`` for
binary and ``0`` for oct.

2. For floating-point type values, it always prints the decimal point
in case of fixed formatting, even if the fraction part is zero.

Filling tags:

* fillspace (default)
* fillzero
* fill(S)

Defines what character should be used to fill the padding in case when
it is present. The default is space. The ``fillzero`` defines the "0"
be used (a dedicated tag is provided because this has its dedicated
marker in the string formatting, as well as it's a known practice to use
leading zeros in case of numbers formatted to the equal width). The
parametrized ``fill`` tag allows to use any string for filling. The
parameter uses the string view type.

Numeric base tags:

* dec (default)
* hex, uhex
* bin, ubin
* oct

Applies the base of decimal, hexadecimal, binary and octal. There are
versions with ``u`` prefix to apply the uppercase. This defines the
case for letters used in the value, but also the letter case for the
prefix, if combined with ``showbase``. For binary, only the latter
applies.

Floating-point tags:

* fixed
* scientific/fexp, uscientific/ufexp
* general/ugeneral
* fhex/ufhex
* precision(N)

These define the floating-point presentation: ``fixed`` is the usual
representation with decimal point, ``fexp`` uses the significand-exponent format
using the letter ``e``, otherwise known as ``scientific`` (the alias provided
for convenience as a name used in iostream, but some may prefer ``fexp`` as
shorter and more straightforward). The ``general`` formats the value as either
scientific or fixed, with the latter used only if the value can be still
printed with all fraction parts with given precision, otherwise scientific.
Versions with ``u`` prefix use uppercase ``E`` letter for exponent and for
``NAN`` or ``INF`` strings.

The ``fhex`` tag requests the floating-point hexadecimal representation.
Note that it is not interchangeable with ``hex``. In this very case the
``f`` specifically designates floating-point, while in the others (``fexp``,
``falt`` and ``fdebug``), it's only to avoid at least visual name conflicts
with too simple names.

The ``precision`` tag is parametrized and defines the number of significant
digits after the decimal point.

Special tags:

* fdebug

If used, requests tracking of all characters being sent out to the format
and nonprintable characters replaced by a special string representation.

