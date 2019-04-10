/*
 * Copyright (C) 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef uint32_t fifo_size_t;

typedef struct {
  fifo_size_t read_index;
  fifo_size_t write_index;
  fifo_size_t buffer_size;
  uint8_t *buffer;
} fifo_t;

void fifo_init(fifo_t *fifo, uint8_t *buffer, fifo_size_t buffer_size);

fifo_size_t fifo_length(fifo_t *fifo);
fifo_size_t fifo_space(fifo_t *fifo);

fifo_size_t fifo_read(fifo_t *fifo, uint8_t *buffer, fifo_size_t length);
fifo_size_t fifo_peek(fifo_t *fifo, uint8_t *buffer, fifo_size_t length);
fifo_size_t fifo_remove(fifo_t *fifo, fifo_size_t length);

fifo_size_t fifo_write(fifo_t *fifo, const uint8_t *buffer, fifo_size_t length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FIFO_H */
