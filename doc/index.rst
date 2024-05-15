Overview
========

**{fmt}** is an open-source formatting library providing a fast and safe
alternative to C stdio and C++ iostreams.

.. raw:: html

   <div class="panel panel-default">
     <div class="panel-heading">What users say:</div>
     <div class="panel-body">
       Thanks for creating this library. It’s been a hole in C++ for
       a long time. I’ve used both <code>boost::format</code> and
       <code>loki::SPrintf</code>, and neither felt like the right answer.
       This does.
     </div>
   </div>

.. _tagged-info:

This version is a forked version of **{fmt}**, which provides additionally
the formatter tags and facilities for sequenced formatting in order to have
an almost perfect drop-in replacement for iostreams and any printing and logging
solutions based on iostreams - `see below <#tagged>`_.

.. _format-api-intro:

String-based format API
-----------------------

The string-based format API is similar in spirit to the C ``printf`` family of
function, although it is safer, simpler and several times `faster
<https://www.zverovich.net/2020/06/13/fast-int-to-string-revisited.html>`_
than common standard library implementations.
The `format string syntax <syntax.rst>`_ is similar to the one used by
`str.format <https://docs.python.org/3/library/stdtypes.html#str.format>`_ in
Python:

.. code:: c++

  std::string s = fmt::format("The answer is {}.", 42);
  
The ``fmt::format`` function returns a string "The answer is 42.". You can use
``fmt::memory_buffer`` to avoid constructing ``std::string``:

.. code:: c++

  auto out = fmt::memory_buffer();
  fmt::format_to(std::back_inserter(out),
            "For a moment, {} happened.", "nothing");
  auto data = out.data(); // pointer to the formatted data
  auto size = out.size(); // size of the formatted data

The ``fmt::print`` function performs formatting and writes the result to a stream:

.. code:: c++

  fmt::print(stderr, "System error code = {}\n", errno);

If you omit the file argument the function will print to ``stdout``:

.. code:: c++

  fmt::print("Don't {}\n", "panic");

The format API also supports positional arguments useful for localization:

.. code:: c++

  fmt::print("I'd rather be {1} than {0}.", "right", "happy");

You can pass named arguments with ``fmt::arg``:

.. code:: c++

  fmt::print("Hello, {name}! The answer is {number}. Goodbye, {name}.",
             fmt::arg("name", "World"), fmt::arg("number", 42));

If your compiler supports C++11 user-defined literals, the suffix ``_a`` offers 
an alternative, slightly terser syntax for named arguments:

.. code:: c++

  using namespace fmt::literals;
  fmt::print("Hello, {name}! The answer is {number}. Goodbye, {name}.",
             "name"_a="World", "number"_a=42);

.. _tagged:

Tagged format API
-----------------

This feature is aimed at a nearly drop-in replacement of the iostream format API.
Two most important part of this feature are:

1. Provide the named tags, similar to iostream manipulators, for formatting the value.
2. Provide variadic functions that will format all arguments one after another and
   glue them together - the same thing that the ``ostream::operator<<`` does, just in the
   style of a function call.

One of the reasons of providing ``operator<<`` for ostream was the lack of variadic
functions in the first C++ standard. Fortunately since C++11 the variadic functions
can be defined, so the following expression:

.. code:: c++

   cout << "I'd rather be " << pri[0] << " than " << pri[1] << "\n";

can be also written as:

.. code:: c++

   ffprint(cout, "I'd rather be ", pri[0], " than ", pri[1], "\n");

By weird reasons, however, it was chosen that ostream will use the formatting
settings as a state. In result, if you want to print the value of RGBA, you
can do simply:

.. code:: c++

   cout << hex << setfill('0') << setw(2) << r << g << b << a;

just the problem is that if you try to put ``<< " " <<`` between the values,
this will result in printing zero followed by a space. This problem doesn't
have a simple solution - either you reset the stream flags after printing
each value (before C++98 there was an idea that these manipulators only change
settings for the next value and get reset after this one is printed) or just
resolve to ``sprintf(buf, "%02 x%02 x%02 x%02x", r, g, b, a)``.

The tagged format API provides the same thing, while not using the state to
keep the formatting settings - all formatting settings are assigned to the
individual value. So you can still use tagged formatters:

