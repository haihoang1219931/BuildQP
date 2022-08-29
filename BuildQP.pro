#-----------------------------------------------------------------------------
# Product: DPP console exampe for Qt (console)
# Last updated for version 6.8.1
# Last updated on  2020-04-04
#
#                    Q u a n t u m  L e a P s
#                    ------------------------
#                    Modern Embedded Software
#
# Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Alternatively, this program may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GNU General Public License and are specifically designed for
# licensees interested in retaining the proprietary status of their code.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <www.gnu.org/licenses>.
#
# Contact information:
# <www.state-machine.com/licensing>
# <info@state-machine.com>
#-----------------------------------------------------------------------------
QT      += core
QT      -= gui
CONFIG  += console
CONFIG  += c++11
DEFINES += QT_NO_STATEMACHINE

QPCPP = $$PWD/qp

INCLUDEPATH = . \
    $$QPCPP/include \
    $$QPCPP/ports/qt

SOURCES += \
    main.cpp \
    blinky.cpp
# QP-Qt port headers/sources
HEADERS +=  \
    $$QPCPP/ports/qt/tickerthread.hpp \
    $$QPCPP/ports/qt/aothread.hpp \
    blinky.hpp \
    bsp.h
SOURCES += \
    $$QPCPP/ports/qt/qf_port.cpp


# QP/C++ headers/sources
SOURCES += \
    $$QPCPP/src/qf/qep_hsm.cpp \
    $$QPCPP/src/qf/qep_msm.cpp \
    $$QPCPP/src/qf/qf_act.cpp \
    $$QPCPP/src/qf/qf_actq.cpp \
    $$QPCPP/src/qf/qf_defer.cpp \
    $$QPCPP/src/qf/qf_dyn.cpp \
    $$QPCPP/src/qf/qf_mem.cpp \
    $$QPCPP/src/qf/qf_ps.cpp \
    $$QPCPP/src/qf/qf_qact.cpp \
    $$QPCPP/src/qf/qf_qeq.cpp \
    $$QPCPP/src/qf/qf_qmact.cpp \
    $$QPCPP/src/qf/qf_time.cpp \
    $$QPCPP/include/qstamp.cpp

INCLUDEPATH += $$QPCPP/src


CONFIG(debug, debug|release) {
} else {
    DEFINES += NDEBUG
}
