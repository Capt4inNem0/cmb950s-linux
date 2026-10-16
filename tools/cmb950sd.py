#!/usr/bin/env python3
import glob
import os
import subprocess
import sys
import time

DRIVER = "/sys/bus/hid/drivers/cmb950s"
INTERVAL = 5


def read_int(path, scale=1):
    try:
        with open(path) as f:
            return int(int(f.read()) / scale)
    except (OSError, ValueError):
        return 0


def hwmon(name):
    for h in glob.glob("/sys/class/hwmon/hwmon*"):
        try:
            with open(os.path.join(h, "name")) as f:
                if f.read().strip() == name:
                    return h
        except OSError:
            pass
    return None


_last = None


def cpu_load():
    global _last
    with open("/proc/stat") as f:
        v = [int(x) for x in f.readline().split()[1:]]
    idle, total = v[3] + v[4], sum(v)
    if _last is None:
        _last = (idle, total)
        return 0
    di, dt = idle - _last[0], total - _last[1]
    _last = (idle, total)
    return int(100 * (dt - di) / dt) if dt else 0


def nvidia():
    try:
        out = subprocess.check_output(
            ["nvidia-smi", "--query-gpu=temperature.gpu,utilization.gpu,clocks.sm,fan.speed,power.draw",
             "--format=csv,noheader,nounits"], text=True, timeout=2)
        t, u, c, fan, p = (float(x) for x in out.strip().split(","))
        return dict(gpu_temperature=int(t), gpu_load=int(u), gpu_frequency=int(c),
                    gpu_fan=int(fan), gpu_power=int(p))
    except (OSError, ValueError, subprocess.SubprocessError):
        return {}


def sample():
    k10 = hwmon("k10temp") or ""
    amdgpu = hwmon("amdgpu") or ""
    v = dict(
        cpu_temperature=read_int(f"{k10}/temp1_input", 1000),
        cpu_load=cpu_load(),
        cpu_frequency=read_int("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", 1000),
        cpu_fan=0,
        cpu_power=0,
        gpu_temperature=read_int(f"{amdgpu}/temp1_input", 1000),
        gpu_load=0,
        gpu_frequency=0,
        gpu_fan=0,
        gpu_power=read_int(f"{amdgpu}/power1_input", 1000000),
    )
    v.update(nvidia())
    return v


def push(values):
    for dev in glob.glob(f"{DRIVER}/*:*"):
        for name, val in values.items():
            path = os.path.join(dev, name)
            if not os.path.exists(path):
                continue
            try:
                with open(path, "w") as f:
                    f.write(str(val))
            except OSError as e:
                print(f"{path}: {e}", file=sys.stderr)


def main():
    once = "--once" in sys.argv
    while True:
        push(sample())
        if once:
            break
        time.sleep(INTERVAL)


if __name__ == "__main__":
    main()