.. code:: c++

   ffprint(cout, ffmt(r, hex, fillzero, width(2)), " ",
				 ffmt(g, hex, fillzero, width(2)), " ",
				 ffmt(b, hex, fillzero, width(2)), " ",
				 ffmt(a, hex, fillzero, width(2)));

and also the string formatters:

.. code:: c++

   ffprint(cout, ffmt(r, "02x"), " ",
				 ffmt(g, "02x"), " ",
				 ffmt(b, "02x"), " ",
				 ffmt(a, "02x"));

The above examples using the string-based format can be then rewritten as:

.. code:: c++

  std::string s = fmt::ffcat("The answer is ", 42, ".");
  
  // with memory_buffer:

  auto out = fmt::memory_buffer();
  fmt::ffmto(std::back_inserter(out),
            "For a moment, ", "nothing", " happened.");
  auto data = out.data(); // pointer to the formatted data
  auto size = out.size(); // size of the formatted data

  // And printing:

  fmt::ffprint(stderr, "System error code = ", errno, "\n");

See `Tagged formatting documentation <tagged.rst>`_ for more information.


.. _safety:

Safety
------

The library is fully type safe, automatic memory management prevents buffer
overflow, errors in format strings are reported using exceptions or at compile
time. For example, the code

.. code:: c++

  fmt::format("The answer is {:d}", "forty-two");

throws the ``format_error`` exception because the argument ``"forty-two"`` is a
string while the format code ``d`` only applies to integers.

The code

.. code:: c++

  format(FMT_STRING("The answer is {:d}"), "forty-two");

reports a compile-time error on compilers that support relaxed ``constexpr``.
See `here <api.html#compile-time-format-string-checks>`_ for details.

The following code

.. code:: c++

  fmt::format("Cyrillic letter {}", L'\x42e');
  
produces a compile-time error because wide character ``L'\x42e'`` cannot be
formatted into a narrow string. For comparison, writing a wide character to
``std::ostream`` results in its numeric value being written to the stream
(i.e. 1070 instead of letter 'ю' which is represented by ``L'\x42e'`` if we
use Unicode) which is rarely desirable.

Compact Binary Code
-------------------

The library produces compact per-call compiled code. For example
(`godbolt <https://godbolt.org/g/TZU4KF>`_),

.. code:: c++

   #include <fmt/core.h>

   int main() {
     fmt::print("The answer is {}.", 42);
   }

compiles to just

.. code:: asm

   main: # @main
     sub rsp, 24
     mov qword ptr [rsp], 42
     mov rcx, rsp
     mov edi, offset .L.str
     mov esi, 17
     mov edx, 1
     call fmt::v7::vprint(fmt::v7::basic_string_view<char>, fmt::v7::format_args)
     xor eax, eax
     add rsp, 24
     ret
   .L.str:
     .asciz "The answer is {}."

.. _portability:

Portability
-----------

The library is highly portable and relies only on a small set of C++11 features:

* variadic templates
* type traits
* rvalue references
* decltype
* trailing return types
* deleted functions
* alias templates

These are available in GCC 4.8, Clang 3.4, MSVC 19.0 (2015) and more recent
compiler version. For older compilers use {fmt} `version 4.x
<https://github.com/fmtlib/fmt/releases/tag/4.1.0>`_ which is maintained and
only requires C++98.

The output of all formatting functions is consistent across platforms.
For example,

.. code::

  fmt::print("{}", std::numeric_limits<double>::infinity());

always prints ``inf`` while the output of ``printf`` is platform-dependent.

.. _ease-of-use:

Ease of Use
-----------

{fmt} has a small self-contained code base with the core library consisting of
just three header files and no external dependencies.
A permissive MIT `license <https://github.com/fmtlib/fmt#license>`_ allows
using the library both in open-source and commercial projects.

`Learn more... <contents.html>`_

.. raw:: html

  <a class="btn btn-success" href="https://github.com/fmtlib/fmt">GitHub Repository</a>

  <div class="section footer">
    <iframe src="https://ghbtns.com/github-btn.html?user=fmtlib&amp;repo=fmt&amp;type=watch&amp;count=true"
            class="github-btn" width="100" height="20"></iframe>
  </div>
