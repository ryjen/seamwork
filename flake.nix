{
  description = "Seamwork modern C++23 development and CI environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          clang
          clang-tools
          cmake
          gcc14
          ninja
        ];

        shellHook = ''
          if [ -z "$CC" ]; then export CC=gcc; fi
          if [ -z "$CXX" ]; then export CXX=g++; fi
          echo "Seamwork C++23 environment"
          echo "Compiler: $($CXX --version | head -n 1)"
          echo "Configure with: cmake --preset dev"
        '';
      };
    };
}
