/*
    Copyright (C) 2012 Carl Hetherington <cth@carlh.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <cmath>
#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>
#include "cxml.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE libdcp_test
#include <boost/test/unit_test.hpp>

using std::string;
using std::cout;
using std::vector;
using std::list;
using boost::shared_ptr;

BOOST_AUTO_TEST_CASE (test)
{
	cxml::Document document ("A");
	document.read_file ("test/ref/a.xml");

	BOOST_CHECK_EQUAL (document.string_child("B"), "42");
	BOOST_CHECK_EQUAL (document.number_child<int>("B"), 42);
	BOOST_CHECK_EQUAL (document.number_child<float>("B"), 42);
	BOOST_CHECK_EQUAL (document.string_child("C"), "fred");
	BOOST_CHECK_EQUAL (document.number_child<double>("D"), 42.9);
	BOOST_CHECK_EQUAL (document.string_child("E"), "yes");
	BOOST_CHECK_EQUAL (document.bool_child("E"), true);
	BOOST_CHECK_THROW (document.bool_child("F"), cxml::Error);

	BOOST_CHECK (document.optional_string_child("B"));
	BOOST_CHECK_EQUAL (document.optional_string_child("B").get(), "42");
	BOOST_CHECK (document.optional_number_child<int>("B"));
	BOOST_CHECK_EQUAL (document.optional_number_child<int>("B").get(), 42);
	BOOST_CHECK (document.optional_number_child<float>("B"));
	BOOST_CHECK_EQUAL (document.optional_number_child<float>("B").get(), 42);
	BOOST_CHECK (document.optional_string_child("C"));
	BOOST_CHECK_EQUAL (document.optional_string_child("C").get(), "fred");
	BOOST_CHECK (document.optional_number_child<double>("D"));
	BOOST_CHECK_EQUAL (document.optional_number_child<double>("D").get(), 42.9);
	BOOST_CHECK (document.optional_string_child("E"));
	BOOST_CHECK_EQUAL (document.optional_string_child("E").get(), "yes");
	BOOST_CHECK (document.optional_bool_child("E"));
	BOOST_CHECK_EQUAL (document.optional_bool_child("E").get(), true);
	BOOST_CHECK_THROW (document.optional_bool_child("F"), cxml::Error);
	BOOST_CHECK (!document.optional_bool_child("G"));
}
