#!/usr/bin/env python3
"""Build and verify liboai using synthetic tests only; never loads API keys."""
import argparse
import os
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', default='build/ci')
    parser.add_argument('--config', choices=['Debug', 'Release'], default='Release')
    parser.add_argument('--openssl', choices=['ON', 'OFF'], default='OFF')
    parser.add_argument('--shared', choices=['ON', 'OFF'], default='OFF')
    parser.add_argument('--sanitizers', action='store_true')
    parser.add_argument('--parallel', type=int, default=2)
    parser.add_argument('--cmake-arg', action='append', default=[])
    args = parser.parse_args()
    source = Path(__file__).resolve().parents[2]
    build = Path(args.build_dir).resolve()
    build.mkdir(parents=True, exist_ok=True)
    prefix = build / 'install'
    env = os.environ.copy()
    # Fixtures set synthetic authorization. Never forward real provider keys.
    for key in list(env):
        if key.endswith('_API_KEY') or key in ('OPENAI_ORG_ID', 'OPENAI_PROJECT_ID'):
            del env[key]
    if args.sanitizers:
        env['ASAN_OPTIONS'] = 'detect_leaks=1:halt_on_error=1'
        env['UBSAN_OPTIONS'] = 'halt_on_error=1:print_stacktrace=1'
    common = [f'-DCMAKE_BUILD_TYPE={args.config}',
              f'-DLIBOAI_WEBHOOK_OPENSSL={args.openssl}',
              f'-DBUILD_SHARED_LIBS={args.shared}']
    for variable in ('CMAKE_TOOLCHAIN_FILE', 'VCPKG_TARGET_TRIPLET'):
        if env.get(variable):
            common.append(f'-D{variable}={env[variable]}')
    common += args.cmake_arg
    if args.sanitizers:
        common += ['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer']

    def run(command):
        print('+ ' + ' '.join(map(str, command)), flush=True)
        subprocess.run(list(map(str, command)), cwd=source, env=env, check=True)

    def configure(src, directory, extra):
        run(['cmake', '-S', src, '-B', directory, *common, *extra])

    def build_and_test(directory):
        run(['cmake', '--build', directory, '--config', args.config,
             '--parallel', str(args.parallel)])
        run(['ctest', '--test-dir', directory, '-C', args.config,
             '--output-on-failure', '--no-tests=error', '--output-junit', directory / 'results.xml'])

    configure(source, build / 'library', ['-DLIBOAI_BUILD_TESTS=ON', '-DLIBOAI_BUILD_LIVE_TESTS=ON',
              '-DBUILD_EXAMPLES=ON', '-DLIBOAI_INSTALL=ON', f'-DCMAKE_INSTALL_PREFIX={prefix}'])
    build_and_test(build / 'library')
    run(['cmake', '--install', build / 'library', '--config', args.config])
    # Use the installed package directly. Dependencies still come from the
    # supplied toolchain/prefix; this must not silently fall back to source.
    configure(source / 'tests/consumer', build / 'installed-consumer',
              [f'-Doai_DIR={prefix / "lib/cmake/oai"}'])
    build_and_test(build / 'installed-consumer')
    configure(source / 'tests/consumer', build / 'parent-consumer',
              [f'-DLIBOAI_SOURCE_DIR={source}', '-DLIBOAI_PARENT_DEPENDENCIES=ON'])
    build_and_test(build / 'parent-consumer')
    print('Library, examples, installed consumer and parent consumer passed.', flush=True)


if __name__ == '__main__':
    try:
        main()
    except subprocess.CalledProcessError as error:
        sys.exit(error.returncode)
