require 'brewkit'

class Mona < Formula
  url 'http://www.brics.dk/mona/download/mona-1.4-17.tar.gz'
  homepage ''
  md5 ''

# depends_on 'cmake'

  def install
    system "./configure"
    system "make"
    system "make install-strip"
  end
end