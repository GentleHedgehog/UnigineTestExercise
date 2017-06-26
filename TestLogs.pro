TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
#    precisetime.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
#    precisetime.h

