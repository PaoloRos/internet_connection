# Internet Connection

A small **macOS desktop application** (Qt Widgets + C++) that continuously checks whether the computer can reach the internet and shows the result with a simple visual status.

The idea came from the fact that the internet connection at my university does not always work properly on my computer. Therefore, I decided to quickly develop a tool to easily and frequently monitor my connection.

## Purpose
- Provide an immediate **Connected / Not connected** view.
- Keep checks periodic and lightweight.
- Show current observed **download / upload throughput**.
- Show concise diagnostics for quick troubleshooting.

## Architecture Summary
- **UI layer (`MainWindow`)**: renders status indicator and diagnostics.
- **Service layer (`ConnectivityChecker`)**: runs asynchronous checks and emits normalized status objects.
- **Traffic layer (`NetworkTrafficMonitor`)**: samples network interface counters and computes rolling throughput statistics.
- **Check strategy**:
  - Primary: HTTP `HEAD` probe to Apple captive-portal endpoint.
  - Fallback: ICMP `ping` with timeout.

## Runtime Behavior
- Connectivity refresh interval: **500 ms**.
- Throughput sampling interval: **1 s**.
- Throughput statistics window: **20 samples**.
- Non-overlapping check cycles (no request piling).
- Diagnostics shown in UI:
  - method used
  - latency (when available)
  - current observed download and upload rates from active non-loopback network interfaces
  - rolling mean and standard deviation for download and upload rates
  - last error
  - last check timestamp

The transfer-rate values are live interface throughput measurements. They show
how much data the Mac is currently receiving and sending; they are intentionally
not a heavy external speed-test benchmark. Rolling statistics are shown as `-`
until the full 20-sample window is available.

## Build
```bash
cmake -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build -j
./build/internet_connection
```

## Install
Default install prefix is `~/usr/local` (applied on first configure when no custom prefix is provided).

```bash
cmake -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build -j
cmake --install build
```

Install to a specific destination:

```bash
cmake --install build --prefix "$HOME/usr/local"
```

## Credits
- Project idea and requirements: [**PaoloRos**](https://github.com/PaoloRos).
- Architecture and implementation: **Codex assistant**.
- AI model used in this environment: **Codex (GPT-5 family)**.
