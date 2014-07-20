/*
    Copyright (C) 2012-2014 Carl Hetherington <cth@carlh.net>

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

#include <sstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <libxml++/libxml++.h>
#include "cxml.h"

using namespace std;
using namespace boost;

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
	assert (_node);
	return _node->get_name ();
}

shared_ptr<cxml::Node>
cxml::Node::node_child (string name) const
{
	list<shared_ptr<cxml::Node> > n = node_children (name);
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
	list<shared_ptr<cxml::Node> > n = node_children (name);
	if (n.size() > 1) {
		throw cxml::Error ("duplicate XML tag " + name);
	} else if (n.empty ()) {
		return shared_ptr<cxml::Node> ();
	}
	
	return n.front ();
}

list<shared_ptr<cxml::Node> >
cxml::Node::node_children (string name) const
{
	/* XXX: using find / get_path should work here, but I can't follow
	   how get_path works.
	*/

	xmlpp::Node::NodeList c = _node->get_children ();
	
	list<shared_ptr<cxml::Node> > n;
	for (xmlpp::Node::NodeList::iterator i = c.begin (); i != c.end(); ++i) {
		if ((*i)->get_name() == name) {
			n.push_back (shared_ptr<Node> (new Node (*i)));
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
	list<shared_ptr<Node> > nodes = node_children (c);
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
	string const s = string_child (c);
	return (s == "1" || s == "yes");
}

optional<bool>
cxml::Node::optional_bool_child (string c) const
{
	optional<string> s = optional_string_child (c);
	if (!s) {
		return optional<bool> ();
	}
	
	return (s.get() == "1" || s.get() == "yes");
}

void
cxml::Node::ignore_child (string name) const
{
	_taken.push_back (name);
}

string
cxml::Node::string_attribute (string name) const
{
	xmlpp::Element const * e = dynamic_cast<const xmlpp::Element *> (_node);
	if (!e) {
		throw cxml::Error ("missing attribute");
	}
	
	xmlpp::Attribute* a = e->get_attribute (name);
	if (!a) {
		throw cxml::Error ("missing attribute");
	}

	return a->get_value ();
}

optional<string>
cxml::Node::optional_string_attribute (string name) const
{
	xmlpp::Element const * e = dynamic_cast<const xmlpp::Element *> (_node);
	if (!e) {
		return optional<string> ();
	}
	
	xmlpp::Attribute* a = e->get_attribute (name);
	if (!a) {
		return optional<string> ();
	}

	return string (a->get_value ());
}

bool
cxml::Node::bool_attribute (string name) const
{
	string const s = string_attribute (name);
	return (s == "1" || s == "yes");
}

optional<bool>
cxml::Node::optional_bool_attribute (string name) const
{
	optional<string> s = optional_string_attribute (name);
	if (!s) {
		return optional<bool> ();
	}

	return (s.get() == "1" || s.get() == "yes");
}

void
cxml::Node::done () const
{
	xmlpp::Node::NodeList c = _node->get_children ();
	for (xmlpp::Node::NodeList::iterator i = c.begin(); i != c.end(); ++i) {
		if (dynamic_cast<xmlpp::Element *> (*i) && find (_taken.begin(), _taken.end(), (*i)->get_name()) == _taken.end ()) {
			throw cxml::Error ("unexpected XML node " + (*i)->get_name());
		}
	}
}

string
cxml::Node::content () const
{
	string content;
	
        xmlpp::Node::NodeList c = _node->get_children ();
	for (xmlpp::Node::NodeList::const_iterator i = c.begin(); i != c.end(); ++i) {
		xmlpp::ContentNode const * v = dynamic_cast<xmlpp::ContentNode const *> (*i);
		if (v) {
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
cxml::Document::read_file (filesystem::path file)
{
	if (!filesystem::exists (file)) {
		throw cxml::Error ("XML file does not exist");
	}
	
	_parser->parse_file (file.string ());
	take_root_node ();
}

void
cxml::Document::read_stream (istream& stream)
{
	_parser->parse_stream (stream);
	take_root_node ();
}

void
cxml::Document::read_string (string s)
{
	stringstream t (s);
	_parser->parse_stream (t);
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
		throw cxml::Error ("unrecognised root node");
	} else if (_root_name.empty ()) {
		_root_name = _node->get_name ();
	}
}

