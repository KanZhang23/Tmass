noinst_PROGRAMS = hello_geners storing_arrays storing_objects \
                  storing_stl_containers inheritance_example \
                  serializing_external_classes use_of_references \
                  storing_tuples optimized_tuple_io tuple_collections \
                  geners_archives storing_user_templates \
                  serializing_external_templates external_inheritance_example

hello_geners_SOURCES = hello_geners.cc

storing_arrays_SOURCES = storing_arrays.cc

storing_objects_SOURCES = storing_objects.cc SimpleSerializable1.cc \
    SimpleSerializable2.cc

storing_stl_containers_SOURCES = storing_stl_containers.cc \
    SimpleSerializable2.cc

inheritance_example_SOURCES = inheritance_example.cc SerializableBase.cc \
    SerializableDerivedA.cc SerializableDerivedB.cc SerializableBaseReader.cc

serializing_external_classes_SOURCES = serializing_external_classes.cc

use_of_references_SOURCES = use_of_references.cc

storing_tuples_SOURCES = storing_tuples.cc

optimized_tuple_io_SOURCES = optimized_tuple_io.cc

tuple_collections_SOURCES = tuple_collections.cc

geners_archives_SOURCES = geners_archives.cc

storing_user_templates_SOURCES = storing_user_templates.cc \
    SimpleSerializable2.cc

serializing_external_templates_SOURCES = serializing_external_templates.cc \
    SimpleSerializable2.cc

external_inheritance_example_SOURCES = external_inheritance_example.cc \
    ExtBaseIO.cc ExtDerivedAIO.cc ExtDerivedBIO.cc

LDADD = -L../geners -lgeners $(DEPS_LIBS) $(BZIP2_LIBS) $(ZLIB_LIBS)

INCLUDES = -I@top_srcdir@/ -I. $(DEPS_CFLAGS) $(BZIP2_CFLAGS) $(ZLIB_CFLAGS)

TESTS = $(noinst_PROGRAMS)

EXTRA_DIST = SerializableBase.hh SerializableBaseReader.hh \
             SerializableDerivedA.hh SerializableDerivedB.hh \
             SimpleSerializable1.hh SimpleSerializable2.hh \
             SerializableTemplate1.hh SerializableTemplate2.hh \
             ExternalTemplate.hh ExternalTemplateIO.hh \
             ExtBase.hh ExtBaseIO.hh ExtDerivedA.hh ExtDerivedAIO.hh \
             ExtDerivedB.hh ExtDerivedBIO.hh 00README.txt
