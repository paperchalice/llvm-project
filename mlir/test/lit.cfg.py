# -*- Python -*-

import os
import platform
import re
import shutil
import subprocess
import tempfile

import lit.formats
import lit.util

from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst
from lit.llvm.subst import FindTool

# Configuration file for the 'lit' test runner.

# name: The name of this test suite.
config.name = "MLIR"

# TODO: Consolidate the logic for turning on the internal shell by default for all LLVM test suites.
# See https://github.com/llvm/llvm-project/issues/106636 for more details.
#
# We prefer the lit internal shell which provides a better user experience on failures
# unless the user explicitly disables it with LIT_USE_INTERNAL_SHELL=0 env var.
use_lit_shell = True
lit_shell_env = os.environ.get("LIT_USE_INTERNAL_SHELL")
if lit_shell_env:
    use_lit_shell = lit.util.pythonize_bool(lit_shell_env)

config.test_format = lit.formats.ShTest(execute_external=not use_lit_shell)

# suffixes: A list of file extensions to treat as test files.
config.suffixes = [
    ".td",
    ".mlir",
    ".toy",
    ".ll",
    ".tc",
    ".py",
    ".yaml",
    ".test",
    ".pdll",
    ".c",
    ".spv",
]

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.mlir_obj_root, "test")

config.substitutions.append(("%PATH%", config.environment["PATH"]))
config.substitutions.append(("%shlibext", config.llvm_shlib_ext))
config.substitutions.append(("%llvm_src_root", config.llvm_src_root))
config.substitutions.append(("%mlir_src_root", config.mlir_src_root))
config.substitutions.append(("%host_cxx", config.host_cxx.strip()))
config.substitutions.append(("%host_cc", config.host_cc.strip()))


# Searches for a runtime library with the given name and returns the found path.
# Correctly handles the platforms shared library directory and naming conventions.
def find_runtime(name):
    path = ""
    for prefix in ["", "lib"]:
        path = os.path.join(
            config.llvm_shlib_dir, f"{prefix}{name}{config.llvm_shlib_ext}"
        )
        if os.path.isfile(path):
            break
    return path


# Searches for a runtime library with the given name and returns a tool
# substitution of the same name and the found path.
def add_runtime(name):
    return ToolSubst(f"%{name}", find_runtime(name))


# Provide the path to asan runtime lib 'libclang_rt.asan_osx_dynamic.dylib' if
# available. This is darwin specific since it's currently only needed on darwin.
# Stolen from llvm/test/lit.cfg.py with a few modifications
def get_asan_rtlib():
    if not "asan" in config.available_features:
        return ""

    if "Darwin" in config.host_os:
        # Find the asan rt lib
        resource_dir = (
            subprocess.check_output([config.host_cc.strip(), "-print-resource-dir"])
            .decode("utf-8")
            .strip()
        )
        return os.path.join(
            resource_dir, "lib", "darwin", "libclang_rt.asan_osx_dynamic.dylib"
        )
    if "Linux" in config.host_os:
        return (
            subprocess.check_output(
                [
                    config.host_cxx.strip(),
                    f"-print-file-name=libclang_rt.asan-{config.host_arch}.so",
                ]
            )
            .decode("utf-8")
            .strip()
        )

    return ""


# On macOS, we can't do the DYLD_INSERT_LIBRARIES trick with a shim python
# binary as the ASan interceptors get loaded too late. Also, when SIP is
# enabled, we can't inject libraries into system binaries at all, so we need a
# copy of the "real" python to work with.
# Stolen from lldb/test/API/lit.cfg.py with a few modifications
def find_real_python_interpreter():
    # If we're running in a virtual environment, we have to copy Python into
    # the virtual environment for it to work.
    if sys.prefix != sys.base_prefix:
        copied_python = os.path.join(sys.prefix, "bin", "copied-python")
    else:
        copied_python = os.path.join(config.mlir_obj_root, "copied-python")

    # Avoid doing any work if we already copied the binary.
    if os.path.isfile(copied_python):
        return copied_python

    # Find the "real" python binary.
    real_python = (
        subprocess.check_output(
            [
                config.python_executable,
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "get_darwin_real_python.py",
                ),
            ]
        )
        .decode("utf-8")
        .strip()
    )

    shutil.copy(real_python, copied_python)

    # Now make sure the copied Python works. The Python in Xcode has a relative
    # RPATH and cannot be copied.
    try:
        # We don't care about the output, just make sure it runs.
        subprocess.check_call([copied_python, "-V"])
    except subprocess.CalledProcessError:
        # The copied Python didn't work. Assume we're dealing with the Python
        # interpreter in Xcode. Given that this is not a system binary SIP
        # won't prevent us form injecting the interceptors, but when running in
        # a virtual environment, we can't use it directly. Create a symlink
        # instead.
        os.remove(copied_python)
        os.symlink(real_python, copied_python)

    # The copied Python works.
    return copied_python


