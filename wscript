APPNAME = 'libcxml'
VERSION = '0.14.0devel'
API_VERSION = '0.0.0'

def options(opt):
    opt.load('compiler_cxx')
    opt.add_option('--target-windows', action='store_true', default=False, help='set up to do a cross-compile to Windows')
    opt.add_option('--static', action='store_true', default=False, help='build statically')
    opt.add_option('--disable-tests', action='store_true', default=False, help='disable building of tests')

def configure(conf):
    conf.load('compiler_cxx')
    conf.env.append_value('CXXFLAGS', ['-Wall', '-Wextra', '-O2'])

    conf.env.TARGET_WINDOWS = conf.options.target_windows
    conf.env.STATIC = conf.options.static
    conf.env.DISABLE_TESTS = conf.options.disable_tests
    conf.env.API_VERSION = API_VERSION

    if conf.options.target_windows:
        boost_lib_suffix = '-mt'
    else:
        boost_lib_suffix = ''

    conf.check_cfg(package='libxml++-2.6', args='--cflags --libs', uselib_store='LIBXML++', mandatory=True)

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
                                  int main() {}
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
        install_path='${LIBDIR}/pkgconfig')

    bld.recurse('src')
    if not bld.env.DISABLE_TESTS:
        bld.recurse('test')
