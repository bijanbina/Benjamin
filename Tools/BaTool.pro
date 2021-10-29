TEMPLATE = app

QT += dbus
QT += multimedia

CONFIG += console

linux:INCLUDEPATH += /usr/include/glib-2.0 \
                     /usr/lib/glib-2.0/include \
                     ../../Kaldi/src \
                     ../../Kaldi/tools/openfst/src/include \
                     ../../Kaldi/tools/portaudio/install/include \
                     /opt/intel/mkl/include

linux:LIBS += -lgio-2.0 \
              -lgobject-2.0 \
              -lglib-2.0 \
              -pthread \
              -L/opt/intel/mkl/lib/intel64 \
              -lmkl_intel_lp64 -lmkl_sequential -lmkl_core \
              -liomp5 -lpthread -lm -ldl \
              -L/home/bijan/Project/Benjamin/Tools/Kaldi/Libs \
              -lkaldi-online2 -lkaldi-online \
              -lkaldi-decoder -lkaldi-lat -lkaldi-fstext \
              -lkaldi-hmm -lkaldi-feat -lkaldi-transform \
              -lkaldi-gmm  -lkaldi-tree -lkaldi-util \
              -lkaldi-matrix -lkaldi-base -lportaudio -lasound -lrt -ljack -lfst

DEFINES += HAVE_MKL \
           HAVE_CXXABI_H

QMAKE_CXXFLAGS += -std=c++14 -m64 -msse -msse2 -pthread -g -isystem

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

HEADERS += \
    Sources/backend.h \
    Sources/bt_captain.h \
    Sources/bt_channel_l.h \
    Sources/bt_chapar.h \
    Sources/bt_cmvn.h \
    Sources/bt_confidence.h \
    Sources/bt_config.h \
    Sources/bt_cyclic.h \
    Sources/bt_online_source.h \
    Sources/bt_online.h \
    Sources/bt_state.h \
    Sources/kd_f_token.h \
    Sources/kd_faster_decoder.h \
    Sources/kd_lattice_decoder.h \
    Sources/kd_online_decoder.h \
    Sources/kd_online_feinput.h \
    Sources/kd_online_ldecoder.h \
    Sources/kd_online.h \
    Sources/kd_online2_gmm.h \
    Sources/kd_online2.h

SOURCES += \
    Sources/backend.cpp \
    Sources/bt_captain.cpp \
    Sources/bt_channel_l.cpp \
    Sources/bt_chapar.cpp \
    Sources/bt_cmvn.cpp \
    Sources/bt_confidence.cpp \
    Sources/bt_cyclic.cpp \
    Sources/bt_online_source.cpp \
    Sources/bt_online.cpp \
    Sources/bt_state.cpp \
    Sources/kd_f_token.cpp \
    Sources/kd_faster_decoder.cpp \
    Sources/kd_lattice_decoder.cpp \
    Sources/kd_online_decoder.cpp \
    Sources/kd_online_feinput.cpp \
    Sources/kd_online_ldecoder.cpp \
    Sources/kd_online.cpp \
    Sources/kd_online2_gmm.cpp \
    Sources/kd_online2.cpp \
    Sources/main.cpp

