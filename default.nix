{ pkgs ? import <nixpkgs> {} }:

with pkgs;

stdenv.mkDerivation rec {
  pname = "simple_producer";
  version = "1.0";

  src = ./.;

  buildInputs = [ rdkafka libyaml ];

  nativeBuildInputs = [ gcc ];

  buildPhase = ''
    cd src/
    gcc main.c \
      connect.c config.c kafka.c util.c \
      -o simple_producer \
      -lyaml -lrdkafka
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp simple_producer $out/bin
  '';
}
