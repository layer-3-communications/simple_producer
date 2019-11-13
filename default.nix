{ pkgs ? import <nixpkgs> {} }:

with pkgs;

stdenv.mkDerivation rec {
  pname = "simple_producer";
  version = "1.0";

  src = ./.;

  buildInputs = [ rdkafka libyaml netcat ];

  nativeBuildInputs = [ gcc ];

  buildPhase = ''
    cd src/
    gcc main.c \
      connect.c config.c kafka.c util.c \
      -o simple_producer \
      -lyaml -lrdkafka \
      -Wall -Werror \
      -O3
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp simple_producer $out/bin
  '';

  shellHook = ''
    function test() {
      nc -Uu sp_sock
    }
  '';
}
