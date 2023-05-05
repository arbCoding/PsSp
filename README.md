# PsSp

Passive-source Seismic-processing with a GUI (Graphical User Interface)

This project depends on my [sac-format](https://github.com/arbCoding/seismic) library.

The purpose of this is to provide an OS-independent, graphical, seismic processing software package targeted at passive seismologists.

---

## Current status

This is extremely early in development.

---

## ToDo

- [X] Read single SAC file
- [X] Display SAC header information
- [ ] Plot Seismogram
- [ ] Read 3-component Seismic data (3 SAC files)
- [ ] Plot 3-component Seismic data
- [ ] Read Array seismic many (many 1-component or 3-component SAC files)
- [ ] Display event epicenter on map
- [ ] Display station/array positions on map
- [ ] Instrument response removal
- [ ] Filtering (1 or many)
- [ ] Record section plotting (many stations)
- [ ] Plot spectrogram
- [ ] Plot Spectrum

---

## Dependencies

The only dependencies that are **NOT** handled automatically are FFTW3, GLFW3, and OpenGL3.
I provide information on installing them on MacOS and Linux [here](#compilation-instructions).

The other dependencies are setup as Git submodules and handled automatically.

Info on dependencies:
 * [Dear ImGui](https://github.com/ocornut/imgui/tree/v1.89.5) v1.89.5. This provides the OS-independent GUI.
 * [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog), Lib_Only branch. This adds OS-independent File Dialogs to Dear ImGui.
 * [ImPlot](https://github.com/epezent/implot). This adds OS-independent plotting functionality to Dear ImGui. (Currently implementing)
 * [OpenGL3](https://www.opengl.org/) this is a graphical backend for the GUI.
 * [GLFW3](https://www.glfw.org/) this is a graphical backend for the GUI.
 * [sac-format](https://github.com/arbCoding/sac-format). This provides binary SAC-file (seismic) I/O, both low-level functions and the high-level SacStream class.
 * [FFTW3](https://www.fftw.org/). This is necessary for spectral functionality (FFT, IFFT).

---

## Compilation instructions

I test this on M1 MacOS (Ventura 13.3.1 (a)), as well as on x86_64 Linux (Specifically Ubuntu 22.04).

You'll need to install FFTW3 and GLFW yourself. I believe GLFW provides/includes OpenGL.

**Note** I do not, currently, have a Windows system to test on. I suspect you'll want to use something along the lines of [Chocolatey](https://chocolatey.org/) or [Cygwin](https://www.cygwin.com/)
to setup your compilation environment on Windows.

On MacOs, I do this with [Homebrew](https://brew.sh/) as follows
```shell
brew install fftw glfw
```

On Linux (Ubuntu 22.04, Debian based)
```shell
sudo apt install libfftw3-dev libglfw3-dev
```

Next you need to clone this project and initialize the submodules
```shell
git clone https://github.com/arbCoding/PsSp.git
cd PsSp
git submodule update --init
```

That will download the appropriate submodule dependencies, with the correct commit version, automatically, from their respective GitHub repositories.
You can confirm that by examining them inside the submodules directory (they will be empty before you initialize them and populate afterward).

Then it is a simple as running
```shell
make
```

To make all of the programs, which will be inside the ./bin/ directory (test programs will go inside the ./bin/test/ directory).

To cleanup (including removing the compiled programs), run:
```shell
make clean
```

For more details, checkout the [Makefile](Makefile). It is heavily commented to make it more accessible.

---
