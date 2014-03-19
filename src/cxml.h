#ifndef LIBCXML_CXML_H
#define LIBCXML_CXML_H

#include <string>
#include <list>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <glibmm.h>

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
	char const * what () const throw () {
		return _message.c_str ();
	}

private:
	/** error message */
	std::string _message;
};

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
		std::string s = string_child (c);
		boost::erase_all (s, " ");
		return boost::lexical_cast<T> (s);
	}

	template <class T>
	boost::optional<T> optional_number_child (std::string c) const
	{
		boost::optional<std::string> s = optional_string_child (c);
		if (!s) {
			return boost::optional<T> ();
		}

		std::string t = s.get ();
		boost::erase_all (t, " ");
		return boost::optional<T> (boost::lexical_cast<T> (t));
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
		return boost::lexical_cast<T> (s);
	}

	template <class T>
	boost::optional<T> optional_number_attribute (std::string c) const
	{
		boost::optional<std::string> s = optional_string_attribute (c);
		if (!s) {
			return boost::optional<T> ();
		}

		std::string t = s.get ();
		boost::erase_all (t, " ");
		return boost::optional<T> (boost::lexical_cast<T> (t));
	}

	/** @return The content of this node */
	std::string content () const;

	/** @return namespace URI of this node */
	std::string namespace_uri () const;

	/** @return namespace prefix of this node */
	std::string namespace_prefix () const;

	boost::shared_ptr<Node> node_child (std::string) const;
	boost::shared_ptr<Node> optional_node_child (std::string) const;

	std::list<boost::shared_ptr<Node> > node_children (std::string) const;

	xmlpp::Node* node () const {
		return _node;
	}
	
protected:
	xmlpp::Node* _node;
	
private:
	mutable std::list<Glib::ustring> _taken;
};

typedef boost::shared_ptr<cxml::Node> NodePtr;

class Document : public Node
{
public:
	Document (std::string root_name);
	Document (std::string root_name, boost::filesystem::path);

	void read_file (boost::filesystem::path);
	void read_stream (std::istream &);
	
	virtual ~Document ();

private:
	void take_root_node ();
	
	xmlpp::DomParser* _parser;
	std::string _root_name;
};

}

#endif
