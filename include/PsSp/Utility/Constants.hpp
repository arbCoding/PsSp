// Copyright 2023 ALexander R. Blanchette

#ifndef PSSP_CONSTANTS_HPP_20231218_1305
#define PSSP_CONSTANTS_HPP_20231218_1305
#pragma once

namespace pssp::constants {
// Number of columns to keep track of for each type
constexpr int sac_float{39};
constexpr int sac_double{22};
constexpr int sac_int{26};
constexpr int sac_bool{4};
// +1 as reminber for kEvNm 4 word instead of 2
constexpr int sac_string{22 + 1};
constexpr int sac_data{2};
}  // namespace pssp::constants
#endif
