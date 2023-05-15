# PsSp

Passive-source Seismic-processing with a GUI (Graphical User Interface)

The purpose of this is to provide an OS-independent, graphical, seismic-processing software package targeted at passive-source seismologists.

---

## Current status

This is extremely early in development. 

It is now officially multi-threaded (11 May 2023)!

User-projects are currently being prototyped. This will allow users to have distinct projects, that protect their original data sources,
have a degree of safety (via auto-save), and keep user defined program settings.

---

## Project Goals

In the passive seismic community, there are many processing tools available. The biggest problem, however, is the steep learning curve needed to use them.

The primary issues that I see today are:
1) There are a lack of tools available to the seismologist that have a graphical user interface (GUI).
2) Often tools only do one or a few jobs. This makes life easier for the developer (following the KISS philosophy),
but it makes life harder for the end-user. Often the end-user needs to stitch/cludge together different tools, developed
by different persons/groups, in order to perform a given research task.

The problem is magnified when you consider that often the end-user doesn't necessarily know how to use the tool, or underlying
assumptions, or limitations. This tends to be discovered after much confusion and frustration. That is not how science should work.

We shouldn't need to expend immense amounts of time/energy/mental-bandwidth on getting our tools to work for us. We should be focused
on doing science.

PsSp is being developed to remove that barrier. To empower the seismologist with tools that are easy to use and foster exploration.
By enabling the scientist to do exploratory analysis quickly, easily, and visually, I hope to allow the end-user to improve their
intuitive understanding of what they are doing with their data so that they can make an informed descision of how best to proceed
with their analysis. I hope this will also make entry into seismology easier (undergrads, summer intern, new graduate students) and
will make it easier for more-seasoned seismologists to use newer and more advanced tools, thus improving their everyone's workflow and
the quality of research that is accomplished while minimizing the amount of time devoted to simply trying to get the tools to work.

The goal can be summarized as extending the productivity suite of the seismologist. Seismologists have their program for writing manuscripts
(MS Word, LaTeX, whatever), their program for giving scientific presentations (MS Powerpoint, Impress Presentation, etc.), their
program for handling emails (Outlook, Thunderbird, whatever). The gap that exists is **what does one open to do seismic anlaysis**?
Of all the choices out there, the seismologist will **most likely** need to code their own tool (as a shell script stitching programs
together, as a Python script using ObsPy, as a SAC macro, etc.). While the ability to do that if desired is awesome, the need to do it
is unfortunate as not everyone wants to (or knows how to) write their own codes. It gets worse when you consider the performance of these
codes, or how the codes end up becoming obsolete after a short time (try using someone's old Python scripts, or Matlab codes, have them not
work and be stuck trying to figure out what is wrong instead of making progress on your research).

---

## ToDo

See also the Todo list at the top of the [main.cpp](/src/code/main.cpp) file for more info on what is going on/planned.

---

## Dependencies

Dependencies that are marked as 'Git submodule' are handled automatically. Other packages must be installed via your package manager of choice
or manually. For those other packages I provide installation guidance for MacOS and Linux systems [here](#compilation-instructions).

The other dependencies are setup as Git submodules and handled automatically.

Info on dependencies:
 * [Dear ImGui](https://github.com/ocornut/imgui/tree/v1.89.5) v1.89.5
    * This provides the OS-independent GUI.
    * Git submodule.
 * [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog), Lib_Only branch
    * This adds OS-independent File Dialogs to Dear ImGui.
    * Git submodule.
 * [ImPlot](https://github.com/epezent/implot).
    * This adds OS-independent plotting functionality to Dear ImGui.
    * Git submodule.
 * [sac-format](https://github.com/arbCoding/sac-format)
    * This provides binary SAC-file (seismic) I/O, both low-level functions and the high-level SacStream class.
    * Git submodule.
 * [OpenGL3](https://www.opengl.org/)
    * This is a graphical backend for the GUI.
 * [GLFW3](https://www.glfw.org/)
    * This is a graphical backend for the GUI.
 * [FFTW3](https://www.fftw.org/)
    * This is necessary for spectral functionality (FFT, IFFT).
 * [MessagePack](https://msgpack.org/)
    * Provides data-serialization, for user project settings.
 * [Boost](https://www.boost.org/)
    * Required by MessagePack

---

## Compilation instructions

I test this on M1 MacOS (Ventura 13.3.1 (a)), as well as on x86_64 Linux (Specifically Ubuntu 22.04).

You'll need to install FFTW3 and GLFW yourself. I believe GLFW provides/includes OpenGL.

**Note** I do not, currently, have a Windows system to test on. I suspect you'll want to use something along the lines of [vcpkg](https://vcpkg.io/), [Chocolatey](https://chocolatey.org/), or [Cygwin](https://www.cygwin.com/)
to setup your compilation environment on Windows.

On MacOs, I do this with [Homebrew](https://brew.sh/) as follows
```shell
brew install fftw glfw msgpack-cxx
```

On Linux (Ubuntu 22.04, Debian based)
```shell
sudo apt install libfftw3-dev libglfw3-dev
```

Next you need to clone this project and initialize the [submodules](submodules)
```shell
git clone https://github.com/arbCoding/PsSp.git
cd PsSp
git submodule update --init
```

That will download the appropriate submodule dependencies, with the correct commit version, automatically, from their respective GitHub repositories.
You can confirm that by examining them inside the submodules directory (they will be empty before you initialize them and populate afterward).

**NOTE** if a submodule is not the correct version (detached head, but submodule was updated)
From the base git dir (PsSp) run
```shell
git submodule update --init --remote submodules/
```

Then it is a simple as running
```shell
make
```

To make PsSp, which will be inside the ./bin/ directory. 

To make the test programs (test programs will go inside the ./bin/test/ directory) run
```shell
make tests
```

To cleanup (including removing the compiled programs), run:
```shell
make clean
```

For more details, checkout the [Makefile](Makefile). It is heavily commented to make it more accessible.

---
