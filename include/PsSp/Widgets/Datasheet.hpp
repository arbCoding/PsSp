// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_DATASHEET_HPP_20231215_1255
#define PSSP_DATASHEET_HPP_20231215_1255
#pragma once
// PsSp
#include "PsSp/Managers/SheetManager.hpp"
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

// I think the Datasheet is sufficiently efficient. The issue of raw-pointers
// vs. smart-pointer for exit time is a topic for the far future.
//
// The focus now is to refactor the code to make it cleaner and more robust.

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

// This class is getting too busy, the inputs should be broken into a
// separate class (Fl_Group holding inputs) that handles all the input
// logic separate from the datasheet
//
// I may even want the cell logic to be separated into a cell_drawer or
// cell_manager class... Need to think more about this.
//
// Perhaps manage the arrays separately as well (Array manager or something).
// SheetManager is a good name (DataManager is too general for what I have in
// mind).
//
// Need to move away from arrays to vectors (so that they are resizable).
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
  int max_col{0};
  // cppcheck-suppress unusedStructMember
  int max_row{0};
  std::unique_ptr<Fl_Input> input{};
  std::unique_ptr<Fl_Int_Input> input_int{};
  std::unique_ptr<Fl_Float_Input> input_float{};
  std::unique_ptr<SheetManager> sheet_manager{};
  void make_inputs();
  static void draw_generic_cell(const datasheet::Cell &cell);
  static void draw_header_cell(structs::Geometry *geo, const std::string &text);
};
}  // namespace pssp

#endif