llvm_config.with_system_environment(["HOME", "INCLUDE", "LIB", "TMP", "TEMP"])

llvm_config.use_default_substitutions()

# excludes: A list of directories to exclude from the testsuite. The 'Inputs'
# subdirectories contain auxiliary inputs for various tests in their parent
# directories.
config.excludes = [
    "Inputs",
    "CMakeLists.txt",
    "README.txt",
    "LICENSE.txt",
    "lit.cfg.py",
    "lit.site.cfg.py",
    "get_darwin_real_python.py",
]

# Tweak the PATH to include the tools dir.
llvm_config.with_environment("PATH", config.mlir_tools_dir, append_path=True)
llvm_config.with_environment("PATH", config.llvm_tools_dir, append_path=True)

tool_dirs = [config.mlir_tools_dir, config.llvm_tools_dir]
tools = [
    "mlir-tblgen",
    "mlir-translate",
    "mlir-lsp-server",
    "mlir-capi-execution-engine-test",
    "mlir-capi-ir-test",
    "mlir-capi-irdl-test",
    "mlir-capi-llvm-test",
    "mlir-capi-pass-test",
    "mlir-capi-pdl-test",
    "mlir-capi-quant-test",
    "mlir-capi-rewrite-test",
    "mlir-capi-sparse-tensor-test",
    "mlir-capi-transform-test",
    "mlir-capi-transform-interpreter-test",
    "mlir-capi-translation-test",
    "mlir-runner",
    add_runtime("mlir_runner_utils"),
    add_runtime("mlir_c_runner_utils"),
    add_runtime("mlir_async_runtime"),
    add_runtime("mlir_float16_utils"),
    "mlir-linalg-ods-yaml-gen",
    "mlir-reduce",
    "mlir-pdll",
    "not",
]

if config.enable_vulkan_runner:
    tools.extend([add_runtime("mlir_vulkan_runtime")])

if config.enable_rocm_runner:
    tools.extend([add_runtime("mlir_rocm_runtime")])

if config.enable_cuda_runner:
    tools.extend([add_runtime("mlir_cuda_runtime")])

if config.enable_sycl_runner:
    tools.extend([add_runtime("mlir_sycl_runtime")])

if config.enable_spirv_cpu_runner:
    tools.extend([add_runtime("mlir_spirv_cpu_runtime")])

if config.mlir_run_arm_sve_tests or config.mlir_run_arm_sme_tests:
    tools.extend([add_runtime("mlir_arm_runner_utils")])

if config.mlir_run_arm_sme_tests:
    config.substitutions.append(
        (
            "%arm_sme_abi_shlib",
            # Use passed Arm SME ABI routines, if not present default to stubs.
            config.arm_sme_abi_routines_shlib or find_runtime("mlir_arm_sme_abi_stubs"),
        )
    )

# The following tools are optional
tools.extend(
    [
        ToolSubst("toyc-ch1", unresolved="ignore"),
        ToolSubst("toyc-ch2", unresolved="ignore"),
        ToolSubst("toyc-ch3", unresolved="ignore"),
        ToolSubst("toyc-ch4", unresolved="ignore"),
        ToolSubst("toyc-ch5", unresolved="ignore"),
        ToolSubst("toyc-ch6", unresolved="ignore"),
        ToolSubst("toyc-ch7", unresolved="ignore"),
        ToolSubst("transform-opt-ch2", unresolved="ignore"),
        ToolSubst("transform-opt-ch3", unresolved="ignore"),
        ToolSubst("transform-opt-ch4", unresolved="ignore"),
        ToolSubst("mlir-transform-opt", unresolved="ignore"),
        ToolSubst("%mlir_lib_dir", config.mlir_lib_dir, unresolved="ignore"),
        ToolSubst("%mlir_src_dir", config.mlir_src_root, unresolved="ignore"),
    ]
)

