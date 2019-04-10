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

/* TODO: MAP-532 add units tests */
#include <assert.h>
#include <string.h>
#include <libswiftnav/fifo_byte.h>

#define INDEX_MASK(p_fifo) ((p_fifo)->buffer_size - 1)
#define LENGTH(p_fifo) \
  ((fifo_size_t)((p_fifo)->write_index - (p_fifo)->read_index))
#define SPACE(p_fifo) ((fifo_size_t)((p_fifo)->buffer_size - LENGTH(p_fifo)))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*
 * This file implements a lock-free single-producer, single-consumer FIFO
 * using a circular buffer.
 */

/** Initialize a FIFO.
 *
 * \param fifo        fifo_t struct to use.
 * \param buffer      Buffer to use for the FIFO data. Must remain valid
 *                    while the FIFO is in use.
 * \param buffer_size Size of buffer. Must be a power of two.
 */
void fifo_init(fifo_t *fifo, uint8_t *buffer, fifo_size_t buffer_size) {
  /* Require buffer_size to be a power of two */
  assert((buffer_size & (buffer_size - 1)) == 0);

  fifo->read_index = 0;
  fifo->write_index = 0;
  fifo->buffer_size = buffer_size;
  fifo->buffer = buffer;
}

/** Get the length for a FIFO.
 *
 * \note If called from the consumer thread, the length is a lower bound.
 * If called from the producer thread, the length is an upper bound.
 *
 * \param fifo        fifo_t struct to use.
 *
 * \return Number of bytes that may be read from the FIFO.
 */
fifo_size_t fifo_length(fifo_t *fifo) { return LENGTH(fifo); }

/** Get the space for a FIFO.
 *
 * \note If called from the consumer thread, the space is an upper bound.
 * If called from the producer thread, the space is a lower bound.
 *
 * \param fifo        fifo_t struct to use.
 *
 * \return Number of bytes that may be written to the FIFO.
 */
fifo_size_t fifo_space(fifo_t *fifo) { return SPACE(fifo); }

/** Read data from a FIFO.
 *
 * \note This function should only be called from a single consumer thread.
 *
 * \param fifo        fifo_t struct to use.
 * \param buffer      Output buffer.
 * \param length      Maximum number of bytes to read.
 *
 * \return Number of bytes read from the FIFO.
 */
fifo_size_t fifo_read(fifo_t *fifo, uint8_t *buffer, fifo_size_t length) {
  fifo_size_t read_length = fifo_peek(fifo, buffer, length);

  if (read_length > 0) {
    read_length = fifo_remove(fifo, read_length);
  }

  return read_length;
}

/** Read data from a FIFO without removing it.
 *
 * \note This function should only be called from a single consumer thread.
 *
 * \param fifo        fifo_t struct to use.
 * \param buffer      Output buffer.
 * \param length      Maximum number of bytes to read.
 *
 * \return Number of bytes read from the FIFO.
 */
fifo_size_t fifo_peek(fifo_t *fifo, uint8_t *buffer, fifo_size_t length) {
  /* Atomic read of write_index to get fifo_length */
  fifo_size_t fifo_length = LENGTH(fifo);

  fifo_size_t read_length = MIN(length, fifo_length);
  if (read_length > 0) {
    fifo_size_t read_index_masked = fifo->read_index & INDEX_MASK(fifo);
    if (read_index_masked + read_length <= fifo->buffer_size) {
      /* One contiguous block */
      memcpy(buffer, &fifo->buffer[read_index_masked], read_length);
    } else {
      /* Two contiguous blocks */
      fifo_size_t copy_len_a = fifo->buffer_size - read_index_masked;
      memcpy(buffer, &fifo->buffer[read_index_masked], copy_len_a);
      memcpy(&buffer[copy_len_a], fifo->buffer, read_length - copy_len_a);
    }
  }

  return read_length;
}

/** Remove data from a FIFO.
 *
 * \note This function should only be called from a single consumer thread.
 *
 * \param fifo        fifo_t struct to use.
 * \param length      Maximum number of bytes to remove.
 *
 * \return Number of bytes removed from the FIFO.
 */
fifo_size_t fifo_remove(fifo_t *fifo, fifo_size_t length) {
  /* Atomic read of write_index to get fifo_length */
  fifo_size_t fifo_length = LENGTH(fifo);

  fifo_size_t read_length = MIN(length, fifo_length);
  if (read_length > 0) {
    /* Atomic write of read_index */
    fifo->read_index += read_length;
  }

  return read_length;
}

/** Write data to a FIFO.
 *
 * \note This function should only be called from a single producer thread.
 *
 * \param fifo        fifo_t struct to use.
 * \param buffer      Input buffer.
 * \param length      Maximum number of bytes to write.
 *
 * \return Number of bytes written to the FIFO.
 */
fifo_size_t fifo_write(fifo_t *fifo,
                       const uint8_t *buffer,
                       fifo_size_t length) {
  /* Atomic read of read_index to get fifo_space */
  fifo_size_t fifo_space = SPACE(fifo);

  fifo_size_t write_length = MIN(length, fifo_space);
  if (write_length > 0) {
    fifo_size_t write_index_masked = fifo->write_index & INDEX_MASK(fifo);
    if (write_index_masked + write_length <= fifo->buffer_size) {
      /* One contiguous block */
      memcpy(&fifo->buffer[write_index_masked], buffer, write_length);
    } else {
      /* Tow contiguous blocks */
      fifo_size_t copy_len_a = fifo->buffer_size - write_index_masked;
      memcpy(&fifo->buffer[write_index_masked], buffer, copy_len_a);
      memcpy(fifo->buffer, &buffer[copy_len_a], write_length - copy_len_a);
    }

    /* Atomic write of write_index */
    fifo->write_index += write_length;
  }

  return write_length;
}
