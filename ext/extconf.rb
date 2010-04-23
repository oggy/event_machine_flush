#!/usr/bin/env ruby

# Find the EventMachine gem.
require 'rubygems'
specs = Gem.source_index.find_name('eventmachine')
specs.size > 0 or
  abort "can't find eventmachine gem"
event_machine_path = specs.first.full_gem_path

# Stub out create_makefile and load EventMachine's extconf.rb. This
# ensures our build environment matches.
require 'mkmf'
alias original_create_makefile create_makefile
def create_makefile(*) end
require "#{event_machine_path}/ext/extconf.rb"
alias create_makefile original_create_makefile

# Create our makefile.
$INCFLAGS << " -I#{event_machine_path}/ext"
create_makefile "event_machine_flush_ext"
