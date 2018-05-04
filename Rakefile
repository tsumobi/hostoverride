require 'rake'
require 'rake/testtask'

Rake::TestTask.new(:test) do |t|
  t.pattern = 'test/test-*.rb'
  t.libs = ['test/helpers']
  t.verbose = true
  t.warning = true
end
