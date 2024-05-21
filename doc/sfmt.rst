.. _lightweight-compat-api:


*******************************************
Lightweight compat on-demand formatting API
*******************************************

This API provides a similar functionality to the tagged on-demand formatting
API on top of ``FILE*`` type objects and using ``snprintf`` for formatting.


Lightweight snprintf wrapper
============================

The following function is used for on-demand formatting:

.. code:: c++

   template<class Value>
   fmt::internal::form_memory_buffer<>
   		fmt::sfmt(Value val, const char* format = NULL);

There's also a version that returns this value as a string:

.. code:: c++

   template<class Value>
   std::string fmt::sfmts(Value val, const char* format = NULL);

The syntax for ``format`` parameter is the following:

1. Use the numeric specifications (for width and precision) and
   characters such as ``#'. +-`` as required.
2. **DO NOT** use any type-specific modifiers (``l``, ``ll``, ``h``
   or ``L``, as well as ``j``, ``z`` or ``t``).
3. A special string ``<!!!>`` will be printed after the formatted
   value, if the formatting string contained any wrong characters.
4. Wrong characters depend on the type. For example, you can't use
   precision for strings, ``x`` for ``double`` or ``u`` for ``int``.
   As a special exception, ``x`` and ``o`` are allowed for ``int``,
   despite that the value will be actually displayed as unsigned.

Note also that all rules for formatting are determined by the syntax
used by the ``printf`` function family and differ to those used by
the {fmt} library.
   
Iostream-style FILE wrappers
============================

To allow the use of these functions, there is provided also the
wrapper for ``FILE*`` using ``operator<<`` for sending data to it.
This is required if you want to use any software that is required
to be C++03-compatible, that is, where variadic functions are not
available.

.. code:: c++

   class ostdiostream;

   ostdiostream::ostdiostream(FILE* file);

   template<class Value>
   ostdiostream& ostdiostream::operator<<(const Value& val);

Versions of ``operator<<`` for ``const char*``, ``std::string`` and
``fmt::internal::form_memory_buffer`` print contained characters in the
original form. For all other values there's ``sfmt`` call done internally
with no format specification. To use any other formatting, you can call
the ``sfmt`` function explicitly:

.. code:: c++

   ostdiostream sout(stdout);

   sout << "The value is " << sfmt(val, "e")
        << " (around " << sfmt(val, ".08f") << ")\n";

Additionally for writing to files that have to be opened and closed by
the user, there's a convenience wrapper:

.. code:: c++

    class ostdiofstream;

    ostdiofstream::ostdiofstream();
    ostdiofstream::ostdiofstream(const std::string& filename, const std::string& mode);
    void ostdiofstream::open(const std::string& filename, const std::string& mode);
    bool ostdiofstream::good(); // returns false if internal FILE* is NULL
    void ostdiofstream::attach(FILE* existing_file);
    FILE* ostdiofstream::detach();   // Sets internal FILE* to NULL, returns previous value
    int ostdiofstream::close();      // Calls ``fclose`` and returns its result
    ostdiofstream::~ostdiofstream(); // Closes the file

The default constructor constructs a NULL-initialized file, which shall not be
used. The ``open`` method and the constructor with filename and mode simply forward
to ``std::fopen`` and don't check the result; you should do it yourself by
calling ``good()`` afterwards. If you use some other function to open a file
than ``fopen`` to create the ``FILE*`` stream (and it should still be closed
by ``fclose``), you can also use ``attach()``. The use of ``detach`` can prevent
the file from being closed in this class's destructor.
