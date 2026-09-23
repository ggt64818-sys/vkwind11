#pragma once

#include <cstdlib>
#include <cstring>
#include <string>

struct Config {
  bool async_pipeline = true;
  bool pipeline_cache = true;
  bool strict_validation = false;
  int max_frame_latency = 2;
  int force_feature_level = 0; // 0 = auto, 0xb000 = 11.0, 0xa100 = 10.1
  std::string gpu_override;

  void loadFromEnv();
};

inline Config g_config;

void configInit();
