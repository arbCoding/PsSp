# Passive-source Seismic-processing (PsSp)

![PsSp Main Window](screenshots/pssp_main_window_16May2023.png)

The purpose of this is to provide an OS-independent, graphical, seismic-processing software package targeted at passive-source seismologists.

---

## Why does this exist?
### Summary

The purpose can be summarized as **extending the productivity suite of the seismologist**. Seismologists have their program for writing manuscripts
(e.g. MS Word, LaTeX), their program for giving scientific presentations (e.g. MS Powerpoint, Impress Presentation, etc.), their
program for handling emails (Outlook, Thunderbird, whatever). The gap that exists is **what program do they use to do seismic anlaysis**? Far too often,
it is whatever they manage to cludge together, so long as it *seems* to work.

### Introduction

Despite the various seismological tools that exist (and honestly, because of how they are designed) the seismologist will **most likely** need to code 
their own tool(s) (as a shell script stitching programs together, as a Python script using ObsPy, as a SAC macro, etc.). While the ability to do that if it is 
desired by the researcher is awesome, the need to do it is unfortunate as not everyone wants to (or knows how to) write their own codes. It gets worse when you consider the performance of these codes, or how the codes end up becoming obsolete after a short time (try using someone's old Python scripts, or Matlab codes, have them not work and be stuck trying to figure out what is wrong instead of making progress on your research).

### Discussion

The primary issues that I see today are:
1) There are a lack of tools available to the seismologist that have a graphical user interface (GUI).
2) Often tools only do one or a few jobs. This makes life easier for the developer (following the [KISS philosophy](https://en.wikipedia.org/wiki/KISS_principle)),
but it makes life harder for the end-user. Often the end-user needs to stitch/cludge together different tools, developed
by different persons/groups, in order to perform a given research task. Add in the additional complication of OS-exclusive software, locking users
of the wrong operating system out from certain tools and you have a tremendously unfortunate mess.

The problem is magnified when you consider that often the end-user doesn't necessarily know how to use the tool, nor the underlying
assumptions, nor the limitations. Often, these tools were never designed to be shared and therefore are designed in a non-intuitive fashion, with virtually no
comments in the actual code. These tools are often not documented (or under-documented, or even *incorrectly documented*), they tend to be assumed as just plain *obvious*, despite that being entirely dependent upon a very specific (and undocumented) workflow. These issues tend to be discovered after much confusion and frustration (hopefully fairly early on, as opposed to while writing a manuscript). That is not how science should work.

The disconnected nature of the typical seismic workflow leads to reproducibility issues. A researcher must keep track of every step taken in the analysis manually,
without error. This is easy when a research task is a straight line. However, when there is back-tracking, iterative analysis with minor tweaks, abandoned lines of
exploration, and so on, it becomes exceedingly difficult to be able to provide an accurate account of the actual processing steps necessary to consistently reproduce presented/published results. In this age of modern computing, it is simply **absurd** that the seismologist has no other choice than to work with this *severe tool-deficit*.

The researcher shouldn't need to expend immense amounts of time/energy/mental-bandwidth on making their tools, nor on making them work together. They should be focused on doing science. While advances in machine learning are allowing the modern seismologist to parse massive amounts of data with little effort, we must still look at our data and question the validity of our analysis/interpretation. And we should be able to do this with relative ease and minimal pain.

### Purpose

PsSp is being developed to solve this problem; to empower the seismologist with tools that are easy to use and foster exploration.
By enabling the scientist to do exploratory analysis quickly, easily, iteratively, and visually, I hope to allow the end-user to improve their
intuitive understanding of what they are doing with their data so that they can make an informed descision of how best to proceed
with their analysis. I hope this will also make entry into seismology easier (undergrads, summer interns, new graduate students, etc.) and
will make it easier for more-seasoned seismologists to use newer and more advanced tools, thus improving everyone's workflow and
the quality of research that is accomplished while minimizing the amount of time (and frustration) devoted to simply trying to get a functional workflow.

---
## Current status

This is extremely early in development.

### Current Focus: Data manipulation/display interface

The next major update is to provide an interface to interact with the data. As opposed to right now, where we do everything via displaying the values of
the sac_deque.

I think we can have the sac_deque window, but instead of should be an unordered_map of data_id's and file_names. It shows the file_names, upon
selecting, we get the data_id and can load that one data_id into a SacStream object, which will need to be tied to an update flag (so that we
can flag that it has been updated to whoever is accessing it). That is the object which gets plotted, or has the spectrum calculated and plotted.

It is also not directly tied to the sac_deque, such that when processing or I/O operations are occuring, we can still display it and everything related
to it (just cannot alter it if other things are happening). That means we won't need to close off as many windows during processing or I/O. We'll need
a flag to pass back to the deque if the user has altered its information (updated a header value, for instance) so that the change can be propagated
back to the sac_deque safely.

Then we need a window for displaying all headers values that allows the user to update them (with formatting restrictions to prevent nonsense).

Additionally, we can have an unordered_map for data_id's to header values, allowing us to show that same information, across all SacStreams in a
tabular format. To allow the user to scan and manipulate their data. (This is later).

Need a mechanism for grouping data (by station, by component, by event, by array, by whatever). An abstract grouping that can be as deep as the data allows. Think of it like grouping objects in illustrator, or in QGIS.

---

## ToDo

See the Todo list at the top of the [ToDo.md](ToDo.md) file for more info on what is currently going-on/planned for the future.

---

## Dependencies

Dependencies that are marked as 'Git submodule' are handled automatically. Other packages must be installed via your package manager of choice
or manually. For those other packages I provide installation guidance for MacOS and Linux systems [here](#compilation-instructions).

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
   * By using a plan-pool, that has an appropriate semaphore lock, I have implemented fft and ifft in a thread-safe fashion (super-fast!).
* [MessagePack](https://msgpack.org/)
   * Provides data-serialization, used to program settings.
* [Boost](https://www.boost.org/)
   * Required by MessagePack
* [SQLite3](https://sqlite.org/)
   * Projects are implemented as internal sqlite3 databases.
   * We are able to maintain data provenance information, processing checkpoints, and so on via a serverless relational database.

---

## Compilation instructions

I test this on M1 MacOS (Ventura 13.3.1 (a)), as well as on x86_64 Linux (Specifically Ubuntu 22.04).

**Note** I do not, currently, have a Windows system to test on. I suspect you'll want to use something along the lines of (in no particular order) [WinGet](https://github.com/microsoft/winget-cli), [Scoop](https://scoop.sh/), [vcpkg](https://vcpkg.io/), [Chocolatey](https://chocolatey.org/), or [Cygwin](https://www.cygwin.com/)
to setup your compilation environment on Windows.

---
### MacOS
Using [Homebrew](https://brew.sh/)
```shell
brew install fftw glfw msgpack-cxx sqlite
```

**NOTE** For MacOS users, if you want a stand-alone Application (PsSp.app, no need to execute from the terminal) there are
additional requirements. Please see the [additional instructions](#special-macos-application) for more information.

### Linux (Ubuntu 22.04/Debian based)
```shell
sudo apt install libfftw3-dev libglfw3-dev libboost-all-dev libmsgpack-dev libsqlite3-dev
```
---
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

---
### Tests
To make the test programs (test programs will go inside the ./bin/test/ directory) run
```shell
make tests
```
---
### Cleanup
To cleanup (including removing the compiled programs), run:
```shell
make clean
```
---

## Special MacOS Application

If you want a stand-alone MacOS application file, there are additional steps.

I do not take credit for figuring this out, I found this [blog post](https://newbedev.com/building-osx-app-bundle) on the topic.

First, I use [dylibbundler](https://github.com/auriamg/macdylibbundler/) to handle rebinding
the links for the non-standard dynamically linked libraries. The application bundle requires that they
be included in the application (such that user doesn't need to install them).

This can be installed via Homebrew
```shell
brew install dylibbundler
```

You can see which dylib's will need to be modified via the `otool` command after the program is compiled:
```shell
otool -L ./bin/PsSp
```

Anything not listed in `/System/Libary/` or `/usr/lib` will need to be included with the application.
Fortunately, **dylibbundler** can handle that for us.
```shell
dylibbundler -s /opt/homebrew/lib/ -od -b -x ./PsSp.app/Contents/MacOS/PsSp -d ./PsSp.app/Contents/libs/
```

Of course, this is implemented automatically in the [Makefile](Makefile), assuming you also used Homebrew to install the other packages (non-Git submodules).

---
For more details, checkout the [Makefile](Makefile). It is heavily commented to make it more accessible.
---
