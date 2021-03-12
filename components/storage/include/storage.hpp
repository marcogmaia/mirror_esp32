/**
 * @file storage.hpp
 * @author Marco A. G. Maia (marcogmaia@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-03-12
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

namespace storage {

void init();

void get_values(uint8_t (&bris)[2]);

void set_values(const uint8_t (&bris)[2]);

}  // namespace storage