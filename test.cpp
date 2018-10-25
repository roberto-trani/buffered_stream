#include <assert.h>
#include <sstream>
#include "BufferedReader.hpp"
#include "BufferedWriter.hpp"


void
test() {
    std::stringstream sstream;

    // START Write - set buffer size to 5
    BufferedWriter<false> bufferedWriter(&sstream, 5);

    std::size_t size1 = 10;
    // put std::size_t
    bufferedWriter.put<std::size_t>(size1);
    // put multiple ints
    for (int i = 0; i < size1; ++i) {
        bufferedWriter.put<int>(i);
    }

    std::size_t size2 = 10;
    // put std::size_t
    bufferedWriter.put<std::size_t>(size2);
    // put multiple chars
    for (int i = 0; i < size2; ++i) {
        char c = 'a' + (char) i;
        bufferedWriter.put<char>(c);
    }

    // write raw string
    const char raw_string[] = "hello world";
    const std::size_t raw_string_size = sizeof(raw_string);
    bufferedWriter.write(raw_string, raw_string_size);

    // flush
    bufferedWriter.flush();
    // END Write


    // START Read - set buffer size to 3
    BufferedReader<true> bufferedReader(&sstream, 3, sizeof(std::size_t) * 2);

    // read std::size_t
    assert(bufferedReader.get<std::size_t>() == size1);
    // read multiple ints
    bufferedReader.increase_num_bytes_constraint(size1 * sizeof(int));
    for (int i = 0; i < size1; ++i) {
        assert(bufferedReader.get<int>() == i);
    }

    // read std::size_t
    assert(bufferedReader.get<std::size_t>() == size2);
    // read multiple chars
    bufferedReader.increase_num_bytes_constraint(size2 * sizeof(char));
    for (int i = 0; i < size2; ++i) {
        assert(bufferedReader.get<char>() == 'a' + (char) i);
    }

    // check the constraints
    assert(bufferedReader.get_num_bytes_remaining() == 0);
    // read raw string
    char str_buffer[raw_string_size];
    bufferedReader.increase_num_bytes_constraint(raw_string_size);
    bufferedReader.read(str_buffer, raw_string_size);  // raw_string_size already includes the \0 at the end
    assert(strcmp(str_buffer, raw_string) == 0);
    // END Read
}


int main(int argc, char **argv) {
    test();

    return 0;
}
