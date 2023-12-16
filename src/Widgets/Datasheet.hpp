// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_DATASHEET_HPP_20231215_1255
#define PSSP_DATASHEET_HPP_20231215_1255
#pragma once

// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl.H>
#include <FL/fl_draw.H>
// This is needed for the window() function to prevent
// disappearing cursors
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Table.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::unique_ptr
#include <memory>
// std::ostringstream
#include <sstream>
// std::string
#include <string>

namespace pssp {
namespace datasheet {
// Temporary limits for prototyping the datasheet
constexpr int max_row{30};
constexpr int max_col{30};
const std::string edit_chars{"0123456789+-\r\n"};
// May want to move Geometry object into full pssp namespace at some point
struct Geometry {
  // cppcheck-suppress unusedStructMember
  int x_pos{0};
  // cppcheck-suppress unusedStructMember
  int y_pos{0};
  // cppcheck-suppress unusedStructMember
  int width{0};
  // cppcheck-suppress unusedStructMember
  int height{0};
};

struct Cell {
  // cppcheck-suppress unusedStructMember
  Geometry full_box{};
  // cppcheck-suppress unusedStructMember
  Geometry text_box{};
  Fl_Font font{FL_HELVETICA};
  Fl_Color text_color{FL_BLACK};
  Fl_Color box_color{FL_GRAY};
  Fl_Boxtype box_type{FL_THIN_UP_BOX};
  Fl_Align alignment{FL_ALIGN_CENTER};
  // cppcheck-suppress unusedStructMember
  std::string text{""};
};
}  // namespace datasheet

class Datasheet : public Fl_Table {
public:
  Datasheet();
  void set_value_hide();
  void start_editing(int row, int col);
  void done_editing();

protected:
  void draw_cell(TableContext context, int row = 0, int col = 0, int x_pos = 0,
                 int y_pos = 0, int width = 0, int height = 0) FL_OVERRIDE;

  // table event callback (instance)
  void event_callback2();

  // table event callback (static) [trick to use event_callback2]
  static void event_callback(Fl_Widget *, void *v) {
    reinterpret_cast<Datasheet *>(v)->event_callback2();
  }

  // Input callback to hide value
  static void input_cb(Fl_Widget *, void *v) {
    reinterpret_cast<Datasheet *>(v)->set_value_hide();
  }

private:
  // cppcheck-suppress unusedStructMember
  int edit_row{0};
  // cppcheck-suppress unusedStructMember
  int edit_col{0};
  std::unique_ptr<Fl_Int_Input> input{};
  // cppcheck-suppress unusedStructMember
  // Temporary while proto-typing editing
  int values[datasheet::max_row][datasheet::max_col];
  void draw_generic_cell(const datasheet::Cell &cell);
  void draw_header_cell(int x_pos, int y_pos, int width, int height,
                        const std::string &text);
};
}  // namespace pssp

#endif