python_executable = config.python_executable
# Python configuration with sanitizer requires some magic preloading. This will only work on clang/linux/darwin.
# TODO: detect Windows situation (or mark these tests as unsupported on these platforms).
if "asan" in config.available_features:
    if "Linux" in config.host_os:
        python_executable = (
            f"env LD_PRELOAD={get_asan_rtlib()} {config.python_executable}"
        )
    if "Darwin" in config.host_os:
        # Ensure we use a non-shim Python executable, for the `DYLD_INSERT_LIBRARIES`
        # env variable to take effect
        real_python_executable = find_real_python_interpreter()
        if real_python_executable:
            python_executable = real_python_executable
            lit_config.note(
                "Using {} instead of {}".format(
                    python_executable, config.python_executable
                )
            )

        asan_rtlib = get_asan_rtlib()
        lit_config.note("Using ASan rtlib {}".format(asan_rtlib))
        config.environment["MallocNanoZone"] = "0"
        config.environment["ASAN_OPTIONS"] = "detect_stack_use_after_return=1"
        config.environment["DYLD_INSERT_LIBRARIES"] = asan_rtlib


# On Windows the path to python could contains spaces in which case it needs to be provided in quotes.
# This is the equivalent of how %python is setup in llvm/utils/lit/lit/llvm/config.py.
elif "Windows" in config.host_os:
    python_executable = '"%s"' % (python_executable)
tools.extend(
    [
        ToolSubst("%PYTHON", python_executable, unresolved="ignore"),
    ]
)

if "MLIR_OPT_CHECK_IR_ROUNDTRIP" in os.environ:
    tools.extend(
        [
            ToolSubst("mlir-opt", "mlir-opt --verify-roundtrip", unresolved="fatal"),
        ]
    )
elif "MLIR_GENERATE_PATTERN_CATALOG" in os.environ:
    tools.extend(
        [
            ToolSubst(
                "mlir-opt",
                "mlir-opt --debug-only=pattern-logging-listener --mlir-disable-threading",
                unresolved="fatal",
            ),
            ToolSubst("FileCheck", "FileCheck --dump-input=always", unresolved="fatal"),
        ]
    )
else:
    tools.extend(["mlir-opt"])

llvm_config.add_tool_substitutions(tools, tool_dirs)


# FileCheck -enable-var-scope is enabled by default in MLIR test
# This option avoids to accidentally reuse variable across -LABEL match,
# it can be explicitly opted-in by prefixing the variable name with $
config.environment["FILECHECK_OPTS"] = "-enable-var-scope --allow-unused-prefixes=false"

# Add the python path for both the source and binary tree.
# Note that presently, the python sources come from the source tree and the
# binaries come from the build tree. This should be unified to the build tree
# by copying/linking sources to build.
if config.enable_bindings_python:
    config.environment["PYTHONPATH"] = os.getenv("MLIR_LIT_PYTHONPATH", "")
    llvm_config.with_environment(
        "PYTHONPATH",
        [
            os.path.join(config.mlir_obj_root, "python_packages", "mlir_core"),
            os.path.join(config.mlir_obj_root, "python_packages", "mlir_test"),
        ],
        append_path=True,
    )

if config.enable_assertions:
    config.available_features.add("asserts")
else:
    config.available_features.add("noasserts")

def have_host_jit_feature_support(feature_name):
    mlir_runner_exe = lit.util.which("mlir-runner", config.mlir_tools_dir)

    if not mlir_runner_exe:
        return False

    try:
        mlir_runner_cmd = subprocess.Popen(
            [mlir_runner_exe, "--host-supports-" + feature_name],
            stdout=subprocess.PIPE,
        )
    except OSError:
        print("could not exec mlir-runner")
        return False

    mlir_runner_out = mlir_runner_cmd.stdout.read().decode("ascii")
    mlir_runner_cmd.wait()

    return "true" in mlir_runner_out


if have_host_jit_feature_support("jit"):
    config.available_features.add("host-supports-jit")

if config.run_nvptx_tests:
    config.available_features.add("host-supports-nvptx")

if config.run_rocm_tests:
    config.available_features.add("host-supports-amdgpu")

if config.arm_emulator_executable:
    config.available_features.add("arm-emulator")
