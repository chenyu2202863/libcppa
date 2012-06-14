/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011, 2012                                                   *
 * Dominik Charousset <dominik.charousset@haw-hamburg.de>                     *
 *                                                                            *
 * This file is part of libcppa.                                              *
 * libcppa is free software: you can redistribute it and/or modify it under   *
 * the terms of the GNU Lesser General Public License as published by the     *
 * Free Software Foundation, either version 3 of the License                  *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * libcppa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with libcppa. If not, see <http://www.gnu.org/licenses/>.            *
\******************************************************************************/


#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "cppa/actor.hpp"
#include "cppa/behavior.hpp"
#include "cppa/any_tuple.hpp"
#include "cppa/exit_reason.hpp"
#include "cppa/partial_function.hpp"
#include "cppa/intrusive/single_reader_queue.hpp"

namespace cppa {

class scheduler;

/**
 * @brief Base class for local running Actors.
 */
class local_actor : public actor {

    friend class scheduler;

 protected:

    bool m_chaining;
    bool m_trap_exit;
    bool m_is_scheduled;
    actor_ptr m_chained;
    actor_ptr m_last_sender;
    any_tuple m_last_dequeued;

 public:

    local_actor(bool is_scheduled = false);

    /**
     * @brief Finishes execution of this actor.
     *
     * Causes this actor to send an exit signal to all of its
     * linked actors, sets its state to @c exited and throws
     * {@link actor_exited} to cleanup the stack.
     * @param reason Exit reason that will be send to linked actors.
     * @throws actor_exited
     */
    virtual void quit(std::uint32_t reason) = 0;

    inline void quit_normal() {
        quit(exit_reason::normal);
    }

    /**
     * @brief
     * @param rules
     * @warning Call only from the owner of the queue.
     */
    virtual void dequeue(behavior& rules) = 0;

    /**
     * @brief Removes the first element from the queue that is matched
     *        by @p rules and invokes the corresponding callback.
     * @param rules
     * @warning Call only from the owner of the queue.
     */
    virtual void dequeue(partial_function& rules) = 0;

    inline bool trap_exit() const {
        return m_trap_exit;
    }

    inline void trap_exit(bool new_value) {
        m_trap_exit = new_value;
    }

    inline bool chaining() const {
        return m_chaining;
    }

    inline void chaining(bool new_value) {
        if (m_is_scheduled) {
            m_chaining = new_value;
        }
    }

    inline any_tuple& last_dequeued() {
        return m_last_dequeued;
    }

    inline actor_ptr& last_sender() {
        return m_last_sender;
    }

    inline actor_ptr& chained_actor() {
        return m_chained;
    }

    inline void send_message(channel* whom, any_tuple what) {
        whom->enqueue(this, std::move(what));
    }

    inline void send_message(actor* whom, any_tuple what) {
        if (m_chaining && !m_chained) {
            if (whom->chained_enqueue(this, std::move(what))) {
                m_chained = whom;
            }
        }
        else {
            whom->enqueue(this, std::move(what));
        }
    }

    /**
     * @ingroup ActorManagement
     * @brief Adds a unidirectional @p monitor to @p whom.
     * @param whom The actor that should be monitored by this actor.
     * @note Each call to @p monitor creates a new, independent monitor.
     * @pre The calling actor receives a "DOWN" message from @p whom when
     *      it terminates.
     */
    void monitor(actor_ptr whom);

    /**
     * @ingroup ActorManagement
     * @brief Removes a monitor from @p whom.
     * @param whom A monitored actor.
     */
    void demonitor(actor_ptr whom);

};

/**
 * @brief A smart pointer to a {@link local_actor} instance.
 * @relates local_actor
 */
typedef intrusive_ptr<local_actor> local_actor_ptr;

} // namespace cppa

#endif // CONTEXT_HPP
