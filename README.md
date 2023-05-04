# PsSp

Passive-source Seismic-processing with a GUI (Graphical User Interface)

This project depends on my [sac-format](https://github.com/arbCoding/seismic) library.

The purpose of this is to provide an OS-independent processing software with a GUI.

---

## Current status

This is extremely early in development. ./src/tests/imgui_test.cpp is currently my playground for setting up the GUI.

---

After cloning this repository, remember to run:
```shell
git submodule update --init
```
To also download the sac-format submodule

---

## Dependencies

The only dependency that is **NOT** handled automatically is FFTW3. The others are submodules and are handled automatically for you.

This project uses [Dear ImGui](https://github.com/ocornut/imgui/tree/v1.89.5) v1.89.5. This is necessary for compilation of the GUI. Specifically, the [OpenGL3](https://www.opengl.org/) and the [GLFW](https://www.glfw.org/) backends.

This project uses [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog.git), Lib_Only branch. This is necessary for file dialogs in Dear ImGui.

This project uses [sac-format](https://github.com/arbCoding/sac-format). This is necessary for compilation of any seismic processing methods.

This project uses [FFTW3](https://www.fftw.org/). This is necessary for spectral functionality (FFT, IFFT).

Additional dependencies are **TBD**.

---

## Compilation instructions

You'll need to install FFTW3 yourself.

On MacOs, I do this with [Homebrew](https://brew.sh/) as follows
```shell
brew install fftw
```

On Linux (Ubuntu 22.04 confirmed, Debian based) (Double check to make sure this is correct)
```shell
sudo apt install libfftw3-dev
```

After you've installed FFTW you need to clone this project and initialize the submodules
```shell
git clone https://github.com/arbCoding/PsSp.git
cd PsSp
git submodule update --init
```

That will download the appropriate submodule dependencies, with the correct commit version, automatically from their respective GitHub repositories.
You can confirm that by examining them inside the submodules directory (they will be empty before you initialize them and population afterward).

Then it is a simple as running
```shell
make
```

To make all the programs, which will be inside the ./bin/ directory (test programs will go inside the ./bin/test/ directory).

To cleanup (including removing the compiled programs), run:
```shell
make clean
```

For more details, checkout the [Makefile](Makefile). It is heavily commented to make it more accessible.

---
