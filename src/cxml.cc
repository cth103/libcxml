/*
    Copyright (C) 2012-2021 Carl Hetherington <cth@carlh.net>

    This file is part of libcxml.

    libcxml is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    libcxml is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libcxml.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "cxml.h"
#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdio>

using std::string;
using std::list;
using std::shared_ptr;
using boost::optional;

cxml::Node::Node ()
	: _node (0)
{

}

cxml::Node::Node (xmlpp::Node* node)
	: _node (node)
{

}

string
cxml::Node::name () const
{
	if (!_node) {
		throw Error ("No node to read name from");
	}
	return _node->get_name ();
}

shared_ptr<cxml::Node>
cxml::Node::node_child (string name) const
{
	auto const n = node_children (name);
	if (n.size() > 1) {
		throw cxml::Error ("duplicate XML tag " + name);
	} else if (n.empty ()) {
		throw cxml::Error ("missing XML tag " + name + " in " + _node->get_name());
	}

	return n.front ();
}

shared_ptr<cxml::Node>
cxml::Node::optional_node_child (string name) const
{
	auto const n = node_children (name);
	if (n.size() > 1) {
		throw cxml::Error ("duplicate XML tag " + name);
	} else if (n.empty ()) {
		return shared_ptr<cxml::Node> ();
	}

	return n.front ();
}

list<shared_ptr<cxml::Node>>
cxml::Node::node_children () const
{
	if (!_node) {
		throw Error ("No node to read children from");
	}

	list<shared_ptr<cxml::Node> > n;
	for (auto i: _node->get_children()) {
		n.push_back (shared_ptr<Node> (new Node (i)));
	}

	return n;
}

list<shared_ptr<cxml::Node>>
cxml::Node::node_children (string name) const
{
	/* XXX: using find / get_path should work here, but I can't follow
	   how get_path works.
	*/

	list<shared_ptr<cxml::Node> > n;
	for (auto i: _node->get_children()) {
		if (i->get_name() == name) {
			n.push_back (shared_ptr<Node> (new Node (i)));
		}
	}

	_taken.push_back (name);
	return n;
}

string
cxml::Node::string_child (string c) const
{
	return node_child(c)->content ();
}

optional<string>
cxml::Node::optional_string_child (string c) const
{
	auto const nodes = node_children (c);
	if (nodes.size() > 1) {
		throw cxml::Error ("duplicate XML tag " + c);
	}

	if (nodes.empty ()) {
		return optional<string> ();
	}

	return nodes.front()->content();
}

bool
cxml::Node::bool_child (string c) const
{
	auto const s = string_child (c);
	return (s == "1" || s == "yes" || s == "True");
}

optional<bool>
cxml::Node::optional_bool_child (string c) const
{
	auto const s = optional_string_child (c);
	if (!s) {
		return optional<bool> ();
	}

	return (s.get() == "1" || s.get() == "yes" || s.get() == "True");
}

void
cxml::Node::ignore_child (string name) const
{
	_taken.push_back (name);
}

string
cxml::Node::string_attribute (string name) const
{
	auto e = dynamic_cast<const xmlpp::Element *> (_node);
	if (!e) {
		throw cxml::Error ("missing attribute " + name);
	}

	auto a = e->get_attribute (name);
	if (!a) {
		throw cxml::Error ("missing attribute " + name);
	}

	return a->get_value ();
}

optional<string>
cxml::Node::optional_string_attribute (string name) const
{
	auto e = dynamic_cast<const xmlpp::Element *> (_node);
	if (!e) {
		return optional<string> ();
	}

	auto a = e->get_attribute (name);
	if (!a) {
		return optional<string> ();
	}

	return string (a->get_value ());
}

bool
cxml::Node::bool_attribute (string name) const
{
	auto const s = string_attribute (name);
	return (s == "1" || s == "yes");
}

optional<bool>
cxml::Node::optional_bool_attribute (string name) const
{
	auto s = optional_string_attribute (name);
	if (!s) {
		return optional<bool> ();
	}

	return (s.get() == "1" || s.get() == "yes");
}

void
cxml::Node::done () const
{
	for (auto i: _node->get_children()) {
		if (dynamic_cast<xmlpp::Element *> (i) && find (_taken.begin(), _taken.end(), i->get_name()) == _taken.end ()) {
			throw cxml::Error ("unexpected XML node " + i->get_name());
		}
	}
}

string
cxml::Node::content () const
{
	string content;

	for (auto i: _node->get_children()) {
		auto v = dynamic_cast<xmlpp::ContentNode const *> (i);
		if (v && dynamic_cast<xmlpp::TextNode const *>(v)) {
			content += v->get_content ();
		}
	}

	return content;
}

