#include <iostream>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "builtin/io.hpp"
#include "builtin/bytearray.hpp"
#include "builtin/channel.hpp"
#include "builtin/class.hpp"
#include "builtin/exception.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/string.hpp"
#include "builtin/bytearray.hpp"
#include "primitives.hpp"

#include "vm/event.hpp"

#include "vm.hpp"
#include "objectmemory.hpp"

#include "vm/object_utils.hpp"

#include "native_thread.hpp"

namespace rubinius {
  void IO::init(STATE) {
    GO(io).set(state->new_class("IO", G(object)));
    G(io)->set_object_type(state, IOType);

    GO(iobuffer).set(state->new_class("Buffer", G(object), G(io)));
    G(iobuffer)->set_object_type(state, IOBufferType);
  }

  IO* IO::create(STATE, int fd) {
    IO* io = state->new_object<IO>(G(io));
    io->descriptor(state, Fixnum::from(fd));
    io->set_mode(state);
    io->ibuffer(state, IOBuffer::create(state));
    io->eof(state, Qfalse);
    io->lineno(state, Fixnum::from(0));
    return io;
  }

  IO* IO::allocate(STATE, Object* self) {
    IO* io = state->new_object<IO>(G(io));
    io->descriptor(state, (Fixnum*)Qnil);
    io->mode(state, (Fixnum*)Qnil);
    io->ibuffer(state, IOBuffer::create(state));
    io->eof(state, Qfalse);
    io->lineno(state, Fixnum::from(0));

    // Ensure the instance's class is set (i.e. for subclasses of IO)
    io->klass(state, as<Class>(self));
    return io;
  }

  Fixnum* IO::open(STATE, String* path, Fixnum* mode, Fixnum* perm) {
    int fd = ::open(path->c_str(), mode->to_native(), perm->to_native());
    return Fixnum::from(fd);
  }


  namespace {
    /** Utility function used by IO::select, returns highest descriptor. */
    static inline native_int hidden_fd_set_from_array(VM* state, Object* maybe_descriptors, fd_set* set) {
      if(NULL == set) {
        return 0;
      }

      Array* descriptors = as<Array>(maybe_descriptors);

      FD_ZERO(set);
      native_int highest = 0;

      for(std::size_t i = 0; i < descriptors->size(); ++i) {
        native_int descriptor = as<IO>(descriptors->get(state, i))->to_fd();
        highest = descriptor > highest ? descriptor : highest;

        FD_SET(descriptor, set);
      }

      return highest;
    }

    /** Utility function used by IO::select, returns Array of IOs that were set. */
    static inline Array* hidden_reject_unset_fds(VM* state, Object* maybe_originals, fd_set* set) {
      if(NULL == set) {
        return Array::create(state, 0);
      }

      Array* originals = as<Array>(maybe_originals);
      Array* selected = Array::create(state, originals->size());

      for(std::size_t i = 0; i < originals->size(); ++i) {
        IO* io = as<IO>(originals->get(state, i));

        if(FD_ISSET(io->to_fd(), set)) {
          selected->set(state, i, io);
        }
      }

      return selected;
    }
  }

  /**
   *  Ergh. select/FD_* is not exactly user-oriented design.
   *
   *  @todo This is highly unoptimised since we always rebuild the FD_SETs. --rue
   */
  Object* IO::select(STATE, Object* readables, Object* writables, Object* errorables, Object* timeout) {
    struct timeval limit;
    struct timeval* maybe_limit = NULL;

    if(!timeout->nil_p()) {
      unsigned long long microseconds = as<Integer>(timeout)->to_ulong_long();
      limit.tv_sec = microseconds / 1000000;
      limit.tv_usec = microseconds % 1000000;
      maybe_limit = &limit;
    }

    fd_set read_set;
    fd_set* maybe_read_set = readables->nil_p() ? NULL : &read_set;

    fd_set write_set;
    fd_set* maybe_write_set = writables->nil_p() ? NULL : &write_set;

    fd_set error_set;
    fd_set* maybe_error_set = errorables->nil_p() ? NULL : &error_set;

    native_int highest = 0;
    native_int candidate = 0;

    /* Build the sets, track the highest descriptor number. These handle NULLs */
    highest = hidden_fd_set_from_array(state, readables, maybe_read_set);

    candidate = hidden_fd_set_from_array(state, writables, maybe_write_set);
    highest = candidate > highest ? candidate : highest;

    candidate = hidden_fd_set_from_array(state, errorables, maybe_error_set);
    highest = candidate > highest ? candidate : highest;

    /* And the main event, pun intended */
    WaitingForSignal waiter;
    state->install_waiter(waiter);

    {
    GlobalLock::UnlockGuard lock(state->global_lock());

    retry:
      native_int events = ::select((highest + 1), maybe_read_set,
                                                  maybe_write_set,
                                                  maybe_error_set,
                                                  maybe_limit);

      if(events == -1) {
        if(errno == EAGAIN || errno == EINTR) {
          goto retry;
        }

        Exception::errno_error(state, "::select() failed!");
      }

      /* Timeout expired */
      if(events == 0) {
        return Qnil;
      }

    }

    state->clear_waiter();

    /* Build the results. */
    Array* events = Array::create(state, 3);

    /* These handle NULL sets. */
    events->set(state, 0, hidden_reject_unset_fds(state, readables, maybe_read_set));
    events->set(state, 1, hidden_reject_unset_fds(state, writables, maybe_write_set));
    events->set(state, 2, hidden_reject_unset_fds(state, errorables, maybe_error_set));

    return events;
  }


/* Instance methods */

