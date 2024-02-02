// Copyright 2023-2024 ALexander R. Blanchette
/*!
  \file PsSp/Utility/Constants.hpp
  \brief Constant definitions for PsSp.
  \author Alexander R. Blanchette
  This file contains constants that are relevant to PsSp.

  \todo So far these are only related to SAC records and are used to prototype
  the interface. In the future, they'll be supplied by the sac-format library
  and not needed to be defined here.
 */
#ifndef PSSP_CONSTANTS_HPP_20231218_1305
#define PSSP_CONSTANTS_HPP_20231218_1305
#pragma once

namespace pssp::constants {
//! Number of float columns for SAC records.
constexpr int sac_float{39};
//! Number of double columns for SAC records.
constexpr int sac_double{22};
//! Number of integer columns for SAC records.
constexpr int sac_int{26};
//! Number of boolean columns for SAC records.
constexpr int sac_bool{4};
//! Number of string columns for SAC records.
constexpr int sac_string{22 + 1};
//! Number of possible data vectors for a SAC record.
constexpr int sac_data{2};
}  // namespace pssp::constants
#endif
