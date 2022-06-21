TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_LFLAGS += -pthread

SOURCES += \
        device.cpp \
        handler.cpp \
        main.cpp \
        parser.cpp \
        server.cpp

HEADERS += \
	device.h \
	handler.h \
	parser.h \
	server.h
