#ifndef JCDP_UTIL_PROPERTIES_HPP_
#define JCDP_UTIL_PROPERTIES_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <fstream>
#include <list>
#include <stdexcept>
#include <string>

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

/******************************************************************************
 * @brief Thrown if a key that is not registered to the properties is
 *        encountered while parsing.
 ******************************************************************************/
class KeyNotRegisteredError : public std::runtime_error {
 public:
   //! Constructor of KeyNotRegisteredError.
   KeyNotRegisteredError(const std::string& key, const std::string& list = "");
};

/******************************************************************************
 * @brief Thrown if the specified configfile is unreadable or not a file.
 ******************************************************************************/
class BadConfigFileError : public std::runtime_error {
 public:
   //! Constructor of BadConfigFileError.
   BadConfigFileError();
};

class Properties;

/******************************************************************************
 * @brief Stores the key and a description of a property but no value.
 ******************************************************************************/
class BasicPropertyInfo {
 public:
   //! Default constructor of BasicPropertyInfo.
   BasicPropertyInfo(
        const std::string& key, const std::string& desc,
        void (*onr)(Properties*));

   //! Default destructor of BasicPropertyInfo.
   virtual ~BasicPropertyInfo() = default;

   //! Will pipe from a stream into the member variable pointed to by
   //! PropertyInfo and invoke _on_read(c).
   virtual auto from_pipe(std::istream&) -> void = 0;

   //! Wraps around from_pipe.
   auto from_str(const std::string& s) -> void;

   //! Return the value as a string.
   virtual auto to_string() const -> const std::string = 0;

   //! Get the key of the property.
   virtual auto key() const -> const std::string&;

   //! Get the description of the property.
   virtual auto desc() const -> const std::string&;

   //! Function pointer to execute something after reading the value.
   void (*m_on_read)(Properties*);

 private:
   //! Identifiers of this property.
   std::string m_key;

   //! Descriptor of this property.
   std::string m_desc;
};

/******************************************************************************
 * @brief Stores a property and its value.
 *
 * @tparam T Type of the stored value.
 ******************************************************************************/
template<typename T>
class PropertyInfo : public BasicPropertyInfo {
 public:
   //! The type of the value stored in here.
   using type = T;

   //! Implements pipe function by deferencing stored pointer _ptr.
   virtual auto from_pipe(std::istream& i) -> void override final;

   //! Return the value as a string.
   virtual auto to_string() const -> const std::string override final;

   //! Constructor, delegating most of its work to that of
   //! BasicPropertyInfo, just initializing the pointer.
   PropertyInfo(
        T* p, const std::string& k, const std::string& d,
        void (*onr)(Properties*));

 private:
   T* m_ptr;
};

/******************************************************************************
 * @brief Allows to parse Arguments to Properties from the command line and
 *        config files.
 *
 * Uses a list of pointers to BasicPropertyInfo to be able to associate a
 * type-safe class member of a class derived from Properties with a run-time
 * string. The Properties class has basic -help and -h capabilities, ie. ie will
 * print all possible keys of -h or -help are encountered. Also has the ability
 * to switch from command line parameter reading to config file reading with
 * -from-file=path. Functions can be registered to be executed with each put()
 * in a PropertyInfo.
 ******************************************************************************/
class Properties {
 public:
   //! Register a property.
   template<typename T>
   auto register_property(
        T& v, const std::string& key, const std::string& desc,
        void (*on_read)(Properties*) = [](Properties*) {}) -> void;

   //! Default constructor for Properties, used for default
   //! properties, such as help flag and swich-to-config-file flag.
   Properties() = default;

   //! The copy constructor is disabled.
   Properties(const Properties&) = delete;

   //! Destructor takes care of deleting dynamic memory pointed to
   //! by entries of _info.
   ~Properties();

   //! Finds the propertiy which is registered under key and pipes value
   //! into it. Also executes the associated _on_read.
   auto put(const std::string& key, std::ifstream& in) -> void;

   //! Parses a config file from a std::ifstream.
   auto parse_config(std::ifstream&& in) -> void;

   //! Prints the keys and descriptions of all registeres properties in a
   //! structured way.
   auto print_help(std::ostream& o) -> void;

   auto print_values(std::ostream& o) -> void;

 private:
   //! Contains pointers to all registered properties.
   std::list<BasicPropertyInfo*> m_info;

   //! Used to store the supplied nothing that comes with -help and -h into
   //! the waste. Therefore private.
   bool m_waste;
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

#include "jcdp/util/properties.inl"  // IWYU pragma: export

#endif  // JCDP_UTIL_PROPERTIES_HPP_
