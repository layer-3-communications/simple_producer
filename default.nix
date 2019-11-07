{ pkgs ? import <nixpkgs> {} }:

with pkgs;

stdenv.mkDerivation rec {
  pname = "simple_producer";
  version = "1.0";

  src = ./.;

  buildInputs = [ rdkafka libyaml ncurses netcat ];

  nativeBuildInputs = [ gcc ];

  buildPhase = ''
    cd src/
    gcc main.c \
      connect.c config.c kafka.c util.c \
      -o simple_producer \
      -lyaml -lrdkafka -lncurses \
      -Wall
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp simple_producer $out/bin
  '';

  shellHook = ''
    connect() {
      nc -uU $1
    }

    echo_lines() {
      while IFS="" read -r p || [ -n "$p" ]
      do
        printf '%s\n' "$p"
      done < test/mobey_dick.txt
    }

    test() {
      echo_lines | nc -uU my_socket_path
    }
  '';
}
