py::module_ msg = gem5.def_submodule("message",
                                     "Message types impelementations.");

py::class_<MemAccess> memaccess(msg, "MemAccess",
                                "Message implementation for memory access "
                                "performed on gem5 side.");

py::enum_<MemAccess::Type>(memaccess, "Type")
.value("R", MemAccess::Type::R, "read memory access")
.value("W", MemAccess::Type::W, "write memory access")
.value("RW", MemAccess::Type::RW, "read and write memory access")
.export_values();

memaccess.def(py::init<MemAccess::Type, uint64_t, uint64_t, uint8_t,
              uint32_t>(),
              "Create and initialize a memory access sample.\n"
              "type: The type of memory access: MemAccess.Type.<R/W/RW>\n"
              "addr: The virtual address of the memory access.\n"
              "numa: The logical index of the numa node being accessed.\n"
              "tid: The logical index of hardware thread accessing "
              "data.\n",
              py::arg("type")=MemAccess::Type::R,
              py::arg("tick")=0,
              py::arg("addr")=0,
              py::arg("numa")=0,
              py::arg("tid")=0);
memaccess.def("from_bytes", &MemAccess::fromBytes,
              "Set memaccess attribute according to the content of a buffer.",
              py::arg("bytes"));
memaccess.def("uid", &MemAccess::uid,
              "Static method returning the identifier for MBind messages.");
memaccess.def("type", &MemAccess::type,
              "The type of memory access: MemAccess.Type.<R/W/RW>");
memaccess.def("tick", &MemAccess::tick,
              "Get the simulation tick on which this memory access occured.");
memaccess.def("address", &MemAccess::address,
              "The physical address of the memory access.");
memaccess.def("numa_node", &MemAccess::numaNode,
              "The logical index of the numa node being accessed.");
memaccess.def("threadid", &MemAccess::ThreadID,
              "The logical index of hardware thread accessing data.");
memaccess.def("__eq__", &MemAccess::operator==,
              "Attributes wise comparison.");
memaccess.def("__ne__", &MemAccess::operator!=,
              "Attributes wise comparison.");

py::class_<MBind> mbind(msg, "MBind",
                        "Message implementation for MBind syscall request"
                        "performed on client side.");

py::enum_<MBind::Flags>(mbind, "Flags")
.value("MPOL_MF_STRICT", MBind::Flags::MPOL_MF_STRICT, "See: man mbind")
.value("MPOL_MF_MOVE", MBind::Flags::MPOL_MF_MOVE, "See: man mbind")
.value("MPOL_MF_MOVE_ALL", MBind::Flags::MPOL_MF_MOVE_ALL, "See: man mbind")
.value("MPOL_F_NODE", MBind::Flags::MPOL_F_NODE, "See: man mbind")
.value("MPOL_F_ADDR", MBind::Flags::MPOL_F_ADDR, "See: man mbind")
.value("MPOL_F_MEMS_ALLOWED", MBind::Flags::MPOL_F_MEMS_ALLOWED,
       "See: man mbind")
.export_values();

py::enum_<MBind::Mode>(mbind, "Mode")
.value("MPOL_DEFAULT", MBind::Mode::MPOL_DEFAULT, "See: man mbind")
.value("MPOL_PREFERRED", MBind::Mode::MPOL_PREFERRED, "See: man mbind")
.value("MPOL_BIND", MBind::Mode::MPOL_BIND, "See: man mbind")
.value("MPOL_INTERLEAVE", MBind::Mode::MPOL_INTERLEAVE, "See: man mbind")
.value("MPOL_LOCAL", MBind::Mode::MPOL_LOCAL, "See: man mbind")
.export_values();

mbind.def(py::init<uint64_t, uint64_t, uint64_t, MBind::Mode, uint8_t>(),
          "Create and initialize a MBind request message. See: man bind\n"
          "addr: The virtual address of the first page to bind\n"
          "size: The size of the virtual address range to bind\n"
          "nodeset: A bitmask of 64 bits where bits represent the logical "
          "index of nodes.\n"
          "mode: The mode used to bind pages: MBind.Mode.<mode>.\n"
          "flags: The flags used to bind pages: MBind.Flags.<flags>.\n",
          py::arg("addr"), py::arg("size"), py::arg("nodeset"),
          py::arg("mode")=MBind::Mode::MPOL_BIND,
          py::arg("flags")=MBind::Flags::MPOL_MF_MOVE);
mbind.def("uid", &MBind::uid,
          "Static method returning the identifier for MBind messages.");
mbind.def("addr", &MBind::addr,
          "The start address of the syscall request to bind pages");
mbind.def("size", &MBind::size,
          "The size of the address range to bind.");
mbind.def("nodeset", &MBind::nodeset,
          "The nodeset (uint64_t) where to bind pages.");
mbind.def("mode", &MBind::mode,
          "The mode used to bind pages.");
mbind.def("flags", &MBind::flags,
          "The flags used to bind pages.");
mbind.def("__eq__", &MBind::operator==,
              "Attributes wise comparison.");
mbind.def("__ne__", &MBind::operator!=,
              "Attributes wise comparison.");
