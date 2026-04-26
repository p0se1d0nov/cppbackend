import argparse
import subprocess
import time
import random
import shlex
import os
import signal
from pathlib import Path

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1
SCRIPT_DIR = Path(__file__).resolve().parent
PERF_DATA_PATH = SCRIPT_DIR / 'perf.data'
GRAPH_SVG_PATH = SCRIPT_DIR / 'graph.svg'


def parse_server_command():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run_process(command, output=None, cwd=None):
    return subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL, cwd=cwd)


def stop_process(process, wait=False, sig=signal.SIGTERM):
    if process is None:
        return
    if process.poll() is None:
        process.send_signal(sig)
        if wait:
            process.wait()


def shoot(ammo):
    hit = run_process('curl ' + ammo, output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop_process(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')


def start_perf_record(pid):
    if os.path.exists(PERF_DATA_PATH):
        os.remove(PERF_DATA_PATH)

    process = subprocess.Popen(
        ['perf', 'record', '-o', str(PERF_DATA_PATH), '-g', '-p', str(pid)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
    )

    time.sleep(1)
    if process.poll() is not None:
        stderr = process.stderr.read().decode('utf-8', errors='replace').strip()
        raise RuntimeError(f'perf record failed to start: {stderr}')

    return process


def build_flamegraph():
    script_dir = Path(__file__).resolve().parent
    flamegraph_dir = script_dir / 'FlameGraph'
    collapse_script = flamegraph_dir / 'stackcollapse-perf.pl'
    flamegraph_script = flamegraph_dir / 'flamegraph.pl'

    if not collapse_script.exists() or not flamegraph_script.exists():
        raise FileNotFoundError('FlameGraph scripts not found next to shoot.py')

    with subprocess.Popen(
        ['perf', 'script', '-i', PERF_DATA_PATH],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
    ) as perf_script_proc:
        with subprocess.Popen(
            [str(collapse_script)],
            stdin=perf_script_proc.stdout,
            stdout=subprocess.PIPE,
            cwd=flamegraph_dir,
            stderr=subprocess.DEVNULL,
        ) as collapse_proc:
            perf_script_proc.stdout.close()
            with open(GRAPH_SVG_PATH, 'w', encoding='utf-8') as graph_file:
                flame_proc = subprocess.Popen(
                    [str(flamegraph_script)],
                    stdin=collapse_proc.stdout,
                    stdout=graph_file,
                    cwd=flamegraph_dir,
                    stderr=subprocess.DEVNULL,
                )
                collapse_proc.stdout.close()
                flame_proc.wait()
                collapse_proc.wait()
        perf_script_proc.wait()

    if not Path(GRAPH_SVG_PATH).exists() or Path(GRAPH_SVG_PATH).stat().st_size == 0:
        raise RuntimeError('Flamegraph generation failed: graph.svg is empty or missing')


def main():
    server_command = parse_server_command()
    server_process = None
    perf_process = None

    try:
        server_process = run_process(server_command)
        time.sleep(1)

        if server_process.poll() is not None:
            raise RuntimeError('Server process terminated unexpectedly')

        perf_process = start_perf_record(server_process.pid)
        time.sleep(1)

        make_shots()

    finally:
        stop_process(perf_process, wait=True, sig=signal.SIGINT)
        stop_process(server_process, wait=True)

    build_flamegraph()
    print('Job done')


if __name__ == '__main__':
    main()
