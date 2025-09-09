# Simple UDP Messaging Server

## Overview

This repository contains a simple UDP messaging server implemented in C. The primary purpose of this project is to refresh my skills in C programming, particularly in socket programming. The server can be used in conjunction with `netcat` to facilitate easy messaging over UDP.

## Features

- **UDP Protocol**: Utilizes the User Datagram Protocol (UDP) for message transmission.
- **Simple Messaging**: Allows users to send and receive messages easily.
- **Compatibility with Netcat**: Can be tested and used with the `netcat -u` utility for quick communication.

## Getting Started

### Prerequisites

To compile and run this program, you will need:

- A C compiler (e.g., `gcc`)
- `netcat` installed on your system

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/massimoGG/membership_messaging.git
   cd membership_messaging

2. Compile the program:
```bash

    gcc -o udp_server udp_server.c
```

### Usage
Start the UDP server:
```bash
./udp_server
```

In a separate terminal/device, use `netcat` to send messages to the server:

```bash
    netcat -u [host] 3490
```

Type your message and press Enter to send it to the server. New "clients" shall be added to the "membership" and receive new messages. Any client already in the "membership" shall receive your message.
The server should display the received message. 
