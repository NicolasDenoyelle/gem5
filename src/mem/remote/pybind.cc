#include "pybind11/pybind11.h"

#include "message.hh"

#include "message.cc"
#include "io.hh"

#include "io.cc"

namespace py = pybind11;

class Buffer {
  private:
    void* _ptr;
    size_t _size;
    bool _owned;

  public:
    Buffer(): _ptr(NULL), _size(0), _owned(false) {}
    Buffer(void* p, size_t s): _ptr(p), _size(s), _owned(false) {}
    Buffer(size_t s): _size(s), _owned(true) { _ptr = malloc(s); }
    ~Buffer() { if (_owned) free(_ptr); }
    void* ptr() { return _ptr; }
    size_t size() { return _size; }
};

PYBIND11_MODULE(gem5, gem5) {
    gem5.doc() = "Python wrapper for communications with gem5 memory "
        "controller.";

    py::class_<Buffer> buffer(gem5, "Buffer",
                              "Buffer class used to read and write from io.");
    buffer.def(py::init<size_t>(),
              "Allocate a buffer.\n"
              "size: The buffer size in bytes.",
              py::arg("size"));
    buffer.def("ptr", &Buffer::ptr, "Get buffer raw pointer.");
    buffer.def("size", &Buffer::ptr, "Get buffer size.");

#include "message.pybind.cc"
#include "io.pybind.cc"

}
