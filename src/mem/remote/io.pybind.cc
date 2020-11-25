py::module_ io = gem5.def_submodule("io", "Communication handles");

py::class_<Client> client(io, "Client");
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
