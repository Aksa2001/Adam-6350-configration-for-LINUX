##Industrial Modbus TCP Integration for ADAM-6350 on Linux
This C application is designed to **replace the vendor-provided Windows-based software** for the **Advantech ADAM-6350** Ethernet I/O module with a reliable, industrial-grade, Linux-compatible solution.

It uses the open-source **libmodbus** library to provide:

- Real-time **Digital Input (DI)** monitoring
- **Digital Output (DO)** coil control
- Thread-safe, fault-tolerant communication over **Modbus TCP**

---

##Purpose
The ADAM-6350 module lacks native industrial support for Linux environments. This application was developed to:

- Integrate the ADAM-6350 into **custom Yocto-based Linux images**
- Provide a **headless**, robust and repeatable solution for industrial I/O
- Enable **reliable Modbus TCP communication** from Linux systems in field deployments

---
