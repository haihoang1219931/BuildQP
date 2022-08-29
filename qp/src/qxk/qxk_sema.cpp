//$file${src::qxk::qxk_sema.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qxk::qxk_sema.cpp}
//
// This code has been generated by QM 5.2.1 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2023-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${src::qxk::qxk_sema.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @file
//! @brief QXK/C++ preemptive kernel counting semaphore implementation

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope internal interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QXK_HPP
#error "Source file included in a project NOT based on the QXK kernel"
#endif // QXK_HPP

namespace { // unnamed local namespace
Q_DEFINE_THIS_MODULE("qxk_sema")
} // unnamed namespace

//============================================================================
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QXK::QXSemaphore} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QXK::QXSemaphore} ........................................................

//${QXK::QXSemaphore::init} ..................................................
void QXSemaphore::init(
    std::uint_fast16_t const count,
    std::uint_fast16_t const max_count) noexcept
{
    //! @pre max_count must be greater than zero
    Q_REQUIRE_ID(100, max_count > 0U);

    m_count     = static_cast<std::uint16_t>(count);
    m_max_count = static_cast<std::uint16_t>(max_count);
    m_waitSet.setEmpty();
}

//${QXK::QXSemaphore::wait} ..................................................
bool QXSemaphore::wait(std::uint_fast16_t const nTicks) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    // volatile into temp.
    QXThread * const curr = QXK_PTR_CAST_(QXThread*, QXK_attr_.curr);

    //! @pre this function must:
    //! - NOT be called from an ISR;
    //! - the semaphore must be initialized
    //! - be called from an extended thread;
    //! - the thread must NOT be already blocked on any object.
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_()) // can't wait inside an ISR
        && (m_max_count > 0U)
        && (curr != nullptr)
        && (curr->m_temp.obj == nullptr));
    //! @pre also: the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_ID(201, QXK_attr_.lockHolder != curr->m_prio);

    bool signaled = true; // assume that the semaphore will be signaled
    if (m_count > 0U) {
        m_count = m_count - 1U; // semaphore taken: decrement

        QS_BEGIN_NOCRIT_PRE_(QS_SEM_TAKE, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            QS_OBJ_PRE_(this); // this semaphore
            QS_2U8_PRE_(curr->m_prio, m_count);
        QS_END_NOCRIT_PRE_()
    }
    else {
        std::uint_fast8_t const p =
            static_cast<std::uint_fast8_t>(curr->m_prio);
        // remove the curr prio from the ready set (will block)
        // and insert to the waiting set on this semaphore
        QF::readySet_.remove(p);
        m_waitSet.insert(p);

        // remember the blocking object (this semaphore)
        curr->m_temp.obj = QXK_PTR_CAST_(QMState*, this);
        curr->teArm_(static_cast<enum_t>(QXK::SEMA_SIG), nTicks);

        QS_BEGIN_NOCRIT_PRE_(QS_SEM_BLOCK, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            QS_OBJ_PRE_(this); // this semaphore
            QS_2U8_PRE_(curr->m_prio, m_count);
        QS_END_NOCRIT_PRE_()

        // schedule the next thread if multitasking started
        static_cast<void>(QXK_sched_());
        QF_CRIT_X_();
        QF_CRIT_EXIT_NOP(); // BLOCK here !!!

        QF_CRIT_E_();   // AFTER unblocking...
        // the blocking object must be this semaphore
        Q_ASSERT_ID(240, curr->m_temp.obj
                         == QXK_PTR_CAST_(QMState*, this));

        // did the blocking time-out? (signal of zero means that it did)
        if (curr->m_timeEvt.sig == 0U) {
            if (m_waitSet.hasElement(p)) { // still waiting?
                m_waitSet.remove(p); // remove unblocked thread
                signaled = false; // the semaphore was NOT signaled
                // semaphore NOT taken: do NOT decrement the count
            }
            else { // semaphore was both signaled and timed out
                m_count = m_count - 1U; // semaphore taken: decrement
            }
        }
        else { // blocking did NOT time out
            // the thread must NOT be waiting on this semaphore
            Q_ASSERT_ID(250, !m_waitSet.hasElement(p));
            m_count = m_count - 1U; // semaphore taken: decrement
        }
        curr->m_temp.obj = nullptr; // clear blocking obj.
    }
    QF_CRIT_X_();

    return signaled;
}

//${QXK::QXSemaphore::tryWait} ...............................................
bool QXSemaphore::tryWait() noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    //! @pre the semaphore must be initialized
    Q_REQUIRE_ID(300, m_max_count > 0U);

    #ifdef Q_SPY
    // volatile into temp.
    QActive const * const curr = QXK_PTR_CAST_(QActive*, QXK_attr_.curr);
    #endif // Q_SPY

    bool isAvailable;
    // is the semaphore available?
    if (m_count > 0U) {
        m_count = m_count - 1U; // semaphore signaled: decrement
        isAvailable = true;

        QS_BEGIN_NOCRIT_PRE_(QS_SEM_TAKE, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            QS_OBJ_PRE_(this); // this semaphore
            QS_2U8_PRE_(curr->m_prio, m_count);
        QS_END_NOCRIT_PRE_()
    }
    else { // the semaphore is NOT available (would block)
        isAvailable = false;

        QS_BEGIN_NOCRIT_PRE_(QS_SEM_BLOCK_ATTEMPT, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            QS_OBJ_PRE_(this); // this semaphore
            QS_2U8_PRE_(curr->m_prio,
                        m_count);
        QS_END_NOCRIT_PRE_()
    }
    QF_CRIT_X_();

    return isAvailable;
}

//${QXK::QXSemaphore::signal} ................................................
bool QXSemaphore::signal() noexcept {
    //! @pre the semaphore must be initialized
    Q_REQUIRE_ID(400, m_max_count > 0U);

    QF_CRIT_STAT_
    QF_CRIT_E_();

    bool signaled = true; // assume that the semaphore will be signaled
    if (m_count < m_max_count) {

        m_count = m_count + 1U; // semaphore signaled: increment

        #ifdef Q_SPY
        // volatile into temp.
        QActive const * const curr = QXK_PTR_CAST_(QActive*, QXK_attr_.curr);
        #endif // Q_SPY

        QS_BEGIN_NOCRIT_PRE_(QS_SEM_SIGNAL, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            QS_OBJ_PRE_(this); // this semaphore
            QS_2U8_PRE_(curr->m_prio,
                        m_count);
        QS_END_NOCRIT_PRE_()

        if (m_waitSet.notEmpty()) {
            // find the highest-priority thread waiting on this semaphore
            std::uint_fast8_t const p = m_waitSet.findMax();
            QXThread * const thr =
                QXK_PTR_CAST_(QXThread*, QActive::registry_[p]);

            // assert that the tread:
            // - must be registered in QF;
            // - must be extended; and
            // - must be blocked on this semaphore;
            Q_ASSERT_ID(410, (thr != nullptr)
                && (thr->m_osObject != nullptr)
                && (thr->m_temp.obj
                    == QXK_PTR_CAST_(QMState*, this)));

            // disarm the internal time event
            static_cast<void>(thr->teDisarm_());

            // make the thread ready to run and remove from the wait-list
            QF::readySet_.insert(p);
            m_waitSet.remove(p);

            QS_BEGIN_NOCRIT_PRE_(QS_SEM_TAKE, thr->m_prio)
                QS_TIME_PRE_();  // timestamp
                QS_OBJ_PRE_(this); // this semaphore
                QS_2U8_PRE_(thr->m_prio, m_count);
            QS_END_NOCRIT_PRE_()

            if (!QXK_ISR_CONTEXT_()) { // not inside ISR?
                static_cast<void>(QXK_sched_()); // schedule the next thread
            }
        }
    }
    else {
        signaled = false; // semaphore NOT signaled
    }
    QF_CRIT_X_();

    return signaled;
}

} // namespace QP
//$enddef${QXK::QXSemaphore} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
