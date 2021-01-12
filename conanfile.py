from conans import ConanFile, CMake


class EstdConan(ConanFile):
    name = "estd"
    version = "0.1.8"
    license = "MIT"
    author = "Malachi Burke <malachi.burke@gmail.com>"
    url = "https://github.com/malachi-iot/estdlib"
    description = "Embedded oriented subset of STD C++ library"
    topics = ("std", "embedded")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    exports_sources = "src/*"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="src")
        cmake.build()

        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include", src="src")
        # since we need manual access to cpp files for esoteric embedded build systems
        self.copy("*.cpp", src="src")

        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["estd"]
