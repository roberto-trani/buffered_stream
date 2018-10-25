#ifndef BUFFEREDWRITER_HPP
#define BUFFEREDWRITER_HPP

#include <memory.h>
#include <iostream>
#include <stdexcept>


template <bool use_write_constraint=false>
class BufferedWriter {
private:
    std::ostream * const    p_ostream;
    char * const            p_buffer;
    const std::size_t       s_buffer_size;

    std::size_t             s_cur_pos;
    std::size_t             s_buffer_remaining;
    std::size_t             s_ostream_remaining;

public:
    BufferedWriter(std::ostream * os, std::size_t buffer_size, std::size_t num_bytes_constraint=(std::size_t)-1):
            p_ostream(os),
            p_buffer(new char[buffer_size]),
            s_buffer_size(buffer_size),
            s_cur_pos(0),
            s_buffer_remaining(0),
            s_ostream_remaining(num_bytes_constraint)
    {
        if (!use_write_constraint && num_bytes_constraint != (std::size_t)-1) {
            throw std::runtime_error("The bytes constraint must be not set when used without write constraints");
        }
    }

    ~BufferedWriter() {
        this->flush();
        delete[](this->p_buffer);
    }

    template <typename T>
    void
    put(const T & t) {
        this->write(reinterpret_cast<const char *>(&t), sizeof(T));
    }

    void
    write(const char * src, std::size_t num_bytes) {
        if (this->s_buffer_remaining >= num_bytes) { // check if the bytes to write fit entirely into the buffer
            // copy into the result
            memcpy(&this->p_buffer[this->s_cur_pos], src, num_bytes);

            // update the state
            this->s_cur_pos += num_bytes;
            this->s_buffer_remaining -= num_bytes;

        } else {
            // check the constraints
            if (use_write_constraint && this->s_ostream_remaining < this->s_buffer_size + num_bytes - this->s_buffer_remaining) {
                throw std::runtime_error("The number of remaining bytes is not enough to write the entire data");
            }

            // write the final part into the stream and then copy a new portion of the element into the buffer
            const std::size_t already_written = this->s_buffer_remaining;
            if (already_written > 0) {
                memcpy(&this->p_buffer[this->s_cur_pos], src, already_written);
            }
            if (this->s_cur_pos > 0) {
                this->p_ostream->write(this->p_buffer, this->s_buffer_size);
                if (this->p_ostream->fail()) {
                    throw std::runtime_error("An error occurred during the write");
                }
                if (use_write_constraint) {
                    this->s_ostream_remaining -= this->s_buffer_size;
                }
            }


            // compute how many bytes to copy and to write
            const std::size_t to_write = num_bytes - already_written;

            if (to_write >= this->s_buffer_size) { // check if the remaining part to write fit into the buffer
                // write into the output stream directly from the source
                this->p_ostream->write(src + already_written, to_write);
                if (this->p_ostream->fail()) {
                    throw std::runtime_error("An error occurred during the write. Only something has been written.");
                }

                // update the state
                this->s_cur_pos = 0;
                this->s_buffer_remaining = 0;
                if (use_write_constraint) {
                    this->s_ostream_remaining -= to_write;
                }

            } else {
                // copy into the buffer
                memcpy(this->p_buffer, src + already_written, to_write);

                // update the state
                this->s_cur_pos = to_write;
                this->s_buffer_remaining = this->s_buffer_size - to_write;
            }
        }
    }

    std::size_t
    get_num_bytes_remaining() const {
        return this->s_ostream_remaining;
    }

    void
    flush() {
        if (this->s_cur_pos > 0) {
            this->p_ostream->write(this->p_buffer, this->s_cur_pos);
            if (this->p_ostream->fail()) {
                throw std::runtime_error("An error occurred during the write");
            }
        }
        this->p_ostream->flush();
    }
};

#endif //BUFFEREDWRITER_HPP
