import os

from conans import ConanFile


class QtPromiseConfig(ConanFile):
    name = "QtPromise"
    version = "master"
    license = "QtPromise is available under the MIT license."
    author = "simonbrunel"
    url = "https://github.com/simonbrunel/qtpromise"
    description = "Promises/A+ implementation for Qt/C++"
    settings = "os", "compiler", "build_type", "arch"

    def package_id(self):
        self.info.header_only()

    def source(self):
        self.run("git clone git@github.com:simonbrunel/qtpromise.git")

    def package(self):
        self.copy("*", dst="include", src="./qtpromise/include")
        self.copy("*", dst="src", src="./qtpromise/src")

    def package_info(self):
        self.cpp_info.libs = self.collect_libs()


if __name__ == '__main__':
    os.system("conan create .")
