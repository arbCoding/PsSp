// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_DATASHEET_HPP_20231215_1255
#define PSSP_DATASHEET_HPP_20231215_1255
#pragma once

/* fltk https://www.fltk.org/doc-1.4 */
#include <FL/Fl.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Table.H>
/* spdlog https://github.com/gabime/spdlog */
#include <spdlog/spdlog.h>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::unique_ptr
#include <memory>

namespace pssp {
class Datasheet : public Fl_Table {
public:
  Datasheet();
private:

};
} // namespace pssp

#endif
