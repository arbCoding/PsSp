// Copyright 2023 Alexander R. Blanchette

#include "Main.hpp"

namespace pssp {
Main_Window::Main_Window() : Fl_Window(0, 0, name_.c_str()) {
  this->begin();
  resizable(this);
  int x_start{};
  int y_start{};
  int width{};
  int height{};
  Fl::screen_work_area(x_start, y_start, width, height);
  this->resize(x_start, y_start, width, height);
  make_menu();
#if defined(__APPLE__)
  int menu_shift{0};
#else
  int menu_shift{menu.h()};
#endif
  plot =
      std::make_unique<Fl_Chart>(10, menu_shift + 10, width - 20, 400, "Chart");
  plot->color(FL_WHITE);
  plot->type(FL_LINE_CHART);
  plot->bounds(0, 1000);
  for (int i{0}; i < 1000; ++i) {
    plot->add(i, nullptr, FL_GREEN);
  }
  plot->redraw();
  menu.resize(0, 0, width, menu.h());
  make_tty();
  this->end();
}

void Main_Window::make_tty() {
  // Debug terminal
  constexpr int height{200};
  debug_tty =
      std::make_unique<Fl_Terminal>(0, this->h() - height, this->w(), height);
  debug_tty->begin();
  constexpr int font_size{14};
  debug_tty->textsize(font_size);
  debug_tty->redraw_style(Fl_Terminal::NO_REDRAW);
  constexpr int num_columns{80};
  debug_tty->display_columns(num_columns);
  append_tty("\033[31mDebug terminal\033[0m\n");
  append_tty("\033[33mStarting Passive-source Seismic-processing!\033[0m\n");
  append_tty("Awaiting Commands...\n");
  debug_tty->end();
}

void Main_Window::make_menu() {
  // Program
  menu.add("&Program/&Quit", FL_COMMAND + 'q', quit_cb, this);
  // Project
  menu.add("&Project/&New", FL_COMMAND + 'n', nullptr, this, FL_MENU_INACTIVE);
  menu.add("&Project/&Load", FL_COMMAND + 'o', nullptr, this, FL_MENU_INACTIVE);
  menu.add("&Project/&Close", FL_COMMAND + 'c', nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Project/&Bookmark", FL_COMMAND + 'b', nullptr, this,
           FL_MENU_INACTIVE);
  // Data
  menu.add("&Data/&Add File", 0, nullptr, this, FL_MENU_INACTIVE);
  menu.add("&Data/&Add Directory", 0, nullptr, this, FL_MENU_INACTIVE);
  menu.add("&Data/&Download Data", 0, nullptr, this, FL_MENU_INACTIVE);
  // Processing
  menu.add("&Processing/&Filters/&Butterworth/&Lowpass", 0, nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Processing/&Filters/&Butterworth/&Highpass", 0, nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Processing/&Filters/&Butterworth/&Bandpass", 0, nullptr, this,
           FL_MENU_INACTIVE);
  // Plotting
  menu.add("&Plot/&Single Component/&Time-series", 0, nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Single Component/&Spectrum/&Real-Imaginary", 0, nullptr,
           this, FL_MENU_INACTIVE);
  menu.add("&Plot/&Single Component/&Spectrum/&Amplitude-Phase", 0, nullptr,
           this, FL_MENU_INACTIVE);
  menu.add("&Plot/&Single Component/&Spectrogram", 0, nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Time-series", 0, nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Spectrum/&Real-Imaginary", 0, nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Spectrum/&Amplitude-Phase", 0, nullptr,
           this, FL_MENU_INACTIVE);
  menu.add("&Plot/&Three Component/&Spectrogram", 0, nullptr, this,
           FL_MENU_INACTIVE);
  menu.add("&Plot/&Profile", 0, nullptr, this, FL_MENU_INACTIVE);
  // Settings
  menu.add("&Settings", 0, nullptr, this, FL_MENU_INACTIVE);
  // Help
  menu.add("&Help", 0, nullptr, this, FL_MENU_INACTIVE);
  // About
  menu.add("&About", 0, nullptr, this, FL_MENU_INACTIVE);
}

void Main_Window::append_tty(const char *msg) { debug_tty->append(msg); }

void Main_Window::quit_cb(Fl_Widget *menu, void *junk) {
  (void)junk;
  // reinterpret_cast is unnecessary, but I wanted to figure it out
  auto *window = reinterpret_cast<Main_Window *>(menu->parent()->as_window());
  window->hide();
}
}  // namespace pssp
