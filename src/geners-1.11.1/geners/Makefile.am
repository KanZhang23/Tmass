lib_LTLIBRARIES = libgeners.la

libgeners_la_SOURCES = ClassId.cc AbsArchive.cc AbsReference.cc \
    CatalogEntry.cc ItemDescriptor.cc ItemLocation.cc ContiguousCatalog.cc \
    WriteOnlyCatalog.cc BinaryFileArchive.cc uriUtils.cc ColumnBuffer.cc \
    BinaryArchiveBase.cc CatalogIO.cc MultiFileArchive.cc GeneralCatalog.cc \
    CStringStream.cc ZlibHandle.cc BZ2Handle.cc CStringBuf.cc CharBuffer.cc \
    StringArchive.cc SearchSpecifier.cc stringArchiveIO.cc

libgeners_la_CPPFLAGS = $(DEPS_CFLAGS) $(BZIP2_CFLAGS) $(ZLIB_CFLAGS)

libgeners_la_LIBADD = $(DEPS_LIBS) $(BZIP2_LIBS) $(ZLIB_LIBS)

INCLUDES = -I@top_srcdir@/

includedir = ${prefix}/include/geners

include_HEADERS = AbsArchive.hh \
    AbsCatalog.hh \
    AbsReader.hh \
    AbsReaderWriter.hh \
    AbsRecord.hh \
    AbsReference.hh \
    allUnique.hh \
    ArchiveRecord.hh \
    ArrayAdaptor.hh \
    ArrayRecord.hh \
    ArrayReference.hh \
    ArrayReference.icc \
    arrayIO.hh \
    assignIfPossible.hh \
    associate_serialization_factory.hh \
    BinaryArchiveBase.hh \
    BinaryFileArchive.hh \
    binaryIO.hh \
    BooleanString.hh \
    BZ2Handle.hh \
    CatalogEntry.hh \
    CatalogIO.hh \
    CatalogIO.icc \
    CharBuffer.hh \
    ClassId.hh \
    ClassId.icc \
    ClearIfPointer.hh \
    collectTupleNames.hh \
    ColumnBuffer.hh \
    ColumnPacker.hh \
    ColumnPacker.icc \
    ColumnPackerHelper.hh \
    complexIO.hh \
    CompressedIO.hh \
    CompressedIO.icc \
    CPBufferRecord.hh \
    CPBufferReference.hh \
    CP_column_iterator.hh \
    CPFooterRecord.hh \
    CPFooterReference.hh \
    CPHeaderRecord.hh \
    CPP11_array.hh \
    CPP11_auto_ptr.hh \
    CPP11_config.hh \
    CPP11_shared_ptr.hh \
    CPP11_type_traits.hh \
    CPReference.hh \
    CStringBuf.hh \
    CStringStream.hh \
    dequeIO.hh \
    findName.hh \
    forward_listIO.hh \
    GeneralCatalog.hh \
    GenericIO.hh \
    GenericIO.icc \
    ContiguousCatalog.hh \
    InsertContainerItem.hh \
    Int2Type.hh \
    IOException.hh \
    IOIsPOD.hh \
    IOIsPair.hh \
    IOIsString.hh \
    IOIsContainer.hh \
    IOIsExternal.hh \
    IOIsWritable.hh \
    IOIsReadable.hh \
    IOIsSharedPtr.hh \
    IOIsClassType.hh \
    IOIsContiguous.hh \
    IOIsNumber.hh \
    IOIsSameType.hh \
    IOIsTuple.hh \
    IOIsIOPtr.hh \
    IOIsAnyPtr.hh \
    IOIsUnsigned.hh \
    IOPointeeType.hh \
    IOPtr.hh \
    IOReferredType.hh \
    IOTraits.hh \
    isStrictlyIncreasingSeq.hh \
    ItemDescriptor.hh \
    ItemLocation.hh \
    listIO.hh \
    mapIO.hh \
    MultiFileArchive.hh \
    PackerIOCycle.hh \
    platform.hh \
    ProcessItem.hh \
    ProcessItem.icc \
    pseudoIO.hh \
    Record.hh \
    Reference.hh \
    Reference.icc \
    Regex.hh \
    RowPacker.hh \
    RowPacker.icc \
    RPHeaderRecord.hh \
    RPBufferRecord.hh \
    RPBufferReference.hh \
    RPFooterRecord.hh \
    RPFooterReference.hh \
    RPReference.hh \
    SearchSpecifier.hh \
    setIO.hh \
    specialize_hash_io.hh \
    static_check.h \
    streamposIO.hh \
    StringArchive.hh \
    stringArchiveIO.hh \
    StrippedType.hh \
    tupleIO.hh \
    unordered_mapIO.hh \
    unordered_setIO.hh \
    uriUtils.hh \
    valarrayIO.hh \
    vectorIO.hh \
    VarPack.hh \
    VPackIOCycler.hh \
    WriteOnlyCatalog.hh \
    ZlibHandle.hh

EXTRA_DIST = CPP11_config_enable.hh CPP11_config_disable.hh
