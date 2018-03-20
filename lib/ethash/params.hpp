// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <cstdint>
#include <cstddef>

namespace ethash
{
static constexpr uint32_t epoch_length = 30000;

constexpr static uint64_t light_cache_init_size = 1 << 24;
constexpr static uint64_t light_cache_growth = 1 << 17;
constexpr static size_t light_cache_rounds = 3;
constexpr static uint64_t full_dataset_init_size = 1 << 30;
constexpr static uint64_t full_dataset_growth = 1 << 23;
constexpr static uint32_t full_dataset_item_parents = 256;

constexpr static size_t mixhash_size = 64;

}