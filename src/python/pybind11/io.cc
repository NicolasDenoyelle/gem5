#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "mem/remote/io.hh"

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

void
pybind_init_io(py::module &m_native)
{
    py::module m = m_native.def_submodule("io", "Communication handles");

    py::class_<Buffer> buffer(m, "Buffer",
                              "Buffer class used to read and write from io.");
    buffer.def(py::init<size_t>(),
              "Allocate a buffer.\n"
              "size: The buffer size in bytes.",
              py::arg("size"));
    buffer.def("ptr", &Buffer::ptr, "Get buffer raw pointer.");
    buffer.def("size", &Buffer::ptr, "Get buffer size.");

    py::class_<Client> client(m, "Client");
    client.def(py::init<pid_t>(),
               "Initialize client. uid must the same as uid used for the "
               "server this client is trying to reach.",
               py::arg("uid")=0);
    client.def("receive",
               [](const Client& c, Buffer& b) -> ssize_t {
                   return c.recvMsg(b.ptr(), b.size());
               },
               "Receive next message from server in a byte array and returns "
               "message id.");
    client.def("send",
               (ssize_t (Client::*)(const MBind&)) &Client::sendMsg,
               "Send a memory access to server.");
    client.def("flush",
               (ssize_t (Client::*)(void)) &Client::flush,
               "Flush send buffer to immediately send messages.");
}
