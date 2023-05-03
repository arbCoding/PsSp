# PsSp
Passive-source Seismic-processing with a GUI (Graphical User Interface)

This project depends on my [sac-format](https://github.com/arbCoding/seismic) library.

The purpose of this is to provide an OS-independent processing software with a GUI.

---

## Dependencies

This project uses [Dear ImGui](https://github.com/ocornut/imgui/tree/v1.89.5) v1.89.5. This is necessary for compilation of the GUI. Specifically, the [OpenGL3](https://www.opengl.org/) and the [GLFW](https://www.glfw.org/) backends.

This project uses [sac-format](https://github.com/arbCoding/sac-format). This is necessary for compilation of any seismic processing methods.

This project uses [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) v3.13. This is necessary for native-OS file dialogs.

This project uses [FFTW3](https://www.fftw.org/). This is necessary for spectral functionality (FFT, IFFT).

Additional dependencies are **TBD**.
