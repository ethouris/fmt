.. _tagged-formatting-api:

**********
Tagged API
**********

The use of tagged API is based on some simple rules:

1. Use one of the functions that take multiple arguments, which will be
   formatted and glued together one by one.
2. If the value is any string-like type, it is copied as is to the output.
3. Values of any other types are passed through the ``fmt::ffmt`` function,
   which will return ``fmt::basic_memory_buffer``.
4. You can also use ``fmt::ffmt`` function explicitly, if you want to
   use any non-default format settings for that value.

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


Value collector functions
=========================

These functions format the values given as variadic arguments, format
them, glue them together, and write into the destination:

* ``ffprint``: writes the result to the Stream (``std::ostream`` or ``FILE*``).

.. code:: c++

    void fmt::ffprint(Stream sout, T&&... args);

* ``ffwrite``: writes the result into the character container.

.. code:: c++

   void fmt::ffwrite(Container c, T&&... args);

* ``ffcat``: writes the result into a string and returns it (use ``wffcat`` for ``std::wstring``)

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

Note that in ostream interface for this library there is also added the
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

When there's a padding required, the value is aligned right or left. For center
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
using the letter ``e``, otherwise known as ``scientific`` (this name is provided
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


Motivation: why tagged API and variadic functions
=================================================

Ok, if you are interested in a long version, `here it is
<https://sektorvanskijlen.wordpress.com/2023/11/27/format-string-considered-not-exactly-that-harmless/>`_.

Short version: According to my professional expreience, the C++ iostream is in many
ways superior to formatting in printf-style by two main reasons:

1. Values are supplied in the order of their printing, as it was since always the
case in all other languages, which is cleaner than having values and specs dispersed
throughout the format string and remaining arguments.

2. The use of language-provided manipulator names, even if need more effort to write,
are clearer to people who are maintaining the code.

Specifying the values to be printed one after another has always been the practice
used everywhere before C's printf, like even in BASIC:

.. code:: basic

   PRINT "origin="; left; ","; bottom; " dimensions="; (right-left); " x "; (top-bottom)

And the same you have today in many languages, notably Python's ``print`` function
or Javascript's ``console.log`` (some offer also interpolation, which is even better,
but it's not possible in C++). C++ simply offers the same, just done with the use of
overloaded operators because of lack of variadic functions in C++98 (added in C++11):

.. code:: c++

   cout << "origin=" << left << "," << bottom << " dimensions="
        << (right-left) << " x " << (top-bottom);

The C's printf was designed this way not because of any advantages for a programmer,
but due to C language limitations. And the problem isn't in the necessity of explicitly
specifying the type, but in the format string itself, and the resulting mess in the
parameter specification. Of course, sometimes the format string is cleaner, but that's
only if you use named tags, as this partially emulates the interpolated string. The
format string with positioned arguments is inferior to any other solution as it comes
to code's clarity and maintenance.

The C++ iostream has one major design flaw: it usees format settings as a state.
This means that when you send the ``std::hex`` manipulator to the stream, all integer
values are since then printed in hex, unless you change them back to dec. If you
write a software that produces some results in a text form, this is awkward.

So my first approach was to expand on the original C++ standard's idea, just solve
somehow the problem with state-based formatting settings. The solution was actually
quite simple - `provide the wrapper function for nondefault formatting
<https://gist.github.com/ethouris/2b431e1086c2197f516e609b1b4bf023>`_:

.. code:: c++

   cout << "origin=" << fmt(left, std::left, std::setw(20))
        << "," << fmt(bottom, std::right, std::setw(20)) << " dimensions="
        << (right-left) << " x " << (top-bottom);

Of course, in the above snippet you can find a "Print" function, which turns the
above into a function call (that's actually the original idea for ``ffprint``).

This solution had one small problem, however: the performance is even poorer
than using plain iostream (what this ``fmt`` call actually does is to save the
ios flags, apply changes through manipulators, send the value to the stream, and
then restore the original flags). This could be done alternatively by having
``fmt`` function use ``std::ostringstream``, format the value, and return the
result as ``std::string``. That could perform even worse.

So, in order to take advantage of the {fmt}'s performance, as well as its formatting
facilities, while implementing this above idea, the best way was to add this to the
{fmt} library. In the beginning I have even tried to negotiate with the {fmt}'s authors
that they expose the settings structure and provide the direct access to the writer
function, but they didn't seem interested. So I decided to use it on my own.


Motivation: why forking {fmt}
=============================

It can be argued, why fork {fmt}, while the solution I'm trying to provide seems
almost completely unrelated: a value-collecting formatter that almost completely
discards the idea of a string-based formatter. Why haven't I written it just anew,
but forked the {fmt} library instead? There are several reasons:

1. The {fmt} library provides already a complete facility for formatting. All I
   needed to make it work is to modify the format specs structure and call a
   function that will do the formatting, with that structure.
2. Translation of a string with specs syntax into the format specs structure was
   also worth reusing by providing the possibility to define format tags this way.
3. The {fmt} library has been proven to be of high performance, beating the standard
   C++ iostream library by an order of magnitude.
4. And, last but not least, the library is distributed with a very liberal license,
   with only a requirement to leave the LICENSE file intact.

My initial idea was to just make a derivative work for {fmt}, with the hope to be
able to do the same with the C++20 standard library. But neither {fmt}'s authors
nor the C++ standard guys didn't think of providing any access to the formatting
facilities - the format specs structure in C++20 has even a mangled name, so any
library provided as an extension to this would not be portable, even across
multiple versions of the same compiler.

Forking the library was actually moreover the better idea because both format string
and named tags are available and you can freely mix together all these facilities and
find they way that fits you best. For example, if you agree with me that the named
tags are the only useful feature of the format string, then this:

.. code:: c++

fmt::print("Hello, {name}! The answer is {number:04}. Goodbye, {name}.",
           fmt::arg("name", "World"), fmt::arg("number", 42));

can be also able to be written as:

.. code:: c++

fmt::print("Hello, {name}! The answer is {number}. Goodbye, {name}.",
           fmt::arg("name", "World"),
           fmt::arg("number", fmt::ffmt(42, "04")));


