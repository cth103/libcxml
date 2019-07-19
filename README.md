libcxml
=======

This is a very small library which provides a slightly tidier
C++ API than libxml++ for parsing HTML.

For example:

    cxml::Document doc("RootNodeName");
    doc.read("foo.xml");

    /* Get the contents of child node <Fred>, throwing an
     * exception if it does not exist
     */
    string s = doc.string_child("Fred");

    /* Get the contents of child node <Jim>, or 42 if it
     * does not exist
     */
    int i = doc.optional_number_child<int>("Jim").get_value_or(42);

    /* Get the contents of <Roger> within <Sheila>,
     * throwing an exception if either node
     * is not present.
     */
    double d = doc.node_child("Sheila").number_child<double>("Roger");

For full details, see src/cxml.h

Building
-------

    ./waf configure
    ./waf build
    sudo ./waf install

### Build options

    --target-windows      set up to do a cross-compile to Windows
    --enable-debug        build with debugging information and without optimisation
    --static              build statically
    --disable-tests       disable building of tests
    --force-cpp11         force use of C++11

### Dependencies

- libxml++ (and libffi)


---
Bug reports and queries to Carl Hetherington <cth@carlh.net>
