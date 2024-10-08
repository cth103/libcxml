# -*- mode: python -*-
#
#    Copyright (C) 2016-2018 Carl Hetherington <cth@carlh.net>
#
#    This file is part of libcxml.
#
#    libcxml is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    libcxml is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with libcxml.  If not, see <http://www.gnu.org/licenses/>.
#

import subprocess
import shlex
from waflib import Context

APPNAME = 'libcxml'

this_version = subprocess.Popen(shlex.split('git tag -l --points-at HEAD'), stdout=subprocess.PIPE).communicate()[0].decode('UTF-8')
last_version = subprocess.Popen(shlex.split('git describe --tags --abbrev=0'), stdout=subprocess.PIPE).communicate()[0].decode('UTF-8')

if this_version == '':
    VERSION = '%sdevel' % last_version[1:].strip()
else:
    VERSION = this_version[1:].strip()

API_VERSION = '0.0.0'

def options(opt):
    opt.load('compiler_cxx')
    opt.add_option('--target-windows-64', action='store_true', default=False, help='set up to do a cross-compile to Windows 64-bit')
    opt.add_option('--target-windows-32', action='store_true', default=False, help='set up to do a cross-compile to Windows 32-bit')
    opt.add_option('--enable-debug', action='store_true', default=False, help='build with debugging information and without optimisation')
    opt.add_option('--static', action='store_true', default=False, help='build statically')
    opt.add_option('--static-boost', action='store_true', default=False, help='link statically against boost')
    opt.add_option('--disable-tests', action='store_true', default=False, help='disable building of tests')
    opt.add_option('--c++17', action='store_true', default=False, help='build with C++17 and libxml++-4.0')

def configure(conf):
    conf.load('compiler_cxx')

    if vars(conf.options)['c++17']:
        cpp_std = '17'
        conf.env.XMLPP_API = '4.0'
        conf.env.GLIBMM_API = '2.68'
    else:
        cpp_std = '11'
        conf.env.XMLPP_API = '2.6'
        conf.env.GLIBMM_API = '2.4'

    if conf.options.enable_debug:
        conf.env.append_value('CXXFLAGS', '-g')
    conf.env.append_value('CXXFLAGS', ['-Wall', '-Wextra', '-O2', '-Wno-deprecated-declarations', '-std=c++' + cpp_std, '-DBOOST_NO_CXX11_SCOPED_ENUMS'])

    conf.env.TARGET_WINDOWS = conf.options.target_windows_32 or conf.options.target_windows_64
    conf.env.STATIC = conf.options.static
    conf.env.DISABLE_TESTS = conf.options.disable_tests
    conf.env.API_VERSION = API_VERSION

    if conf.env.TARGET_WINDOWS:
        boost_lib_suffix = '-mt-x32' if conf.options.target_windows_32 else '-mt-x64'
        conf.env.append_value('CXXFLAGS', '-DLIBCXML_WINDOWS')
    else:
        boost_lib_suffix = ''
        conf.env.append_value('CXXFLAGS', '-DLIBCXML_POSIX')

    if not conf.options.static_boost:
        conf.env.append_value('CXXFLAGS', '-DBOOST_TEST_DYN_LINK')

    conf.check_cfg(package='libxml++-' + conf.env.XMLPP_API, args='--cflags --libs', uselib_store='LIBXML++', mandatory=True)

    conf.check_cxx(fragment="""
 		   #include <boost/filesystem.hpp>\n
    		   int main() { boost::filesystem::copy_file ("a", "b"); }\n
		   """,
                   msg='Checking for boost filesystem library',
                   libpath='/usr/local/lib',
                   lib=['boost_filesystem%s' % boost_lib_suffix, 'boost_system%s' % boost_lib_suffix],
                   uselib_store='BOOST_FILESYSTEM')

    if not conf.options.disable_tests:
        conf.check_cxx(fragment="""
                                  #define BOOST_TEST_MODULE Config test\n
                                  #include <boost/test/unit_test.hpp>\n
                                  BOOST_AUTO_TEST_CASE(foo) {}
                                  """,
                                  msg='Checking for boost unit testing library',
                                  lib=['boost_unit_test_framework%s' % boost_lib_suffix, 'boost_system%s' % boost_lib_suffix],
                                  uselib_store='BOOST_TEST')

        conf.recurse('test')

def build(bld):
    bld(source='libcxml.pc.in',
        version=VERSION,
        includedir='%s/include' % bld.env.PREFIX,
        libs="-L${libdir} -lcxml",
        install_path='${LIBDIR}/pkgconfig',
        xmlpp_api=bld.env.XMLPP_API,
        glibmm_api=bld.env.GLIBMM_API)

    bld.recurse('src')
    if not bld.env.DISABLE_TESTS:
        bld.recurse('test')
