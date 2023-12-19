// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_DATASHEET_HPP_20231215_1255
#define PSSP_DATASHEET_HPP_20231215_1255
#pragma once
// PsSp
#include "PsSp/Utility/Constants.hpp"
#include "PsSp/Utility/Enums.hpp"
#include "PsSp/Utility/Structs.hpp"
// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl.H>
#include <FL/fl_draw.H>
// This is needed for the window() function to prevent
// disappearing cursors
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Table.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
#include <array>
// std::unique_ptr
#include <memory>
// std::ostringstream
#include <sstream>
// std::string
#include <string>

// Note: the debug build of this on my old hp laptop
// can easily handle a datasheet 138*1'000'000 (all integers!)
// (138 is an upper-limit on the number of sac values
// that would be in the datasheet for a trace) (~1 second startup)
//
// With 10'000'000 rows (all integers!) ~12 second startup (release ~3 seconds)
//
// That is far larger than any project is likely to be (on the upper-end
// I expect maybe 100'000, which would be a HUGE project).

// Updated Notes:
// For 10'000'000 rows (100x ultra-high end for a project)
// With things split between string, int, float, double, bool the way it will
// be in the actual workflow
// It takes roughly 10 seconds for the debug version (clang) to boot up
// (and about as long to close)
// (When everything was strings it wouldn't boot, not enough ram)
// Release version (clang) is ~5 seconds to boot up, almost instant close
//
// macOS debug is fine with 10'000'000
// macOS release seg faults with 10'000'000
//
// Question: If I use raw pointers instead of smart pointers, does it close
// faster? In theory the OS should handle cleanup in that scenario, so exiting
// the program should be a lot faster, while still safe. Considered bad practice
// in modern times, but maybe for some things it would be fine.

namespace pssp {
namespace datasheet {
struct Spec {
  // cppcheck-suppress unusedStructMember
  int height{0};
  // cppcheck-suppress unusedStructMember
  int header_height{0};
  // cppcheck-suppress unusedStructMember
  int width{0};
  // cppcheck-suppress unusedStructMember
  int header_width{0};
};
constexpr int font_size{14};
constexpr int cell_buffer{3};
constexpr int max_row{100};
// Number of parameter for a sac trace
constexpr int max_col{num_fields};
constexpr int max_chars{10};
const std::string edit_chars{"0123456789+-\r\n"};
struct Cell {
  // cppcheck-suppress unusedStructMember
  structs::Geometry full_box{};
  // cppcheck-suppress unusedStructMember
  structs::Geometry text_box{};
  Fl_Font font{FL_HELVETICA};
  Fl_Color text_color{FL_BLACK};
  Fl_Color box_color{FL_GRAY};
  Fl_Boxtype box_type{FL_THIN_UP_BOX};
  Fl_Align alignment{FL_ALIGN_CENTER};
  // cppcheck-suppress unusedStructMember
  std::string text{};
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
  static void event_callback(Fl_Widget *widget, void *datasheet) {
    (void)widget;
    reinterpret_cast<Datasheet *>(datasheet)->event_callback2();
  }

  // Input callback to hide value
  static void input_cb(Fl_Widget *widget, void *datasheet) {
    (void)widget;
    reinterpret_cast<Datasheet *>(datasheet)->set_value_hide();
  }

private:
  // cppcheck-suppress unusedStructMember
  int edit_row{0};
  // cppcheck-suppress unusedStructMember
  int edit_col{0};
  // cppcheck-suppress unusedStructMember
  std::unique_ptr<Fl_Input> input{};
  std::unique_ptr<Fl_Int_Input> input_int{};
  std::unique_ptr<Fl_Float_Input> input_float{};
  // Temporary while proto-typing editing
  std::array<std::array<std::string, constants::sac_string>, datasheet::max_row>
      // cppcheck-suppress unusedStructMember
      values_string{};
  std::array<std::array<float, constants::sac_float>, datasheet::max_row>
      // cppcheck-suppress unusedStructMember
      values_float{};
  std::array<std::array<double, constants::sac_double>, datasheet::max_row>
      // cppcheck-suppress unusedStructMember
      values_double{};
  std::array<std::array<int, constants::sac_int>, datasheet::max_row>
      // cppcheck-suppress unusedStructMember
      values_int{};
  std::array<std::array<bool, constants::sac_bool>, datasheet::max_row>
      // cppcheck-suppress unusedStructMember
      values_bool{};
  void make_inputs();
  void make_generic_column(int col);
  void make_string_column(const trace_info &info, int col);
  void make_int_column(const trace_info &info, int col);
  void make_float_column(const trace_info &info, int col);
  void make_double_column(const trace_info &info, int col);
  void make_bool_column(const trace_info &info, int col);
  void make_sheet();
  static void draw_generic_cell(const datasheet::Cell &cell);
  static void draw_header_cell(structs::Geometry *geo, const std::string &text);
};
}  // namespace pssp

#endif
