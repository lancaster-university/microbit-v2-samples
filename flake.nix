
{
  description = "Dev environment for CODAL";

  inputs = {
    nixpkgs.url = "github:NixOs/nixpkgs/nixos-26.05";
  };

  outputs = { self, nixpkgs }: 
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forEachSupportedSystem = f:
        nixpkgs.lib.genAttrs supportedSystems
          (system: f {
            pkgs = import nixpkgs { inherit system; };
          });

    in
    {
      devShells = forEachSupportedSystem ({ pkgs }:
        let
          commonPackages = [
                # Build toolchain
                pkgs.gcc-arm-embedded-13
                pkgs.git
                pkgs.cmake
                pkgs.python3

                # Debugging
                pkgs.openocd
          ];
        in
        {
        default = pkgs.mkShell {
          packages = commonPackages;
        };

        ninja = pkgs.mkShell {
          packages = commonPackages ++ [
            pkgs.ninja
          ];
        };
      });
    };
}
