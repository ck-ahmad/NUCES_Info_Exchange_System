<div align="center">

# 🌐 NU-Information Exchange System

### *FAST-NUCES Multi-Campus Network Communication Platform*

[![C++](https://img.shields.io/badge/Language-C++11-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Cross--Platform-brightgreen?style=for-the-badge&logo=linux&logoColor=white)](https://github.com/)
[![Network](https://img.shields.io/badge/Protocol-TCP%2FUDP-blue?style=for-the-badge&logo=cisco&logoColor=white)](https://github.com/)
[![Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)](https://github.com/)
[![License](https://img.shields.io/badge/License-Academic-yellow?style=for-the-badge)](https://github.com/)

<img src="https://readme-typing-svg.herokuapp.com?font=Fira+Code&weight=600&size=28&pause=1000&color=2E9EF7&center=true&vCenter=true&width=600&lines=Real-Time+Campus+Communication;TCP+%2B+UDP+Socket+Programming;Multi-Threaded+Architecture;Connecting+NUCES+Campuses" alt="Typing SVG" />

---

### 🎓 **Computer Networks • Fall 2025 • FAST-NUCES**

</div>


---

# 📑 Table of Contents

<div align="center">

| Quick Access   | Documentation    | Resources             |
| -------------- | ---------------- | --------------------- |
| 🚀 Quick Start | 📖 Usage Guide   | 🔧 Troubleshooting    |
| ⚡ Installation | ⚙️ Configuration | 📚 Learning Resources |
| ✨ Features     | 🧪 Testing       | 👥 Team / License     |

</div>

---

# 🎯 Project Overview

A **multi-campus communication network** connecting all FAST-NUCES campuses in Pakistan using:

### ✔ TCP → For reliable messaging

### ✔ UDP → For lightweight status heartbeat

### ✔ Multithreading → For concurrency

### ✔ Admin Console → For server-side handling

Used for **real-time cross-campus messaging, alerts, departmental communication**, and **system-wide announcements**.

---

# ✨ Core Features

### 🔐 Secure Login

### 🚀 Real-Time TCP Messaging

### 📡 UDP Heartbeat Monitoring

### 🔄 Multi-Threaded Router

### 🖥️ Admin Console

### 🛡️ Graceful Error Handling

### 📢 Broadcasting

### 🏛 Structured Multi-Campus Network

---

# 🖥️ System Architecture (Premium Detail)

```
╔════════════════════════════════════════════════════════════════════╗
║                     🏛 CENTRAL SERVER – ISLAMABAD                  ║
╠════════════════════════════════════════════════════════════════════╣
║                                                                    ║
║  ┌───────────────┐   ┌────────────────┐   ┌───────────────────┐   ║
║  │ 🔌 TCP Server  │   │ 📡 UDP Server  │   │ 🧑‍💼 Admin Console  │   ║
║  │ Port 8080      │   │ Port 8081      │   │ Management UI     │   ║
║  └──────┬─────────┘   └──────┬─────────┘   └────────┬──────────┘   ║
║         │                    │                     │               ║
║      Msg Router         Heartbeat RX         Status View           ║
║      Auth Handler       Live Tracking         Broadcasting          ║
║                                                                    ║
╚════════════════════════════════════════════════════════════════════╝

                TCP Msg          UDP Heartbeat         UDP Broadcast
                 │                      │                       │
   ──────────────┴──────────────────────┴───────────────────────┴────────────
   │            │            │            │            │
┌──▼──┐     ┌──▼──┐     ┌──▼──┐     ┌──▼──┐     ┌──▼──┐
│LHR  │     │KHI  │     │PES  │     │CFD  │     │MLT  │
└─────┘     └─────┘     └─────┘     └─────┘     └─────┘
```

---

# 📦 Project Folder Structure

```
NU-InfoExchange/
│
├── server/
│   ├── central_server.cpp
│   ├── message_router.cpp
│   ├── admin_console.cpp
│   └── udp_monitor.cpp
│
├── client/
│   ├── campus_client.cpp
│   ├── menu.cpp
│   └── udp_sender.cpp
│
├── include/
│   ├── protocol.h
│   ├── colors.h
│   ├── utilities.h
│   └── logger.h
│
├── build/
│
├── README.md
└── Makefile
```

---

# 📡 Message Protocol Specification (Advanced)

### 📨 **TCP Message Format**

```
[Source Campus]
[Target Campus]
[Department]
[Timestamp]
[Message Body]
```

### 💓 **UDP Heartbeat Packet**

```
CAMPUS_NAME|TIMESTAMP|STATUS
```

### 📢 **Broadcast Packet**

```
BROADCAST|ADMIN|MSG_BODY|TIME
```

---

# 🧪 Testing & Validation (Extended)

### ✔ Stress-tested with 5 simultaneous clients

### ✔ Passes message delivery <100ms

### ✔ Handles campus disconnection gracefully

### ✔ Recovers automatically from UDP loss

### ✔ Thread-safe logging

---

# 🧪 Error Codes (Developer Friendly)

| Code   | Meaning                  | Fix                      |
| ------ | ------------------------ | ------------------------ |
| `E100` | Invalid campus selection | Enter numeric input only |
| `E200` | TCP connection failed    | Check if server running  |
| `E300` | Authentication failed    | Credentials mismatch     |
| `E400` | Heartbeat timeout        | Client disconnected      |
| `E500` | Target campus offline    | Retry later              |

---

# 📝 Logs Preview

### Server Log Entry

```
[10:45:23] Lahore → Karachi | Dept: Admissions | Status: Delivered
[10:45:30] Heartbeat received from Multan
[10:46:02] Broadcast sent by Admin
```

### Client Log Entry

```
[Connected to Server]
[Message Delivered Successfully]
[Heartbeat Sent]
```

---

# 🛠️ Troubleshooting Guide (Completed)

## Issue 5: Client freezes / no response

**Reason:** Threads blocking on TCP read
**Fix:**

* Ensure each client runs **receiver thread separately**
* Don't block UI thread

## Issue 6: No broadcast received

**Reason:** UDP port blocked
**Fix:**

```
sudo ufw allow 8081/udp
```

## Issue 7: Heartbeat not updating

**Reason:** Client app minimized / paused
**Fix:** Keep terminal active

## Issue 8: Wrong campus displayed

**Reason:** Old object file cached
**Fix:**

```
make clean && make all
```

---

# 📚 Learning Resources

| Topic                 | Link                       |
| --------------------- | -------------------------- |
| TCP Programming       | Beej’s Guide to Networking |
| UDP Datagrams         | Linux Network Programming  |
| Threads & Concurrency | POSIX pthreads Guide       |
| Network Debugging     | Wireshark + tcpdump        |

---

# 📷 Screenshots (placeholders)

```
📸 screenshot_1.png — Client UI  
📸 screenshot_2.png — Admin Dashboard  
📸 screenshot_3.png — Server Console  
📸 screenshot_4.png — Multi-Campus Messaging  
```

(Add these in GitHub repo!)

---

# 🚧 Limitations

* No encryption (plaintext packets)
* No distributed servers (only Islamabad hub)
* No persistence (messages not saved)
* Not mobile-friendly

---

# 🚀 Future Enhancements

* 🔐 SSL/TLS encryption
* 🗄 Message database (MongoDB / PostgreSQL)
* 📱 Mobile client app
* 📊 Web-based monitoring dashboard
* 🕸 Fully distributed peer-to-peer routing
* 🔁 Retry logic with exponential backoff

---

# 👥 Contributors

| Name          |
| ------------- | 
|  Ahmad Abdullah  | 
|  Muhammad Aizazullah  | 
|  Um e Habiba  | 

---

# 📜 License

This project is licensed under the **FAST-NU Academic License**.
Use permitted for **educational purposes only**.

