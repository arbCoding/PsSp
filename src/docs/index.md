# Passive-source Seismic-Processing {#mainpage}

Passive-source Seismic-processing (PsSp) aims to provide an OS-independent,
graphically driven, and free seismic processing application targeted at
passive-source seismologists.

## Summary of Purpose

The purpose of this project is to **extend the productivity suite** of the
passive-source seismologist. Great tools exist for writing manuscripts (such as
MS Word, LibreOffice Write, LaTeX, and so on). Great tools exist for creating
presentations (e.g. MS Powerpoint, Impress Persentation, and so on). Great tools
exist for communicating with each other across the world (e.g. MS Outlook,
Thunderbird, Zoom, MS Teams, and so on). What tools exist for actually doing the
seismic analysis? Far too often it is whatever the analyst manages to kludge
together. PsSp aims to fill this gap with a modern graphical-interface, fast
computation, and some much needed quality of life functionality (undo/redo,
notes, checkpoints, and so on).

## Introduction

Despite the numerous seismological tools that exist (SAC, Seismic Unix,
Computer Programs in Seismology, ObsPy, and so on), and by the nature of their
design, the typical seismologist will **most likely** need to code their own
tool(s) and workflow(s). Often, this takes the form of scripts/macros to stitch
together the output from one program to the input of another---taking into
account any necessary intermediate data transformations. Having the ability to
do this is awesome, needing to do this is not. This leads to poorly written,
designed, documented, and tested codes. Even mature programs suffer from these
problems, placing the onus on the user to make up for the mistakes of the
creator.

It gets worse. Scientists often choose a language out of convenience: Fortran
because everyone uses it (often using archaic programming conventions that were
best lost to their decade of origin); or Python because it's easy and a ton of
fun when it breaks every few months after a library gets updated, commands get
deprecated, or during the ugly transition from 2.x to 3.x; Matlab because it
provides all the fun of code-breakage from Python **and** its language features
are stuck behind a paywall---like playing a modern video game; or whatever other
language is in vogue as the next greatest innovation in developing barely
functional code quickly.

For an analyst pushing the envelop to develop entirely novel analysis
approaches, programming will always be a necessity. For the numerous others that
are focused exclusively on **using** already developed analysis methods,
programming should not be necessary. PsSp will fill this gap as a modern
software solution.
