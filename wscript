APPNAME = 'libcxml'
VERSION = '0.02pre'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')
    conf.env.append_value('CXXFLAGS', ['-Wall', '-Wextra', '-O2'])

    conf.check_cfg(package = 'libxml++-2.6', args = '--cflags --libs', uselib_store = 'LIBXML++', mandatory = True)

    conf.check_cxx(fragment = """
 		   #include <boost/filesystem.hpp>\n
    		   int main() { boost::filesystem::copy_file ("a", "b"); }\n
		   """,
                   msg = 'Checking for boost filesystem library',
                   libpath = '/usr/local/lib',
                   lib = ['boost_filesystem', 'boost_system'],
                   uselib_store = 'BOOST_FILESYSTEM')

    conf.recurse('test')

def build(bld):

    bld(source = 'libcxml.pc.in',
        version = VERSION,
        includedir = '%s/include' % bld.env.PREFIX,
        libs = "-L${libdir} -lcxml",
        install_path = '${LIBDIR}/pkgconfig')

    bld.recurse('src')
    bld.recurse('test')
