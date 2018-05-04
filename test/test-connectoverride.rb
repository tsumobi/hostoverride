require 'test/unit'

require 'ipaddr'

require 'test-helpers'

class TestConncetoverride < Test::Unit::TestCase
  include TestHelpers

  unless CONNECTOVERRIDE_EXE = ENV['CONNECTOVERRIDE_EXE']
    raise "CONNECTOVERRIDE_EXE not set"
  end

  LOOPBACK = '127.0.0.1'
  LOOPBACK_IPv6 = IPAddr.new(LOOPBACK).ipv4_mapped.to_s

  def test_32bit_compatible
    sh(CONNECTOVERRIDE_EXE, emit_dummy_prog('i386'))
  end

  def test_64bit_compatible
    sh(CONNECTOVERRIDE_EXE, emit_dummy_prog('x86_64'))
  end

  def test_connectoverride_ipv6
    random_token = (0...20).map { ('a'..'z').to_a.sample }.join

    # Start a simple tcp server, exposing details only through CONNECTOVERRIDE
    dest_port, thr = serve_token_once_tcp(random_token)

    src_ip = IPAddr.new(random_unused_loopback_ip).ipv4_mapped.to_s
    src_port = rand(1023) + 1 # privileged port

    env = {
      'CONNECTOVERRIDE' => "#{src_ip}:#{src_port}=#{LOOPBACK_IPv6}:#{dest_port}"
    }
    actual = pread(env, CONNECTOVERRIDE_EXE,
        *read_token_tcp_cmd(src_ip, src_port)).chomp

    assert_equal(random_token, actual)
  ensure
    thr.join
  end

  def test_connectoverride_ipv4
    random_token = (0...20).map { ('a'..'z').to_a.sample }.join

    # Start a simple tcp server, exposing details only through CONNECTOVERRIDE
    dest_port, thr = serve_token_once_tcp(random_token)

    src_ip = random_unused_loopback_ip
    src_port = rand(1023) + 1 # privileged port

    env = {
      'CONNECTOVERRIDE' => "#{src_ip}:#{src_port}=#{LOOPBACK}:#{dest_port}"
    }
    actual = pread(env, CONNECTOVERRIDE_EXE,
        *read_token_tcp_cmd(src_ip, src_port)).chomp

    assert_equal(random_token, actual)
  ensure
    thr.join
  end
end
