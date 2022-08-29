#ifndef BSP_H
#define BSP_H

#include "qpcpp.hpp"

#define BSP_TICKS_PER_SEC   100U

//============================================================================
Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    QS_ASSERTION(module, loc, 10000U); // send assertion info to the QS trace
    qFatal("Assertion failed in module %s, location %d", module, loc);
}

//============================================================================
namespace QP {

//............................................................................
void QF_onClockTick(void) {
    QTimeEvt::TICK_X(0U, &l_time_tick);
}
//............................................................................
void QF::onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC);
    QS_OBJ_DICTIONARY(&l_time_tick);
}
//............................................................................
void QF::onCleanup(void) {
}

#ifdef Q_SPY

#include "qspy.h"

static QTime l_time;

//............................................................................
bool QS::onStartup(void const *) {
    static uint8_t qsBuf[4*1024]; // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    // QS configuration options for this application...
    QSpyConfig const config = {
        QP_VERSION,          // version
        0U,                  // endianness (little)
        QS_OBJ_PTR_SIZE,     // objPtrSize
        QS_FUN_PTR_SIZE,     // funPtrSize
        QS_TIME_SIZE,        // tstampSize
        Q_SIGNAL_SIZE,       // sigSize
        QF_EVENT_SIZ_SIZE,   // evtSize
        QF_EQUEUE_CTR_SIZE,  // queueCtrSize
        QF_MPOOL_CTR_SIZE,   // poolCtrSize
        QF_MPOOL_SIZ_SIZE,   // poolBlkSize
        QF_TIMEEVT_CTR_SIZE, // tevtCtrSize
        { 0U, 0U, 0U, 0U, 0U, 0U} // tstamp
    };

    QSPY_config(&config,
                nullptr,     // no matFile,
                nullptr,     // no seqFile
                nullptr,     // no seqList
                nullptr);    // no custom parser function

    l_time.start();          // start the time stamp

    return true; // success
}
//............................................................................
void QS::onCleanup(void) {
    QSPY_stop();
}
//............................................................................
void QS::onFlush(void) {
    uint16_t nBytes = 1024U;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        QSPY_parse(block, nBytes);
        nBytes = 1024U;
    }
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {
    return (QSTimeCtr)l_time.elapsed();
}

//............................................................................
void QS::onReset(void) {
    //TBD
}
//............................................................................
void QS_onEvent(void) {
    uint16_t nBytes = 1024;
    uint8_t const *block;
    QF_CRIT_ENTRY(dummy);
    if ((block = QS::getBlock(&nBytes)) != (uint8_t *)0) {
        QF_CRIT_EXIT(dummy);
        QSPY_parse(block, nBytes);
    }
    else {
        QF_CRIT_EXIT(dummy);
    }
}
//............................................................................
void QS::onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

//............................................................................
extern "C" void QSPY_onPrintLn(void) {
    qDebug(QSPY_line);
}

#endif // Q_SPY

} // namespace QP
#endif // BSP_H
