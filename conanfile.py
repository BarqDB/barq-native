from conan import ConanFile
from conan.tools.build import check_min_cppstd, can_run
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.scm import Git

class barq_nativeRecipe(ConanFile):
    name = "barq_native"
    version = "2.2.0"

    # Optional metadata
    license = "Apache-2.0"
    url = "https://github.com/barq/barq-native"
    description = "Barq C++ SDK for the Barq local-first database."

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}

    def is_darwin(self):
        return self.settings.os == "Macos" or self.settings.os == "iOS" or self.settings.os == "watchOS"

    def validate(self):
        check_min_cppstd(self, "17")

    def requirements(self):
        self.requires(self.tested_reference_str)
        if not self.is_darwin() and not self.settings.os == "Emscripten":
            self.requires("zlib/1.3")
        if not self.is_darwin():
            self.requires("openssl/3.2.0")
            self.requires("libuv/1.48.0")
    def source(self):
        git = Git(self)
        git.clone(url="https://github.com/barq/barq-native", target=".")
        git.folder = "."
        git.checkout(commit="5ec1bda338dfd0c91ce1eea2ccb2c0adf7d86690")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["USES_CONAN"] = "ON"
        tc.variables["BARQ_NATIVE_NO_TESTS"] = "ON"
        tc.variables["BARQ_CORE_SUBMODULE_BUILD"] = "ON"
        tc.variables["BARQ_USE_SYSTEM_OPENSSL"] = "ON"
        if self.settings.os == "Windows":
            self.cpp_info.cxxflags = ["/Zc:preprocessor /bigobj"]
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        if self.settings.build_type == "Debug":
            self.cpp_info.libs = ["barq_native-dbg", "barq-object-store-dbg", "barq-parser-dbg", "barq-sync-dbg", "barq-dbg"]
        else:
            self.cpp_info.libs = ["barq_native", "barq-object-store", "barq-parser", "barq-sync", "barq"]
        if self.is_darwin():
            self.cpp_info.frameworks = ["Foundation", "Security", "Compression", "z"]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs = ["Version"]
