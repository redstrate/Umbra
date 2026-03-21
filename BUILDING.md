# Building Umbra

There are two methods to build Umbra, either via [Flatpak](https://flatpak.org/) or manually using your system libraries. It's highly recommended to prefer the Flatpak, especially if you don't have experience with CMake, C++ and such.

## Flatpak

Building the Flatpak version is easy, and there's a helper script to speed up the process. You must run it from the repository root:

```
$ cd umbra
$ ./scripts/build-flatpak.sh
```

The process should only take a few minutes on a moderately powerful machine. It does require an Internet connection and the relevant permissions to install the required Flatpak runtimes and extensions.

When it's complete, a file called `umbra.flatpak` will appear in the repository root and that can be installed with the `flatpak` CLI tool or your preferred application store.

## Manual

The process to build Umbra manually is a little bit more involved, but not difficult. It's easiest to do on rolling release Linux distributions. 

### Dependencies

#### Required

* [Linux](https://kernel.org/)
  * Windows, macOS and other systems may work but are currently unsupported. Patches are accepted to fix any problems with those OSes though.
* [CMake](https://cmake.org) 3.25 or later
* [Qt](https://www.qt.io/) 6.6 or later
  * Base, Declarative, WebView, Concurrent
* [KDE Frameworks](https://develop.kde.org/products/frameworks/) 6
  * Extra CMake Modules, Kirigami, I18n, Config, CoreAddons and Archive.
* [QtKeychain](https://github.com/frankosterfeld/qtkeychain)
* [QCoro](https://qcoro.dvratil.cz/)

### Getting source code

Umbra has git submodules that must be cloned alongside the repository, so make sure to pass the `--recursive` flag:

```bash
$ git clone --recursive https://github.com/redstrate/Umbra.git
```

If you missed it, it's possible to initialize the submodules afterward:

```bash
$ git submodule update --init --recursive
```

### Configuring

To configure, run `cmake` in the source directory:

```bash
$ cd Umbra
$ cmake -S . -B build
```

This command will create a new build directory and configure the source directory (`.`). If you want to enable more options, pass them now:

```bash
$ cmake -S . -B build
```

## Building

Now begin building the project:

```bash
$ cmake --build build
```

If the build was successful, an `umbra` binary will be found in `./build/bin/umbra`.
