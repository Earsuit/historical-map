from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.system.package_manager import Apt

class HistoricalMap(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("glew/2.2.0")
        self.requires("glfw/3.4")
        self.requires("sqlite3/3.46.0")
        self.requires("libcurl/8.8.0")
        self.requires("libgettext/0.21")

    def build_requirements(self):
        self.tool_requires("gettext/0.21")
        self.test_requires("gtest/1.14.0")

    def system_requirements(self):
        Apt(self).install(["libglu1-mesa-dev"], update=True, check=True) # for GL/glu.h on Ubuntu