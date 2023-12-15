// Copyright 2023 Alexander R. Blanchette

#include "Main.hpp"

namespace pssp {
Main_Window::Main_Window() : Fl_Double_Window(0, 0, name_.c_str()) {
  make_tty();
  spdlog::trace("Building \033[1mMain_Window\033[0m.");
  this->begin();
  resizable(this);
  // Minimum window size width/height
  this->size_range(300, 300);
  int x_start{};
  int y_start{};
  int width{};
  int height{};
  Fl::screen_work_area(x_start, y_start, width, height);
  this->resize(x_start, y_start, width, height);
  make_menu();
  menu.resize(0, 0, width, menu.h());
  status_bar_ = std::make_unique<Status_Bar>(this->h(), this->w(), menu.h());
#if defined(__APPLE__)
  int menu_shift{0};
#else
  int menu_shift{menu.h()};
#endif
  gridspace_ = std::make_unique<Fl_Grid>(0, menu_shift, this->w(),
                                         this->h() - menu_shift - menu.h());
  gridspace_->begin();
  gridspace_->add(debug_tty.get());
  gridspace_->show_grid(1); // 1 to show guide lines
  gridspace_->layout(10, 10, 1, 1);
  list_ = std::make_unique<Fl_Box>(0, 0, 0, 0, "List");
  list_->box(FL_BORDER_BOX);
  list_->color(FL_WHITE);
  datasheet_ = std::make_unique<Datasheet>();
  gridspace_->widget(debug_tty.get(), 7, 0, 3, 10);
  gridspace_->widget(list_.get(), 0, 0, 7, 2);
  gridspace_->widget(datasheet_.get(), 0, 2, 7, 8);
  gridspace_->end();
  this->end();
  this->resizable(status_bar_.get());
  this->resizable(datasheet_.get());
  this->resizable(gridspace_.get());
  about_window_ = std::make_unique<About_Window>();
  about_window_->hide();
  spdlog::trace("Done making \033[1mMain_Window\033[0m.");
}

void Main_Window::make_tty() {
  // Debug terminal
  debug_tty = std::make_unique<Fl_Terminal>(0, 0, 0, 0);
  sink = std::make_shared<Console_Sink_mt>(debug_tty.get());
  logger = std::make_shared<spdlog::logger>("tty logger", sink);
  spdlog::set_default_logger(logger);
  // levels are critical, error, warn, info, debug, trace
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("\33[1m\33[32m[%Y-%m-%d %T]\33[33m[%l]\33[36m[thread %t]\33[0m %v");
  debug_tty->begin();
  constexpr int font_size{14};
  debug_tty->textsize(font_size);
  debug_tty->redraw_style(Fl_Terminal::NO_REDRAW);
  constexpr int num_columns{80};
  debug_tty->display_columns(num_columns);
  spdlog::trace("Logger started.");
  debug_tty->end();
  resizable();
}

void Main_Window::make_menu() {
  spdlog::trace("Making \033[1mMenu\033[0m.");
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
  menu.add("&About", 0, about_cb, this);
  spdlog::trace("Done making \033[1mMenu\033[0m.");
}

void Main_Window::append_tty(const char *msg) { debug_tty->append(msg); }

void Main_Window::quit_cb(Fl_Widget *menu, void *junk) {
  (void)junk;
  // reinterpret_cast is unnecessary, but I wanted to figure it out
  auto *window = reinterpret_cast<Main_Window *>(menu->parent()->as_window());
  window->hide();
}

void Main_Window::show_about() { about_window_->show(); }

void Main_Window::about_cb(Fl_Widget *menu, void *junk) {
  (void)junk;
  auto *window = reinterpret_cast<Main_Window *>(menu->parent()->as_window());
  window->show_about();
}
}  // namespace pssp
