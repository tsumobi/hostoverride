require 'fileutils'
require 'socket'
require 'timeout'
require 'tmpdir'

module TestHelpers
  DUMMY_SOURCE_CONTENTS = 'int main() { return 0; }'

  def setup
    @scratch_dir = Dir.mktmpdir
  end

  def teardown
    FileUtils.rm_rf(@scratch_dir)
  end

  def emit_dummy_prog(arch)
    dummy_source = write_file(@scratch_dir, 'dummy.c', DUMMY_SOURCE_CONTENTS)
    dummy = File.join(@scratch_dir, 'dummy')

    sh('gcc', '-Wall', '-arch', arch, '-o', dummy, dummy_source)

    dummy
  end

  # Be sure the first octet is 127 to not disturb anybody with a failing test
  def random_unused_loopback_ip
    [127, *(0...3).map { (rand(127) + 128)}].map(&:to_s).join('.')
  end

  # Single use tcp server. It will write the supplied token to the first cilent
  # to establish a connection.
  # Waits up to timeout for a client to connect.
  # returns the port it's serving on and a thread that contains the server.
  def serve_token_once_tcp(token, timeout=5)
    server = TCPServer::new(0)
    port = server.addr[1]
    thr = Thread.start do
      begin
        sock = Timeout::timeout(timeout) { server.accept }

        sock.write(token)
      ensure
        sock.close if sock
        server.close
      end
    end

    return [port, thr]
  end

  def write_file(*path_fragments, contents)
    path = File.join(*path_fragments)
    File.open(path, 'w') { |io| io << contents }

    path
  end

  def read_token_tcp_cmd(ip, port)
    ['ruby', '-rsocket', '-e', "puts TCPSocket.new(#{ip.inspect}, #{port}).read"]
  end

  def pread(*cmd)
    out = IO.popen(cmd, &:read)
    unless $?.success?
      raise [
        "Error (#$?): #{cmd.inspect} failed", "STDOUT", out
      ].join("\n")
    end

    out
  end

  def sh(*cmd)
    unless system(*cmd)
      raise "Command failed (#$?): #{cmd.inspect}"
    end
  end
end
