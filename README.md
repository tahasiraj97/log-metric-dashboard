# Log Metric Dashboard

A lightweight system monitoring dashboard written in C using SDL3.

The dashboard is designed for headless Linux systems such as a Raspberry Pi 5 or Debian and provides a simple visual interface for displaying system information, service status, and logs.

## Features

- Real-time system monitoring
- CPU usage
- Memory usage
- Storage usage
- Service monitoring
- Live log display
- SDL3-based graphical interface
- Designed for headless Linux systems
- ARM64 and x86_64 builds
- Remote display support

## Dashboard Layout

The dashboard is divided into two main sections.

### System and Service Information

The left side of the dashboard displays information about the host system and monitored services.

This includes information such as:

- CPU usage
- Memory usage
- Storage usage
- Service availability
- Device information

### Logs

The right side of the dashboard displays incoming log messages.

Logs are displayed using a monospace font and can be updated dynamically while the dashboard is running.

## Technologies

The project uses:

- C
- SDL3
- SDL3_ttf
- Fontconfig
- Linux
- Docker for development and builds

## Project Structure

```text
log-metric-dashboard/
├── src/
│   └── dashboard/
├── build/
│   └── dashboard/
├── docker-compose.yml
├── build-dashboard.sh
└── README.md
```

## Building

The dashboard can be built using the provided build script.

```bash
./build-dashboard.sh
```

Build output is placed in:

```text
build/dashboard/
```

The build system can generate binaries for supported architectures including:

```text
ARM64
x86_64
```

## Development

The development environment can be run using Docker Compose.

```bash
docker compose up
```

To view live container logs:

```bash
docker compose logs -f
```

To rebuild the environment:

```bash
docker compose up --build
```

## Running the Dashboard

Run the appropriate binary for the target architecture.

Example:

```bash
./dashboard
```

The application requires a working display environment.

For headless systems, the dashboard can be used with a virtual X server and remote display solution such as:

- Xvfb
- x11vnc
- noVNC

## Target Platform

The primary target is a Raspberry Pi 5 running Debian Linux.

The project can also be built for x86_64 Linux systems for development and testing.

## Purpose

The goal of the project is to provide a lightweight dashboard that can run continuously on a dedicated device and display useful server information without requiring a full monitoring platform or desktop environment.

The dashboard is intended to provide a quick visual overview of:

- System health
- Resource utilization
- Service availability
- Recent logs
