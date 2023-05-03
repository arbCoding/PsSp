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

This project uses [Dear ImGui](https://github.com/ocornut/imgui/tree/v1.89.5) v1.89.5. This is necessary for compilation of the GUI. Specifically, the [OpenGL3](https://www.opengl.org/) and the [GLFW](https://www.glfw.org/) backends.

This project uses [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog.git), Lib_Only branch. This is necessary for file dialogs in Dear ImGui.

This project uses [sac-format](https://github.com/arbCoding/sac-format). This is necessary for compilation of any seismic processing methods.

This project uses [FFTW3](https://www.fftw.org/). This is necessary for spectral functionality (FFT, IFFT).

Additional dependencies are **TBD**.
