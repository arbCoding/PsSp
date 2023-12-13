// Copyright 2023 Alexander R. Blanchette

#include "Main.hpp"

namespace pssp {
Main_Window::Main_Window() : Fl_Window(mw_width, mw_height, name_.c_str()) {
  this->begin();
  resizable(this);
  // Program
  menu.add("&Program/&Quit", FL_COMMAND + 'q', quit_cb, this);
  // Project
  menu.add("&Project/&New", FL_COMMAND + 'n', 0, this, FL_MENU_INACTIVE);
  menu.add("&Project/&Load", FL_COMMAND + 'o', 0, this, FL_MENU_INACTIVE);
  menu.add("&Project/&Close", FL_COMMAND + 'c', 0, this, FL_MENU_INACTIVE);
  menu.add("&Project/&Bookmark", FL_COMMAND + 'b', 0, this, FL_MENU_INACTIVE);
  // Data
  menu.add("&Data/&Add File", 0, 0, this, FL_MENU_INACTIVE);
  menu.add("&Data/&Add Directory", 0, 0, this, FL_MENU_INACTIVE);
  menu.add("&Data/&Download Data", 0, 0, this, FL_MENU_INACTIVE);
  // Processing
  menu.add("&Processing/&Filters/&Butterworth/&Lowpass", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Processing/&Filters/&Butterworth/&Highpass", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Processing/&Filters/&Butterworth/&Bandpass", 0, 0, this,
           FL_MENU_INACTIVE);
  // Plotting
  menu.add("&Plot/&Single Component/&Time-series", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Single Component/&Spectrum/&Real-Imaginary", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Single Component/&Spectrum/&Amplitude-Phase", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Single Component/&Spectrogram", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Time-series", 0, 0, this, FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Spectrum/&Real-Imaginary", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Spectrum/&Amplitude-Phase", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Spectrogram", 0, 0, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Profile", 0, 0, this, FL_MENU_INACTIVE);
  // Settings
  menu.add("&Settings", 0, 0, this, FL_MENU_INACTIVE);
  // Help
  menu.add("&Help", 0, 0, this, FL_MENU_INACTIVE);
  // About
  menu.add("&About", 0, 0, this, FL_MENU_INACTIVE);
  this->end();
}
}  // namespace pssp
