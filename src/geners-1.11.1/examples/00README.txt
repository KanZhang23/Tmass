
This directory contains a number of examples which illustrate the usage
of "geners" package serialization and I/O facilities. Read the code
(it is reasonably well commented), run it, modify it and see what happens.

hello_geners.cc -- This program illustrates basic usage of "geners"
                   archives and I/O.

geners_archives.cc -- Illustrates various archives that can be used
                      for object I/O.

use_of_references.cc -- This program explains usage of archive references.

storing_arrays.cc -- This example shows how to store arrays in the archives.

storing_objects.cc -- Archive I/O of user-defined classes.

storing_user_templates.cc -- Archive I/O of user-defined templates.

storing_stl_containers.cc -- This program illustrates storage of STL
                             containers in the "geners" archives.

serializing_external_classes.cc -- This example shows how to extend the
                                   "geners" package so that it can store
                                   and retrieve classes which do not have
                                   their own read/write methods.

serializing_external_templates.cc -- This example shows how to extend the
                                     "geners" package so that it can store
                                     and retrieve templates which do not
                                     have their own read/write methods.

inheritance_example.cc -- This example shows how to serialize/deserialize
                          classes which belong to an inheritance hierarchy
                          using pointers and references to the base class.

external_inheritance_example.cc -- This example shows how to non-intrusively
                                   serialize and deserialize classes belonging
                                   to an inheritance hierarchy over which you
                                   have no control (that is, you can not have
                                   virtual "classId" and "write" methods).

storing_tuples.cc      -- These three examples illustrate the facilities
optimized_tuple_io.cc     provided by the "geners" package for storing
tuple_collections.cc      tuples (heterogeneous containers introduced
                          in C++11).

To assist in serializing external templates, a boilerplate code generator
executable called "template_io_boilerplate" is provided in the "tools"
directory of this package. Run that executable without any arguments to see
its usage instructions.
