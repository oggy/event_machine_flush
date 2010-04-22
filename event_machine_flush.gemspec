# -*- encoding: utf-8 -*-
$:.unshift File.expand_path('lib', File.dirname(__FILE__))
require 'event_machine_flush/version'

Gem::Specification.new do |s|
  s.name        = 'event_machine_flush'
  s.date        = Date.today.strftime('%Y-%m-%d')
  s.version     = EventMachineFlush::VERSION.join('.')
  s.platform    = Gem::Platform::RUBY
  s.authors     = ["George Ogata"]
  s.email       = ["george.ogata@gmail.com"]
  s.homepage    = "http://github.com/oggy/event_machine_flush"
  s.summary     = "Flush EventMachine::Connection output buffers immediately."
  s.description = <<-EOS.gsub(/^ *\|/, '')
    |For when you\'ve abandoned all hope.
  EOS

  s.add_dependency 'eventmachine', '~> 0.12.10'
  s.required_rubygems_version = ">= 1.3.6"
  s.files = Dir["{doc,lib,rails}/**/*", "ext/*.{cpp,h,rb}"] + %w(LICENSE README.markdown Rakefile CHANGELOG)
  s.extensions << 'ext/extconf.rb'
  s.test_files = Dir["spec/**/*"]
  s.extra_rdoc_files = ["LICENSE", "README.markdown"]
  s.require_paths = ['ext', 'lib']
  s.specification_version = 3
  s.rdoc_options = ["--charset=UTF-8"]
end
