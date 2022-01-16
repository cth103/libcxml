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

#ifndef LIBCXML_CXML_H
#define LIBCXML_CXML_H

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <exception>
#include <memory>
#include <string>
#include <vector>

/* Hack for OS X compile failure; see https://bugs.launchpad.net/hugin/+bug/910160 */
#ifdef check
#undef check
#endif

namespace xmlpp {
	class Node;
	class DomParser;
}

namespace cxml {

/** @brief An error */
class Error : public std::exception
{
public:
	/** Construct an Error exception.
	 *  @param message Error message.
	 */
	Error (std::string const & message) : _message (message) {}

	/** Error destructor */
	~Error () throw () {}

	/** @return error message.  Caller must not free the returned
	 *  value.
	 */
	char const * what () const noexcept override {
		return _message.c_str ();
	}

private:
	/** error message */
	std::string _message;
};

/** A sort-of version of boost::lexical_cast that does uses the "C"
 *  locale (i.e. no thousands separators and a . for the decimal separator).
 */
template <typename P, typename Q>
P
raw_convert (Q)
{
	/* We can't write a generic version of raw_convert; all required
	   versions must be specialised.
	*/
	BOOST_STATIC_ASSERT (sizeof(Q) == 0);
}

template <>
int
raw_convert (std::string v);

template <>
unsigned int
raw_convert (std::string v);

template <>
long int
raw_convert (std::string v);

template <>
long unsigned int
raw_convert (std::string v);

template <>
long long
raw_convert (std::string v);

template <>
long long unsigned
raw_convert (std::string v);

template <>
float
raw_convert (std::string v);

template <>
double
raw_convert (std::string v);

/** @brief A wrapper for a xmlpp::Node which simplifies parsing */
class Node
{
public:
	Node ();

	/** Construct a Node from an xmlpp::Node.  This class will
	 *  not destroy the xmlpp::Node.
	 *  @param node xmlpp::Node.
	 */
	Node (xmlpp::Node* node);

	std::string name () const;

	/* A set of methods which look up a child of this node by
	 * its name, and return its contents as some type or other.
	 *
	 * If, for example, this object has been created with
	 * a node named "Fred", we might have the following XML:
	 *
	 * <Fred>
	 *   <Jim>42</Jim>
	 * </Fred>
	 *
	 * string_child ("Jim") would return "42"
	 * number_child<int64_t> ("Jim") would return 42.
	 * ...and so on.
	 *
	 * The methods not marked "optional" will throw an exception
	 * if the child node is not present.  The "optional" methods
	 * will return an empty boost::optional<> in that case.
	 *
	 * All methods will also throw an exception if there is more
	 * than one of the specified child node.
	 */

	std::string string_child (std::string c) const;
	boost::optional<std::string> optional_string_child (std::string) const;

	bool bool_child (std::string) const;
	boost::optional<bool> optional_bool_child (std::string) const;

	template <class T>
	T number_child (std::string c) const
	{
		auto s = string_child (c);
		boost::erase_all (s, " ");
		return raw_convert<T> (s);
	}

	template <class T>
	boost::optional<T> optional_number_child (std::string c) const
	{
		auto s = optional_string_child (c);
		if (!s) {
			return {};
		}

		auto t = s.get ();
		boost::erase_all (t, " ");
		return raw_convert<T> (t);
	}

	/** This will mark a child as to be ignored when calling done() */
	void ignore_child (std::string) const;

	/** Check whether all children of this Node have been looked up
	 *  or passed to ignore_child().  If not, an exception is thrown.
	 */
	void done () const;

	/* These methods look for an attribute of this node, in the
	 * same way as the child methods do.
	 */

	std::string string_attribute (std::string) const;
	boost::optional<std::string> optional_string_attribute (std::string) const;

	bool bool_attribute (std::string) const;
	boost::optional<bool> optional_bool_attribute (std::string) const;

	template <class T>
	T number_attribute (std::string c) const
	{
		std::string s = string_attribute (c);
		boost::erase_all (s, " ");
		return raw_convert<T> (s);
	}

	template <class T>
	boost::optional<T> optional_number_attribute (std::string c) const
	{
		auto s = optional_string_attribute (c);
		if (!s) {
			return boost::optional<T> ();
		}

		auto t = s.get ();
		boost::erase_all (t, " ");
		return raw_convert<T> (t);
	}

	/** @return The text content of this node (excluding comments or CDATA) */
	std::string content () const;

	/** @return namespace URI of this node */
	std::string namespace_uri () const;

	/** @return namespace prefix of this node */
	std::string namespace_prefix () const;

	std::shared_ptr<Node> node_child (std::string) const;
	std::shared_ptr<Node> optional_node_child (std::string) const;

	std::vector<std::shared_ptr<Node>> node_children () const;
	std::vector<std::shared_ptr<Node>> node_children (std::string) const;

	xmlpp::Node* node () const {
		return _node;
	}

protected:
	xmlpp::Node* _node;

private:
	mutable std::vector<std::string> _taken;
};

typedef std::shared_ptr<cxml::Node> NodePtr;
typedef std::shared_ptr<const cxml::Node> ConstNodePtr;

class Document : public Node
{
public:
	Document ();
	Document (std::string root_name);
	Document (std::string root_name, boost::filesystem::path);

	Document (Document const&) = delete;
	Document& operator= (Document const&) = delete;

	virtual ~Document ();

	void read_file (boost::filesystem::path);
	void read_string (std::string);

	std::string root_name () const {
		return _root_name;
	}

private:
	void take_root_node ();

	xmlpp::DomParser* _parser;
	std::string _root_name;
};

}

#endif
