#pragma once

void* grid_thread_func(void* arg);
void* weather_avg_thread_func(void* arg);
void* weather_raw_thread_func(void* arg);
void* influx_thread_func(void* arg);