  Object* IO::reopen(STATE, IO* other) {
    native_int cur_fd   = to_fd();
    native_int other_fd = other->to_fd();

    if(dup2(other_fd, cur_fd) == -1) {
      Exception::errno_error(state, "reopen");
    }

    set_mode(state);
    if(IOBuffer* ibuf = try_as<IOBuffer>(ibuffer())) {
      ibuf->reset(state);
    }

    return Qtrue;
  }

  Object* IO::ensure_open(STATE) {
    if(descriptor_->nil_p()) {
      Exception::io_error(state, "uninitialized stream");
    }
    else if(to_fd() == -1) {
      Exception::io_error(state, "closed stream");
    }
    else if(to_fd() == -2) {
      Exception::io_error(state, "shutdown stream");
    }

    return Qnil;
  }

  Object* IO::connect_pipe(STATE, IO* lhs, IO* rhs) {
    int fds[2];
    if(pipe(fds) == -1) {
      Exception::errno_error(state, "creating pipe");
    }

    lhs->descriptor(state, Fixnum::from(fds[0]));
    rhs->descriptor(state, Fixnum::from(fds[1]));

    lhs->set_mode(state);
    rhs->set_mode(state);
    return Qtrue;
  }

  Integer* IO::seek(STATE, Integer* amount, Fixnum* whence) {
    ensure_open(state);

    off_t position = lseek(to_fd(), amount->to_long_long(), whence->to_native());

    if(position == -1) {
      Exception::errno_error(state);
    }

    return Integer::from(state, position);
  }

  /** This is NOT the same as shutdown(). */
  Object* IO::close(STATE) {
    ensure_open(state);

    /** @todo   Should this be just int? --rue */
    native_int desc = to_fd();

    switch(::close(desc)) {
    case -1:
      Exception::errno_error(state);
      break;

    case 0:
      // state->events->clear_by_fd(desc);
      descriptor(state, Fixnum::from(-1));
      break;

    default:
      std::ostringstream message;
      message << "::close(): Unknown error on fd " << desc;
      Exception::system_call_error(state, message.str());
    }

    return Qnil;
  }

  /**
   *  This is NOT the same as close().
   *
   *  @todo   Need to build the infrastructure to be able to only
   *          remove read or write waiters if a partial shutdown
   *          is requested. --rue
   */
  Object* IO::shutdown(STATE, Fixnum* how) {
    ensure_open(state);

    int which = how->to_int();
    native_int desc = to_fd();

    if(which != SHUT_RD && which != SHUT_WR && which != SHUT_RDWR) {
      std::ostringstream message;
      message << "::shutdown(): Invalid `how` " << which << " for fd " << desc;
      Exception::argument_error(state, message.str().c_str());
    }

    switch(::shutdown(desc, which)) {
    case -1:
      Exception::errno_error(state);
      break;

    case 0:
      if(which == SHUT_RDWR) {
        /* Yes, it really does need to be closed still. */
        (void) close(state);

        descriptor(state, Fixnum::from(-2));
      }

      /** @todo   Fix when can only remove read or write events. --rue */
      state->events->clear_by_fd(desc);
      break;

    default:
      std::ostringstream message;
      message << "::shutdown(): Unknown error on fd " << desc;
      Exception::system_call_error(state, message.str());
    }

    return how;
  }

  native_int IO::to_fd() {
    return descriptor_->to_native();
  }

  void IO::set_mode(STATE) {
    int acc_mode = fcntl(to_fd(), F_GETFL);
    if(acc_mode < 0) {
      Exception::errno_error(state);
    }
    mode(state, Fixnum::from(acc_mode));
  }

  void IO::unsafe_set_descriptor(native_int fd) {
    descriptor_ = Fixnum::from(fd);
  }

