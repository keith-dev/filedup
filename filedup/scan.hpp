#pragma once

#include <string>

//---------------------------------------------------------------------------

struct options_t;
struct file_stamps_t;

void scan(file_stamps_t& file_stamps, const options_t& opts, std::string name);
void show(const file_stamps_t& file_stamps, const options_t& opts);
