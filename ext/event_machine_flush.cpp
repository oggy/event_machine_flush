#include "ruby.h"
#include "project.h"


static void push_back_data(ConnectionDescriptor *ed, char *data, int len) {
  char *buffer = (char*)malloc(len + 1);
  if (!buffer)
    throw std::runtime_error("bad alloc throwing back data");
  memcpy(buffer, data, len);
  buffer[len] = 0;
  ed->OutboundPages.push_front(ConnectionDescriptor::OutboundPage(buffer, len));
}

extern "C" {
  VALUE EventMachineFlush_flush(VALUE self, VALUE connection) {
    unsigned long signature = NUM2ULONG(rb_iv_get(connection, "@signature"));
    ConnectionDescriptor *ed = dynamic_cast<ConnectionDescriptor*>(Bindable_t::GetObject(signature));

    // Based on ConnectionDescriptor::_WriteOutboundData.
    while (ed->OutboundPages.size() > 0) {
      int sd = ed->GetSocket();

      if (sd == INVALID_SOCKET)
        break;

      size_t nbytes = 0;
#ifdef HAVE_WRITEV
      int iovcnt = ed->OutboundPages.size();
      if (iovcnt > 16) iovcnt = 16;

#  ifdef CC_SUNWspro
      struct iovec iov[16];
#  else
      struct iovec iov[iovcnt];
#  endif

      for(int i = 0; i < iovcnt; i++){
        ConnectionDescriptor::OutboundPage *op = &(ed->OutboundPages[i]);
#  ifdef CC_SUNWspro
        iov[i].iov_base = (char *)(op->Buffer + op->Offset);
#  else
        iov[i].iov_base = (void *)(op->Buffer + op->Offset);
#  endif
        iov[i].iov_len	= op->Length - op->Offset;

        nbytes += iov[i].iov_len;
      }
#else
      char output_buffer[16 * 1024];

      while ((ed->OutboundPages.size() > 0) && (nbytes < sizeof(output_buffer))) {
        ConnectionDescriptor::OutboundPage *op = &(OutboundPages[0]);
        if ((nbytes + op->Length - op->Offset) < sizeof(output_buffer)) {
          memcpy(output_buffer + nbytes, op->Buffer + op->Offset, op->Length - op->Offset);
          nbytes += (op->Length - op->Offset);
          op->Free();
          ed->OutboundPages.pop_front();
        } else {
          int len = sizeof(output_buffer) - nbytes;
          memcpy(output_buffer + nbytes, op->Buffer + op->Offset, len);
          op->Offset += len;
          nbytes += len;
        }
      }
#endif

      // Noop if only empty pages have been queued.
      if (nbytes == 0)
        break;

#ifdef HAVE_WRITEV
      int bytes_written = writev(ed->GetSocket(), iov, iovcnt);
#else
      int bytes_written = write(ed->GetSocket(), output_buffer, nbytes);
#endif

      // Noop if we couldn't write. Since we didn't select on the
      // socket, it mightn't be writable.
      if (bytes_written <= 0) {
#ifndef HAVE_WRITEV
        push_back_data(ed, output_buffer, nbytes);
#endif
        break;
      }
      ed->OutboundDataSize -= bytes_written;

#ifdef HAVE_WRITEV
      unsigned int sent = bytes_written;
      deque<ConnectionDescriptor::OutboundPage>::iterator op = ed->OutboundPages.begin();

      for (int i = 0; i < iovcnt; i++) {
        if (iov[i].iov_len <= sent) {
          // Sent this page in full, free it.
          op->Free();
          ed->OutboundPages.pop_front();

          sent -= iov[i].iov_len;
        } else {
          // Sent part (or none) of this page, increment offset to send the remainder
          op->Offset += sent;
          break;
        }

        // Shouldn't be possible run out of pages before the loop ends
        assert(op != ed->OutboundPages.end());
        *op++;
      }
#else
      if ((size_t)bytes_written < nbytes)
        push_back_data(ed, output_buffer + bytes_written, nbytes - bytes_written);
#endif
    }

    return Qnil;
  }

  void Init_event_machine_flush_ext() {
    VALUE mEventMachineFlush = rb_define_module("EventMachineFlush");
    rb_define_singleton_method(mEventMachineFlush, "flush", (VALUE(*)(...))EventMachineFlush_flush, 1);
  }
}
