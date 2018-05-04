require 'test/unit'

require 'test-helpers'

class TestHostoverride < Test::Unit::TestCase
  include TestHelpers

  unless HOSTOVERRIDE_EXE = ENV['HOSTOVERRIDE_EXE']
    raise "HOSTOVERRIDE_EXE not set"
  end

  def test_32bit_compatible
    sh(HOSTOVERRIDE_EXE, emit_dummy_prog('i386'))
  end

  def test_64bit_compatitblle
    sh(HOSTOVERRIDE_EXE, emit_dummy_prog('x86_64'))
  end

  def test_simple_override
    overrides = { 'x' => "1.1.1.1" }
    assert_equal("1.1.1.1", hostlookup(overrides, 'x'))

    # Can handle multiple overrides
    overrides['y'] = "2.2.2.2"
    assert_equal("2.2.2.2", hostlookup(overrides, 'y'))
  end

  # Should deal with nil nodename argument to getaddrinfo(4)
  def test_nil_getaddrinfo
    hostlookup({ 'x' => '1.1.1.1' }, nil)
  end

  private

  def hostlookup(hostoverrides, host)
    env = {
      'HOSTOVERRIDE' => hostoverrides.map { |k, v| "#{k}=#{v}" }.join(' ')
    }

    cmd = [
      env, HOSTOVERRIDE_EXE, 'ruby', '-rsocket', '-e',
      "print Socket::getaddrinfo(#{host.inspect}, 0)[0][3]"
    ]

    pread(*cmd)
  end
end