string
cxml::Node::namespace_uri () const
{
	return _node->get_namespace_uri ();
}

string
cxml::Node::namespace_prefix () const
{
	return _node->get_namespace_prefix ();
}

cxml::Document::Document (string root_name)
	: _root_name (root_name)
{
	_parser = new xmlpp::DomParser;
}

cxml::Document::Document (string root_name, boost::filesystem::path file)
	: _root_name (root_name)
{
	_parser = new xmlpp::DomParser ();
	read_file (file);
}

cxml::Document::Document ()
{
	_parser = new xmlpp::DomParser ();
}

cxml::Document::~Document ()
{
	delete _parser;
}

void
cxml::Document::read_file (boost::filesystem::path file)
{
	if (!boost::filesystem::exists (file)) {
		throw cxml::Error ("XML file " + file.string() + " does not exist");
	}

	_parser->parse_file (file.string ());
	take_root_node ();
}

void
cxml::Document::read_string (string s)
{
	_parser->parse_memory (s);
	take_root_node ();
}

void
cxml::Document::take_root_node ()
{
	if (!_parser) {
		throw cxml::Error ("could not parse XML");
	}

	_node = _parser->get_document()->get_root_node ();
	if (!_root_name.empty() && _node->get_name() != _root_name) {
		throw cxml::Error ("unrecognised root node " + _node->get_name() + " (expecting " + _root_name + ")");
	} else if (_root_name.empty ()) {
		_root_name = _node->get_name ();
	}
}

static
string
make_local (string v)
{
	auto lc = localeconv ();
	boost::algorithm::replace_all (v, ".", lc->decimal_point);
	/* We hope it's ok not to add in thousands separators here */
	return v;
}

template <typename P, typename Q>
P
locale_convert (Q x)
{
	/* We can't write a generic version of locale_convert; all required
	   versions must be specialised.
	*/
	BOOST_STATIC_ASSERT (sizeof(Q) == 0);
}

template<>
int
locale_convert (string x)
{
	int y = 0;
	sscanf (x.c_str(), "%d", &y);
	return y;
}

template<>
unsigned int
locale_convert (string x)
{
	unsigned int y = 0;
	sscanf (x.c_str(), "%u", &y);
	return y;
}

template<>
long int
locale_convert (string x)
{
	long int y = 0;
	sscanf (x.c_str(), "%ld", &y);
	return y;
}

template<>
long unsigned int
locale_convert (string x)
{
        long unsigned int y = 0;
#ifdef LIBCXML_WINDOWS
        __mingw_sscanf (x.c_str(), "%lud", &y);
#else
        sscanf (x.c_str(), "%lud", &y);
#endif
        return y;
}

template<>
long long
locale_convert (string x)
{
	long long y = 0;
#ifdef LIBCXML_WINDOWS
	__mingw_sscanf (x.c_str(), "%lld", &y);
#else
	sscanf (x.c_str(), "%lld", &y);
#endif
	return y;
}

template<>
long long unsigned
locale_convert (string x)
{
	long long unsigned y = 0;
#ifdef LIBCXML_WINDOWS
        __mingw_sscanf (x.c_str(), "%llud", &y);
#else
        sscanf (x.c_str(), "%llud", &y);
#endif
        return y;
}

template<>
float
locale_convert (string x)
{
	float y = 0;
	sscanf (x.c_str(), "%f", &y);
	return y;
}

template <>
double
locale_convert (string x)
{
	double y = 0;
	sscanf (x.c_str(), "%lf", &y);
	return y;
}

template <>
int
cxml::raw_convert (string v)
{
	return locale_convert<int> (make_local(v));
}

template <>
unsigned int
cxml::raw_convert (string v)
{
	return locale_convert<unsigned int> (make_local(v));
}

template <>
long int
cxml::raw_convert (string v)
{
	return locale_convert<long int> (make_local(v));
}

template <>
long unsigned int
cxml::raw_convert (string v)
{
	return locale_convert<long unsigned int> (make_local(v));
}

template <>
long long
cxml::raw_convert (string v)
{
	return locale_convert<long long> (make_local(v));
}

template <>
long long unsigned
cxml::raw_convert (string v)
{
	return locale_convert<long long unsigned> (make_local(v));
}

template <>
float
cxml::raw_convert (string v)
{
	return locale_convert<float> (make_local(v));
}

template <>
double
cxml::raw_convert (string v)
{
	return locale_convert<double> (make_local(v));
}
