#ifndef BUFFEREDREADER_HPP
#define BUFFEREDREADER_HPP

#include <iostream>
#include <memory.h>


template <bool use_read_constraint=false>
class BufferedReader {
private:
    std::istream * const    p_istream;
    char * const            p_buffer;
    const std::size_t       s_buffer_size;

    std::size_t             s_cur_pos;
    std::size_t             s_buffer_remaining;
    std::size_t             s_istream_remaining;

public:
    BufferedReader(std::istream * is, std::size_t buffer_size, std::size_t num_bytes_constraint=(std::size_t)-1):
            p_istream(is),
            p_buffer(new char[buffer_size]),
            s_buffer_size(buffer_size),
            s_cur_pos(0),
            s_buffer_remaining(0),
            s_istream_remaining(num_bytes_constraint)
    {
        if (!use_read_constraint && num_bytes_constraint != (std::size_t)-1) {
            throw std::runtime_error("The bytes constraint must be not set when used without read constraints");
        }
    }

    ~BufferedReader() {
        delete[](this->p_buffer);
    }

    template <typename T>
    T
    get() {
        char result[sizeof(T)];
        this->read(result, sizeof(T));
        return *(reinterpret_cast<T*>(result));
    }

    void
    read(char * dest, std::size_t num_bytes) {
        if (this->s_buffer_remaining >= num_bytes) { // check if the bytes to read are already into the buffer
            // copy into the destination
            memcpy(dest, &this->p_buffer[this->s_cur_pos], num_bytes);
            // update the state
            this->s_cur_pos += num_bytes;
            this->s_buffer_remaining -= num_bytes;

        } else {
            // check the constraints
            if (use_read_constraint && this->s_buffer_remaining + this->s_istream_remaining < num_bytes) {
                throw std::runtime_error("The number of remaining bytes is not enough to read the entire data");
            }

            // copy the final part into the destination and then read a new portion of the input from the stream
            const std::size_t already_read = this->s_buffer_remaining;
            if (already_read > 0) {
                memcpy(dest, &this->p_buffer[this->s_cur_pos], already_read);
            }

            // compute how many bytes to copy and to read
            const std::size_t to_read = num_bytes - already_read;

            if (to_read >= this->s_buffer_size) { // check if the remaining part to read fit into the buffer
                // copy into the destination directly from the stream
                this->p_istream->read(dest + already_read, to_read);
                if (this->p_istream->fail()) {
                    throw std::runtime_error("An error occurred during the read");
                }

                // update the state
                this->s_cur_pos = 0;
                this->s_buffer_remaining = 0;
                if (use_read_constraint) {
                    this->s_istream_remaining -= to_read;
                }

            } else {
                std::size_t real_read = (!use_read_constraint || this->s_istream_remaining > this->s_buffer_size) ? this->s_buffer_size
                                                                                         : this->s_istream_remaining;
                // read from the stream
                this->p_istream->read(this->p_buffer, real_read);
                if (this->p_istream->fail()) {
                    if (this->p_istream->gcount() >= (int) to_read) {
                        real_read = (std::size_t) this->p_istream->gcount();
                    } else {
                        throw std::runtime_error("An error occurred during the read. Only something has been read.");
                    }
                }

                // copy into the destination
                memcpy(dest + already_read, this->p_buffer, to_read);
                // update the state
                this->s_cur_pos = to_read;
                this->s_buffer_remaining = real_read - to_read;
                if (use_read_constraint) {
                    this->s_istream_remaining -= real_read;
                }
            }
        }
    }

    std::size_t
    get_num_bytes_remaining() const {
        return this->s_istream_remaining;
    }

    void
    increase_num_bytes_constraint(std::size_t num_bytes_to_read) {
        if (use_read_constraint) {
            this->s_istream_remaining += num_bytes_to_read;
        }
    }
};

#endif //BUFFEREDREADER_HPP