  void IO::force_read_only(STATE) {
    int m = mode_->to_native();
    mode(state, Fixnum::from((m & ~O_ACCMODE) | O_RDONLY));
  }

  void IO::force_write_only(STATE) {
    int m = mode_->to_native();
    mode(state, Fixnum::from((m & ~O_ACCMODE) | O_WRONLY));
  }


  Object* IO::sysread(STATE, Fixnum* number_of_bytes) {
    std::size_t count = number_of_bytes->to_ulong();
    String* buffer = String::create(state, number_of_bytes);

    ssize_t bytes_read;

  retry:
    WaitingForSignal waiter;
    state->install_waiter(waiter);

    {
      GlobalLock::UnlockGuard lock(state->global_lock());
      bytes_read = ::read(descriptor()->to_native(),
                          buffer->data()->bytes,
                          count);
    }

    state->clear_waiter();

    if(bytes_read == -1) {
      if(errno == EAGAIN || errno == EINTR) {
        goto retry;
      }

      return NULL;
    }

    if(bytes_read == 0) {
      return Qnil;
    }

    buffer->num_bytes(state, Fixnum::from(bytes_read));
    return buffer;
  }


  Object* IO::write(STATE, String* buf) {
    ssize_t cnt = ::write(this->to_fd(), buf->data()->bytes, buf->size());

    if(cnt == -1) {
      Exception::errno_error(state);
    }

    return Integer::from(state, cnt);
  }

  Object* IO::blocking_read(STATE, Fixnum* bytes) {
    String* str = String::create(state, bytes);

    ssize_t cnt = ::read(this->to_fd(), str->data()->bytes, bytes->to_native());
    if(cnt == -1) {
      Exception::errno_error(state);
    } else if(cnt == 0) {
      return Qnil;
    }

    str->num_bytes(state, Fixnum::from(cnt));

    return str;
  }

  Object* IO::query(STATE, Symbol* op) {
    ensure_open(state);

    native_int fd = to_fd();

    if(op == state->symbol("tty?")) {
      return isatty(fd) ? Qtrue : Qfalse;
    } else if(op == state->symbol("ttyname")) {
      return String::create(state, ttyname(fd));
    } else {
      return Qnil;
    }
  }

/* IOBuffer methods */

  IOBuffer* IOBuffer::create(STATE, size_t bytes) {
    IOBuffer* buf = state->new_object<IOBuffer>(G(iobuffer));
    buf->storage(state, ByteArray::create(state, bytes));
    buf->channel(state, Channel::create(state));
    buf->total(state, Fixnum::from(bytes));
    buf->used(state, Fixnum::from(0));
    buf->reset(state);

    return buf;
  }

  IOBuffer* IOBuffer::allocate(STATE) {
    return create(state);
  }

  Object* IOBuffer::unshift(STATE, String* str, Fixnum* start_pos) {
    write_synced(state, Qfalse);
    native_int start_pos_native = start_pos->to_native();
    native_int total_sz = str->size() - start_pos_native;
    native_int used_native = used_->to_native();
    native_int available_space = total_->to_native() - used_native;

    if(total_sz > available_space) {
      total_sz = available_space;
    }

    memcpy(storage_->bytes + used_native, str->byte_address() + start_pos_native, total_sz);
    used(state, Fixnum::from(used_native + total_sz));

    return Fixnum::from(total_sz);
  }

  Object* IOBuffer::fill(STATE, IO* io, CallFrame* calling_environment) {
    ssize_t bytes_read;

  retry:
    WaitingForSignal waiter;
    state->install_waiter(waiter);

    {
      GlobalLock::UnlockGuard lock(state->global_lock());
      bytes_read = read(io->descriptor()->to_native(),
                        at_unused(),
                        left());
    }

    state->clear_waiter();

    if(bytes_read == -1 && errno == EINTR) {
      if(!state->check_async(calling_environment)) return NULL;
      goto retry;
    }

    if(bytes_read > 0) {
      read_bytes(state, bytes_read);
    }

    return Fixnum::from(bytes_read);
  }

  void IOBuffer::reset(STATE) {
    used(state, Fixnum::from(0));
    start(state, Fixnum::from(0));
    eof(state, Qfalse);
  }

  void IOBuffer::read_bytes(STATE, size_t bytes) {
    used(state, Fixnum::from(used_->to_native() + bytes));
  }

  char* IOBuffer::byte_address() {
    return (char*)storage_->bytes;
  }

  size_t IOBuffer::left() {
    return total_->to_native() - used_->to_native();
  }

  char* IOBuffer::at_unused() {
    char* start = (char*)storage_->bytes;
    start += used_->to_native();
    return start;
  }
};
