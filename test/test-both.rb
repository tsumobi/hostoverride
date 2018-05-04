require 'test/unit'

require 'test-helpers'

class TestBoth < Test::Unit::TestCase
  include TestHelpers

  LOOPBACK = '127.0.0.1'

  unless CONNECTOVERRIDE_EXE = ENV['CONNECTOVERRIDE_EXE']
    raise "CONNECTOVERRIDE_EXE not set"
  end

  unless HOSTOVERRIDE_EXE = ENV['HOSTOVERRIDE_EXE']
    raise "HOSTOVERRIDE_EXE not set"
  end

  def test_both_together
    random_token = (0...20).map { ('a'..'z').to_a.sample }.join

    # Start a simple tcp server, exposing details only through CONNECTOVERRIDE
    dest_port, thr = serve_token_once_tcp(random_token)

    src_host = "xyz"
    src_ip = random_unused_loopback_ip
    src_port = rand(1023) + 1 # privileged port

    env = {
      'HOSTOVERRIDE' => "#{src_host}=#{src_ip}",
      'CONNECTOVERRIDE' => "#{src_ip}:#{src_port}=#{LOOPBACK}:#{dest_port}"
    }

    actual = pread(env, HOSTOVERRIDE_EXE, CONNECTOVERRIDE_EXE,
        *read_token_tcp_cmd(src_ip, src_port)).chomp

    assert_equal(random_token, actual)
  ensure
    thr.join
  end
end
