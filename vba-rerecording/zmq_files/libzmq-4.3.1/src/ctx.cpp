/*
    Copyright (c) 2007-2017 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "precompiled.hpp"
#include "macros.hpp"
#ifndef ZMQ_HAVE_WINDOWS
#include <unistd.h>
#endif

#include <limits>
#include <climits>
#include <new>
#include <sstream>
#include <string.h>

#include "ctx.hpp"
#include "socket_base.hpp"
#include "io_thread.hpp"
#include "reaper.hpp"
#include "pipe.hpp"
#include "err.hpp"
#include "msg.hpp"
#include "random.hpp"

#ifdef ZMQ_HAVE_VMCI
#include <vmci_sockets.h>
#endif

#define ZMQ_CTX_TAG_VALUE_GOOD 0xabadcafe
#define ZMQ_CTX_TAG_VALUE_BAD 0xdeadbeef

int clipped_maxsocket (int max_requested_)
{
    if (max_requested_ >= zmq::poller_t::max_fds ()
        && zmq::poller_t::max_fds () != -1)
        // -1 because we need room for the reaper mailbox.
        max_requested_ = zmq::poller_t::max_fds () - 1;

    return max_requested_;
}

zmq::ctx_t::ctx_t () :
    _tag (ZMQ_CTX_TAG_VALUE_GOOD),
    _starting (true),
    _terminating (false),
    _reaper (NULL),
    _max_sockets (clipped_maxsocket (ZMQ_MAX_SOCKETS_DFLT)),
    _max_msgsz (INT_MAX),
    _io_thread_count (ZMQ_IO_THREADS_DFLT),
    _blocky (true),
    _ipv6 (false),
    _zero_copy (true)
{
#ifdef HAVE_FORK
    _pid = getpid ();
#endif
#ifdef ZMQ_HAVE_VMCI
    _vmci_fd = -1;
    _vmci_family = -1;
#endif

    //  Initialise crypto library, if needed.
    zmq::random_open ();
}

bool zmq::ctx_t::check_tag ()
{
    return _tag == ZMQ_CTX_TAG_VALUE_GOOD;
}

zmq::ctx_t::~ctx_t ()
{
    //  Check that there are no remaining _sockets.
    zmq_assert (_sockets.empty ());

    //  Ask I/O threads to terminate. If stop signal wasn't sent to I/O
    //  thread subsequent invocation of destructor would hang-up.
    for (io_threads_t::size_type i = 0; i != _io_threads.size (); i++) {
        _io_threads[i]->stop ();
    }

    //  Wait till I/O threads actually terminate.
    for (io_threads_t::size_type i = 0; i != _io_threads.size (); i++) {
        LIBZMQ_DELETE (_io_threads[i]);
    }

    //  Deallocate the reaper thread object.
    LIBZMQ_DELETE (_reaper);

    //  The mailboxes in _slots themselves were deallocated with their
    //  corresponding io_thread/socket objects.

    //  De-initialise crypto library, if needed.
    zmq::random_close ();

    //  Remove the tag, so that the object is considered dead.
    _tag = ZMQ_CTX_TAG_VALUE_BAD;
}

bool zmq::ctx_t::valid () const
{
    return _term_mailbox.valid ();
}

int zmq::ctx_t::terminate ()
{
    _slot_sync.lock ();

    bool save_terminating = _terminating;
    _terminating = false;

    // Connect up any pending inproc connections, otherwise we will hang
    pending_connections_t copy = _pending_connections;
    for (pending_connections_t::iterator p = copy.begin (), end = copy.end ();
         p != end; ++p) {
        zmq::socket_base_t *s = create_socket (ZMQ_PAIR);
        // create_socket might fail eg: out of memory/sockets limit reached
        zmq_assert (s);
        s->bind (p->first.c_str ());
        s->close ();
    }
    _terminating = save_terminating;

    if (!_starting) {
#ifdef HAVE_FORK
        if (_pid != getpid ()) {
            // we are a forked child process. Close all file descriptors
            // inherited from the parent.
            for (sockets_t::size_type i = 0; i != _sockets.size (); i++)
                _sockets[i]->get_mailbox ()->forked ();

            _term_mailbox.forked ();
        }
#endif

        //  Check whether termination was already underway, but interrupted and now
        //  restarted.
        bool restarted = _terminating;
        _terminating = true;

        //  First attempt to terminate the context.
        if (!restarted) {
            //  First send stop command to sockets so that any blocking calls
            //  can be interrupted. If there are no sockets we can ask reaper
            //  thread to stop.
            for (sockets_t::size_type i = 0; i != _sockets.size (); i++)
                _sockets[i]->stop ();
            if (_sockets.empty ())
                _reaper->stop ();
        }
        _slot_sync.unlock ();

        //  Wait till reaper thread closes all the sockets.
        command_t cmd;
        int rc = _term_mailbox.recv (&cmd, -1);
        if (rc == -1 && errno == EINTR)
            return -1;
        errno_assert (rc == 0);
        zmq_assert (cmd.type == command_t::done);
        _slot_sync.lock ();
        zmq_assert (_sockets.empty ());
    }
    _slot_sync.unlock ();

#ifdef ZMQ_HAVE_VMCI
    _vmci_sync.lock ();

    VMCISock_ReleaseAFValueFd (_vmci_fd);
    _vmci_family = -1;
    _vmci_fd = -1;

    _vmci_sync.unlock ();
#endif

    //  Deallocate the resources.
    delete this;

    return 0;
}

int zmq::ctx_t::shutdown ()
{
    scoped_lock_t locker (_slot_sync);

    if (!_starting && !_terminating) {
        _terminating = true;

        //  Send stop command to sockets so that any blocking calls
        //  can be interrupted. If there are no sockets we can ask reaper
        //  thread to stop.
        for (sockets_t::size_type i = 0; i != _sockets.size (); i++)
            _sockets[i]->stop ();
        if (_sockets.empty ())
            _reaper->stop ();
    }

    return 0;
}

int zmq::ctx_t::set (int option_, int optval_)
{
    int rc = 0;
    if (option_ == ZMQ_MAX_SOCKETS && optval_ >= 1
        && optval_ == clipped_maxsocket (optval_)) {
        scoped_lock_t locker (_opt_sync);
        _max_sockets = optval_;
    } else if (option_ == ZMQ_IO_THREADS && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _io_thread_count = optval_;
    } else if (option_ == ZMQ_IPV6 && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _ipv6 = (optval_ != 0);
    } else if (option_ == ZMQ_BLOCKY && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _blocky = (optval_ != 0);
    } else if (option_ == ZMQ_MAX_MSGSZ && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _max_msgsz = optval_ < INT_MAX ? optval_ : INT_MAX;
    } else if (option_ == ZMQ_ZERO_COPY_RECV && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _zero_copy = (optval_ != 0);
    } else {
        rc = thread_ctx_t::set (option_, optval_);
    }
    return rc;
}

int zmq::ctx_t::get (int option_)
{
    int rc = 0;
    if (option_ == ZMQ_MAX_SOCKETS)
        rc = _max_sockets;
    else if (option_ == ZMQ_SOCKET_LIMIT)
        rc = clipped_maxsocket (65535);
    else if (option_ == ZMQ_IO_THREADS)
        rc = _io_thread_count;
    else if (option_ == ZMQ_IPV6)
        rc = _ipv6;
    else if (option_ == ZMQ_BLOCKY)
        rc = _blocky;
    else if (option_ == ZMQ_MAX_MSGSZ)
        rc = _max_msgsz;
    else if (option_ == ZMQ_MSG_T_SIZE)
        rc = sizeof (zmq_msg_t);
    else if (option_ == ZMQ_ZERO_COPY_RECV) {
        rc = _zero_copy;
    } else {
        rc = thread_ctx_t::get (option_);
    }
    return rc;
}

bool zmq::ctx_t::start ()
{
    //  Initialise the array of mailboxes. Additional two slots are for
    //  zmq_ctx_term thread and reaper thread.
    _opt_sync.lock ();
    const int term_and_reaper_threads_count = 2;
    const int mazmq = _max_sockets;
    const int ios = _io_thread_count;
    _opt_sync.unlock ();
    int slot_count = mazmq + ios + term_and_reaper_threads_count;
    try {
        _slots.reserve (slot_count);
        _empty_slots.reserve (slot_count - term_and_reaper_threads_count);
    }
    catch (const std::bad_alloc &) {
        errno = ENOMEM;
        return false;
    }
    _slots.resize (term_and_reaper_threads_count);

    //  Initialise the infrastructure for zmq_ctx_term thread.
    _slots[term_tid] = &_term_mailbox;

    //  Create the reaper thread.
    _reaper = new (std::nothrow) reaper_t (this, reaper_tid);
    if (!_reaper) {
        errno = ENOMEM;
        goto fail_cleanup_slots;
    }
    if (!_reaper->get_mailbox ()->valid ())
        goto fail_cleanup_reaper;
    _slots[reaper_tid] = _reaper->get_mailbox ();
    _reaper->start ();

    //  Create I/O thread objects and launch them.
    _slots.resize (slot_count, NULL);

    for (int i = term_and_reaper_threads_count;
         i != ios + term_and_reaper_threads_count; i++) {
        io_thread_t *io_thread = new (std::nothrow) io_thread_t (this, i);
        if (!io_thread) {
            errno = ENOMEM;
            goto fail_cleanup_reaper;
        }
        if (!io_thread->get_mailbox ()->valid ()) {
            delete io_thread;
            goto fail_cleanup_reaper;
        }
        _io_threads.push_back (io_thread);
        _slots[i] = io_thread->get_mailbox ();
        io_thread->start ();
    }

    //  In the unused part of the slot array, create a list of empty slots.
    for (int32_t i = static_cast<int32_t> (_slots.size ()) - 1;
         i >= static_cast<int32_t> (ios) + term_and_reaper_threads_count; i--) {
        _empty_slots.push_back (i);
    }

    _starting = false;
    return true;

fail_cleanup_reaper:
    _reaper->stop ();
    delete _reaper;
    _reaper = NULL;

fail_cleanup_slots:
    _slots.clear ();
    return false;
}

zmq::socket_base_t *zmq::ctx_t::create_socket (int type_)
{
    scoped_lock_t locker (_slot_sync);

    if (unlikely (_starting)) {
        if (!start ())
            return NULL;
    }

    //  Once zmq_ctx_term() was called, we can't create new sockets.
    if (_terminating) {
        errno = ETERM;
        return NULL;
    }

    //  If max_sockets limit was reached, return error.
    if (_empty_slots.empty ()) {
        errno = EMFILE;
        return NULL;
    }

    //  Choose a slot for the socket.
    uint32_t slot = _empty_slots.back ();
    _empty_slots.pop_back ();

    //  Generate new unique socket ID.
    int sid = (static_cast<int> (max_socket_id.add (1))) + 1;

    //  Create the socket and register its mailbox.
    socket_base_t *s = socket_base_t::create (type_, this, slot, sid);
    if (!s) {
        _empty_slots.push_back (slot);
        return NULL;
    }
    _sockets.push_back (s);
    _slots[slot] = s->get_mailbox ();

    return s;
}

void zmq::ctx_t::destroy_socket (class socket_base_t *socket_)
{
    scoped_lock_t locker (_slot_sync);

    //  Free the associated thread slot.
    uint32_t tid = socket_->get_tid ();
    _empty_slots.push_back (tid);
    _slots[tid] = NULL;

    //  Remove the socket from the list of sockets.
    _sockets.erase (socket_);

    //  If zmq_ctx_term() was already called and there are no more socket
    //  we can ask reaper thread to terminate.
    if (_terminating && _sockets.empty ())
        _reaper->stop ();
}

zmq::object_t *zmq::ctx_t::get_reaper ()
{
    return _reaper;
}

zmq::thread_ctx_t::thread_ctx_t () :
    _thread_priority (ZMQ_THREAD_PRIORITY_DFLT),
    _thread_sched_policy (ZMQ_THREAD_SCHED_POLICY_DFLT)
{
}

void zmq::thread_ctx_t::start_thread (thread_t &thread_,
                                      thread_fn *tfn_,
                                      void *arg_) const
{
    static unsigned int nthreads_started = 0;

    thread_.setSchedulingParameters (_thread_priority, _thread_sched_policy,
                                     _thread_affinity_cpus);
    thread_.start (tfn_, arg_);
#ifndef ZMQ_HAVE_ANDROID
    std::ostringstream s;
    if (!_thread_name_prefix.empty ())
        s << _thread_name_prefix << "/";
    s << "ZMQbg/" << nthreads_started;
    thread_.setThreadName (s.str ().c_str ());
#endif
    nthreads_started++;
}

int zmq::thread_ctx_t::set (int option_, int optval_)
{
    int rc = 0;
    if (option_ == ZMQ_THREAD_SCHED_POLICY && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _thread_sched_policy = optval_;
    } else if (option_ == ZMQ_THREAD_AFFINITY_CPU_ADD && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _thread_affinity_cpus.insert (optval_);
    } else if (option_ == ZMQ_THREAD_AFFINITY_CPU_REMOVE && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        if (0 == _thread_affinity_cpus.erase (optval_)) {
            errno = EINVAL;
            rc = -1;
        }
    } else if (option_ == ZMQ_THREAD_NAME_PREFIX && optval_ >= 0) {
        std::ostringstream s;
        s << optval_;
        scoped_lock_t locker (_opt_sync);
        _thread_name_prefix = s.str ();
    } else if (option_ == ZMQ_THREAD_PRIORITY && optval_ >= 0) {
        scoped_lock_t locker (_opt_sync);
        _thread_priority = optval_;
    } else {
        errno = EINVAL;
        rc = -1;
    }
    return rc;
}

int zmq::thread_ctx_t::get (int option_)
{
    int rc = 0;
    if (option_ == ZMQ_THREAD_PRIORITY) {
        scoped_lock_t locker (_opt_sync);
        rc = _thread_priority;
    } else if (option_ == ZMQ_THREAD_SCHED_POLICY) {
        scoped_lock_t locker (_opt_sync);
        rc = _thread_sched_policy;
    } else if (option_ == ZMQ_THREAD_NAME_PREFIX) {
        scoped_lock_t locker (_opt_sync);
        rc = atoi (_thread_name_prefix.c_str ());
    } else {
        errno = EINVAL;
        rc = -1;
    }
    return rc;
}

void zmq::ctx_t::send_command (uint32_t tid_, const command_t &command_)
{
    _slots[tid_]->send (command_);
}

zmq::io_thread_t *zmq::ctx_t::choose_io_thread (uint64_t affinity_)
{
    if (_io_threads.empty ())
        return NULL;

    //  Find the I/O thread with minimum load.
    int min_load = -1;
    io_thread_t *selected_io_thread = NULL;
    for (io_threads_t::size_type i = 0; i != _io_threads.size (); i++) {
        if (!affinity_ || (affinity_ & (uint64_t (1) << i))) {
            int load = _io_threads[i]->get_load ();
            if (selected_io_thread == NULL || load < min_load) {
                min_load = load;
                selected_io_thread = _io_threads[i];
            }
        }
    }
    return selected_io_thread;
}

int zmq::ctx_t::register_endpoint (const char *addr_,
                                   const endpoint_t &endpoint_)
{
    scoped_lock_t locker (_endpoints_sync);

    const bool inserted =
      _endpoints.ZMQ_MAP_INSERT_OR_EMPLACE (std::string (addr_), endpoint_)
        .second;
    if (!inserted) {
        errno = EADDRINUSE;
        return -1;
    }
    return 0;
}

int zmq::ctx_t::unregister_endpoint (const std::string &addr_,
                                     socket_base_t *socket_)
{
    scoped_lock_t locker (_endpoints_sync);

    const endpoints_t::iterator it = _endpoints.find (addr_);
    if (it == _endpoints.end () || it->second.socket != socket_) {
        errno = ENOENT;
        return -1;
    }

    //  Remove endpoint.
    _endpoints.erase (it);

    return 0;
}

void zmq::ctx_t::unregister_endpoints (socket_base_t *socket_)
{
    scoped_lock_t locker (_endpoints_sync);

    for (endpoints_t::iterator it = _endpoints.begin (),
                               end = _endpoints.end ();
         it != end;) {
        if (it->second.socket == socket_)
#if __cplusplus >= 201103L
            it = _endpoints.erase (it);
#else
            _endpoints.erase (it++);
#endif
        else
            ++it;
    }
}

zmq::endpoint_t zmq::ctx_t::find_endpoint (const char *addr_)
{
    scoped_lock_t locker (_endpoints_sync);

    endpoints_t::iterator it = _endpoints.find (addr_);
    if (it == _endpoints.end ()) {
        errno = ECONNREFUSED;
        endpoint_t empty = {NULL, options_t ()};
        return empty;
    }
    endpoint_t endpoint = it->second;

    //  Increment the command sequence number of the peer so that it won't
    //  get deallocated until "bind" command is issued by the caller.
    //  The subsequent 'bind' has to be called with inc_seqnum parameter
    //  set to false, so that the seqnum isn't incremented twice.
    endpoint.socket->inc_seqnum ();

    return endpoint;
}

void zmq::ctx_t::pend_connection (const std::string &addr_,
                                  const endpoint_t &endpoint_,
                                  pipe_t **pipes_)
{
    scoped_lock_t locker (_endpoints_sync);

    const pending_connection_t pending_connection = {endpoint_, pipes_[0],
                                                     pipes_[1]};

    endpoints_t::iterator it = _endpoints.find (addr_);
    if (it == _endpoints.end ()) {
        //  Still no bind.
        endpoint_.socket->inc_seqnum ();
        _pending_connections.ZMQ_MAP_INSERT_OR_EMPLACE (addr_,
                                                        pending_connection);
    } else {
        //  Bind has happened in the mean time, connect directly
        connect_inproc_sockets (it->second.socket, it->second.options,
                                pending_connection, connect_side);
    }
}

void zmq::ctx_t::connect_pending (const char *addr_,
                                  zmq::socket_base_t *bind_socket_)
{
    scoped_lock_t locker (_endpoints_sync);

    std::pair<pending_connections_t::iterator, pending_connections_t::iterator>
      pending = _pending_connections.equal_range (addr_);
    for (pending_connections_t::iterator p = pending.first; p != pending.second;
         ++p)
        connect_inproc_sockets (bind_socket_, _endpoints[addr_].options,
                                p->second, bind_side);

    _pending_connections.erase (pending.first, pending.second);
}

void zmq::ctx_t::connect_inproc_sockets (
  zmq::socket_base_t *bind_socket_,
  options_t &bind_options_,
  const pending_connection_t &pending_connection_,
  side side_)
{
    bind_socket_->inc_seqnum ();
    pending_connection_.bind_pipe->set_tid (bind_socket_->get_tid ());

    if (!bind_options_.recv_routing_id) {
        msg_t msg;
        const bool ok = pending_connection_.bind_pipe->read (&msg);
        zmq_assert (ok);
        const int rc = msg.close ();
        errno_assert (rc == 0);
    }

    if (!get_effective_conflate_option (pending_connection_.endpoint.options)) {
        pending_connection_.connect_pipe->set_hwms_boost (bind_options_.sndhwm,
                                                          bind_options_.rcvhwm);
        pending_connection_.bind_pipe->set_hwms_boost (
          pending_connection_.endpoint.options.sndhwm,
          pending_connection_.endpoint.options.rcvhwm);

        pending_connection_.connect_pipe->set_hwms (
          pending_connection_.endpoint.options.rcvhwm,
          pending_connection_.endpoint.options.sndhwm);
        pending_connection_.bind_pipe->set_hwms (bind_options_.rcvhwm,
                                                 bind_options_.sndhwm);
    } else {
        pending_connection_.connect_pipe->set_hwms (-1, -1);
        pending_connection_.bind_pipe->set_hwms (-1, -1);
    }

    if (side_ == bind_side) {
        command_t cmd;
        cmd.type = command_t::bind;
        cmd.args.bind.pipe = pending_connection_.bind_pipe;
        bind_socket_->process_command (cmd);
        bind_socket_->send_inproc_connected (
          pending_connection_.endpoint.socket);
    } else
        pending_connection_.connect_pipe->send_bind (
          bind_socket_, pending_connection_.bind_pipe, false);

    // When a ctx is terminated all pending inproc connection will be
    // connected, but the socket will already be closed and the pipe will be
    // in waiting_for_delimiter state, which means no more writes can be done
    // and the routing id write fails and causes an assert. Check if the socket
    // is open before sending.
    if (pending_connection_.endpoint.options.recv_routing_id
        && pending_connection_.endpoint.socket->check_tag ()) {
        send_routing_id (pending_connection_.bind_pipe, bind_options_);
    }
}

#ifdef ZMQ_HAVE_VMCI

int zmq::ctx_t::get_vmci_socket_family ()
{
    zmq::scoped_lock_t locker (_vmci_sync);

    if (_vmci_fd == -1) {
        _vmci_family = VMCISock_GetAFValueFd (&_vmci_fd);

        if (_vmci_fd != -1) {
#ifdef FD_CLOEXEC
            int rc = fcntl (_vmci_fd, F_SETFD, FD_CLOEXEC);
            errno_assert (rc != -1);
#endif
        }
    }

    return _vmci_family;
}

#endif

//  The last used socket ID, or 0 if no socket was used so far. Note that this
//  is a global variable. Thus, even sockets created in different contexts have
//  unique IDs.
zmq::atomic_counter_t zmq::ctx_t::max_socket_id;