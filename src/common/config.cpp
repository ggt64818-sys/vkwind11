#include "config.h"
#include "logging.h"

void Config::loadFromEnv() {
  const char* v;

  v = getenv("VKWIND11_ASYNC_PIPELINE");
  if (v) async_pipeline = (strcmp(v, "1") == 0 || strcmp(v, "true") == 0);

  v = getenv("VKWIND11_PIPELINE_CACHE");
  if (v) pipeline_cache = (strcmp(v, "1") == 0 || strcmp(v, "true") == 0);

  v = getenv("VKWIND11_STRICT_VALIDATION");
  if (v) strict_validation = (strcmp(v, "1") == 0 || strcmp(v, "true") == 0);

  v = getenv("VKWIND11_MAX_FRAME_LATENCY");
  if (v) max_frame_latency = atoi(v);

  v = getenv("VKWIND11_FEATURE_LEVEL");
  if (v) force_feature_level = strtol(v, nullptr, 0);

  v = getenv("VKWIND11_GPU");
  if (v) gpu_override = v;

  VKWIND11_LOG_INFO("Config: async_pipeline=%d, pipeline_cache=%d, max_frame_latency=%d",
    async_pipeline, pipeline_cache, max_frame_latency);
}

void configInit() {
  g_config.loadFromEnv();
}
