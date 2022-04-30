#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace {


const uint8_t white_threshold = 0x0f;
const int vga_bitmap_size { 64000 };
const int c64_bitmap_size { 8000 };

class AutoRel
{
    private:
        int _fd { -1 };

    public:
        AutoRel(int fd) : _fd(fd)
        {
            if (_fd == -1) {
                throw std::runtime_error("Unable to open file!");
            }
        };
        ~AutoRel()
        {
            if (_fd == -1) {
                return;
            }

            this->close();
        };

        void close()
        {
            if (::close(_fd) == -1)
            {
                throw std::runtime_error("Unable to close file!");
            }
            _fd = -1;
        }

        int get() const
        {
            return _fd;
        }
};

class AutoRelMmap
{
    private:
        void *_mem { nullptr };
        size_t _len { 0 };

    public:
        AutoRelMmap(void *mem, const size_t len) : _mem(mem), _len(len)
        {
            if (_len == 0) {
                throw std::runtime_error("Len is zero!");
            }
            if (!is_ok()) {
                throw std::runtime_error("Memory is NULL or error in allocation!");
            }
        };

        ~AutoRelMmap()
        {
            if (!is_ok()) {
                return;
            }

            munmap(_mem, _len);
        }

        void *get_mem() const
        {
            return _mem;
        }

        bool is_ok() const
        {
            return (_mem != nullptr && _mem != MAP_FAILED);
        }
};

};

int main (int argc, char **argv)
{
    int ret = -1;
    if (argc < 3) {
        return EXIT_FAILURE;
    }

    const char *const in_file = argv[1];
    const char *const out_file = argv[2];

    try {

        auto a_fd = AutoRel { open(in_file, O_RDONLY) };
        auto a_fd_map = AutoRelMmap { mmap(NULL, vga_bitmap_size, PROT_READ, MAP_FILE | MAP_SHARED, a_fd.get(), 0), vga_bitmap_size };

        auto a_fd_out = AutoRel { open(out_file, O_RDWR | O_CREAT | O_TRUNC) };
        ret = ::ftruncate(a_fd_out.get(), c64_bitmap_size);
        if (ret == -1) {
            return EXIT_FAILURE;
        }
        auto a_out_map = AutoRelMmap { mmap(NULL, c64_bitmap_size, PROT_WRITE | PROT_READ, MAP_FILE | MAP_SHARED, a_fd_out.get(), 0),
                                    c64_bitmap_size };

        const uint8_t *const img = static_cast<const uint8_t *>(a_fd_map.get_mem());
        uint8_t *actual_pixel = static_cast<uint8_t *>(a_out_map.get_mem());

        // 320 * 200
        for (int line_off = 0; line_off < 200; line_off += 8) {
            const uint8_t *const start_line = &img[line_off * 320];

            for (int p = 0; p < 320; p += 8) {
                // 8 * 8 matrix
                for (int i = 0; i < 8; ++i) {
                    const uint8_t *const actual_line = &start_line[i * 320];
                    for (int pixel_off = 0; pixel_off < 8; ++pixel_off)
                    {
                        // 0 - black
                        // 1 - white
                        uint8_t pixel = actual_line[pixel_off + p];
                        if (pixel == 0x00) {
                            pixel = 0;
                        } else if (pixel > white_threshold) {
                            pixel = 0;
                        } else {
                            pixel = 1;
                        }

                        *actual_pixel = (*actual_pixel << 1) | pixel;
                    }
                    ++actual_pixel;
                }
            }
        }

    } catch(const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
