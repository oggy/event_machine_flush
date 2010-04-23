#include "ruby.h"
#include "project.h"

extern "C" {
  VALUE EventMachineFlush_flush(VALUE self, VALUE connection) {
    unsigned long signature = NUM2ULONG(rb_iv_get(connection, "@signature"));
    ConnectionDescriptor *ed = dynamic_cast<ConnectionDescriptor*>(Bindable_t::GetObject(signature));
    while (ed->OutboundPages.size() > 0)
      ed->Write();
    return Qnil;
  }

  void Init_event_machine_flush_ext() {
    VALUE mEventMachineFlush = rb_define_module("EventMachineFlush");
    rb_define_singleton_method(mEventMachineFlush, "flush", (VALUE(*)(...))EventMachineFlush_flush, 1);
  }
}
