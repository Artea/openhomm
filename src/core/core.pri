INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
contains(QT_CONFIG, zlib) { 
    INCLUDEPATH += $$(QTDIR)/src/3rdparty/zlib
    DEPENDPATH += $$(QTDIR)/src/3rdparty/zlib
}
HEADERS += hrString.hpp \
    hrLodEngine.hpp \
    hrLodEngineIterator.hpp \
    hrSndEngineIterator.hpp \
    hrSndEngine.hpp \
    hrPlatform.hpp \
    hrSettings.hpp \
    hrFilesystem.hpp \
    hrFileEngineHandlers.hpp \
    hrMessageOutput.hpp
SOURCES += hrString.cpp \
    hrLodEngine.cpp \
    hrLodEngineIterator.cpp \
    hrSndEngineIterator.cpp \
    hrSndEngine.cpp \
    hrSettings.cpp \
    hrFilesystem.cpp \
    hrFileEngineHandlers.cpp \
    hrMessageOutput.cpp
