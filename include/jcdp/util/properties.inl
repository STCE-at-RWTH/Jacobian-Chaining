#ifndef JCDP_INCLUDE_UTIL_PROPERTIES_INL_
#define JCDP_INCLUDE_UTIL_PROPERTIES_INL_

// IWYU pragma: private, include "jcdp/util/properties.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstddef>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <filesystem>

#include "jcdp/util/properties.hpp"  // IWYU pragma: keep

// >>>>>>>>>>>>>>>>>>>>>> TEMPLATE AND INLINE CONTENTS <<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

// ------------------------- KeyNotRegisteredError -------------------------- //

/******************************************************************************
 * @brief Constructor of KeyNotRegisteredError.
 *
 * @param[in] key Name of the key that is not registered.
 * @param[in] list List of known keys.
 ******************************************************************************/
inline KeyNotRegisteredError::KeyNotRegisteredError(
     const std::string& key, const std::string& list)
   : std::runtime_error(
          "The key \"" + key + "\" is not registered here!" +
          ((list == "") ? "" : " Known keys are:\n") + list) {}

// --------------------------- BadConfigFileError --------------------------- //

/******************************************************************************
 * @brief Constructor of BadConfigFileError.
 ******************************************************************************/
inline BadConfigFileError::BadConfigFileError()
   : std::runtime_error("The specified config file is invalid or unreadable!") {
}

// --------------------- Properties - BasicPropertyInfo --------------------- //

/******************************************************************************
 * @brief Default constructor of BasicPropertyInfo.
 *
 * @param[in] k std::string Key to register the property under.
 * @param[in] d std::string Description of the property.
 * @param[in] onr void (*)(Properties*) Function to be executed on reading
 *                this property.
 ******************************************************************************/
inline BasicPropertyInfo::BasicPropertyInfo(
     const std::string& key, const std::string& desc, void (*onr)(Properties*))
   : m_on_read(onr), m_key(key), m_desc(desc) {}

/******************************************************************************
 * @brief Wraps around pipe.
 *
 * @param[in] String to initialize the property from.
 ******************************************************************************/
inline auto BasicPropertyInfo::from_str(const std::string& s) -> void {
   std::stringstream sst;
   sst << s;
   from_pipe(sst);
}

/******************************************************************************
 * @brief Get the key of the property.
 *
 * @returns The key of the property.
 ******************************************************************************/
inline auto BasicPropertyInfo::key() const -> const std::string& {
   return m_key;
}

/******************************************************************************
 * @brief Get the description of the property.
 *
 * @returns The description of the property.
 ******************************************************************************/
inline auto BasicPropertyInfo::desc() const -> const std::string& {
   return m_desc;
}

// ----------------------- Properties - PropertyInfo ------------------------ //

/******************************************************************************
 * @brief Implements pipe function by deferencing stored pointer m_ptr.
 *
 * @param[in] i std::ifstream& The stream to intialize the property from.
 ******************************************************************************/
template<typename T>
inline auto PropertyInfo<T>::from_pipe(std::istream& i) -> void {
   i >> *(this->m_ptr);
}

template<typename T1, typename T2>
inline auto PropertyInfo<std::pair<T1, T2>>::from_pipe(std::istream& i)
     -> void {
   i >> this->m_ptr->first >> this->m_ptr->second;
}

template<typename T>
inline auto PropertyInfo<std::vector<T>>::from_pipe(std::istream& i) -> void {
   this->m_ptr->clear();

   std::string line;
   i >> line;
   std::istringstream vec_elems(line);

   std::string item;
   while (std::getline(vec_elems, item, ',')) {
      this->m_ptr->emplace_back();
      std::istringstream(item) >> this->m_ptr->back();
   }
}

//! Return the value as a string.
template<typename T>
inline auto PropertyInfo<T>::to_string() const -> const std::string {
   return std::to_string(*(this->m_ptr));
}

template<typename T1, typename T2>
inline auto PropertyInfo<std::pair<T1, T2>>::to_string() const
     -> const std::string {
   return std::to_string(this->m_ptr->first) + " " +
          std::to_string(this->m_ptr->second);
}

template<typename T>
inline auto PropertyInfo<std::vector<T>>::to_string() const
     -> const std::string {
   std::ostringstream oss;
   for (std::size_t i = 0; i < this->m_ptr->size(); ++i) {
      if (i != 0) {
         oss << ",";
      }
      oss << this->m_ptr->at(i);
   }
   return oss.str();
}

/******************************************************************************
 * @brief Constructor, delegating most of its work to that of
 *        BasicPropertyInfo, just initializing the pointer.
 *
 * @param[in] p pointer to T, the value of this property.
 * @param[in] k std::string Key to register the property under.
 * @param[in] d std::string Description of the property.
 * @param[in] onr void (*)(Properties*) Function to be executed on reading
 *                this property.
 ******************************************************************************/
template<typename T>
inline PropertyInfoBase<T>::PropertyInfoBase(
     T* p, const std::string& k, const std::string& d, void (*onr)(Properties*))
   : BasicPropertyInfo(k, d, onr), m_ptr(p) {}

// ------------------------------- Properties ------------------------------- //

/******************************************************************************
 * @brief Register a property.
 *
 * @tparam T Type of the value.
 * @param[in] v Reference to the value.
 * @param[in] key key Key to register the property under.
 * @param[in] desc Description of the property to be printed when help
 *                 flag is encountered.
 * @param[in] on_read Pointer to function to be called after setting
 *                    the property value.
 ******************************************************************************/
template<typename T>
inline auto Properties::register_property(
     T& v, const std::string& key, const std::string& desc,
     void (*on_read)(Properties*)) -> void {
   m_info.emplace_back(new PropertyInfo<T>(&v, key, desc, on_read));
}

/******************************************************************************
 * @brief Destructor takes care of deleting dynamic memory pointed to
 *        by entries of m_info.
 ******************************************************************************/
inline Properties::~Properties() {
   for (auto& i : m_info) {
      delete i;
   }
}

/******************************************************************************
 * @brief Finds the propertiy which is registered under key and pipes value
 *        into it. Also executes the associated m_on_read.
 *
 * @param[in] key The key into which to put the value.
 * @param[in] value The value to write\. Will be converted from string.
 * @param[in] skip_not_registered_keys Ignore when we encounter an unknown key.
 ******************************************************************************/
inline auto Properties::put(
     const std::string& key, std::ifstream& in,
     const bool skip_not_registered_keys) -> void {

   for (auto& pi : m_info) {
      if (key == pi->key()) {
         pi->from_pipe(in);
         pi->m_on_read(this);
         return;
      }
   }
   if (!skip_not_registered_keys) {
      throw KeyNotRegisteredError(key);
   }
}

/******************************************************************************
 * @brief Parses a config file from a std::ifstream.
 *
 * First checks if the file may be empty or invalid.
 *
 * @param[in] filename File from which to read the properties.
 * @param[in] skip_not_registered_keys Ignore when we encounter an unknown key.
 ******************************************************************************/
inline auto Properties::parse_config(
     const std::filesystem::path& config_filename,
     const bool skip_not_registered_keys) -> void {

   std::ifstream in(config_filename);
   if (in.eof() || in.fail() || !in.good()) {
      throw BadConfigFileError();
   }

   std::string key;
   while (in >> key) {
      put(key, in, skip_not_registered_keys);
   }
}

/******************************************************************************
 * @brief Prints the keys and descriptions of all registeres properties in a
 *        structured way.
 *
 * @param[in] o Reference to std::ofstream to print the description
 *              of the properties.
 ******************************************************************************/
inline auto Properties::print_help(std::ostream& o) -> void {
   size_t l = 0;
   for (auto& pi : m_info) {
      l = (pi->key().length() > l) ? pi->key().length() : l;
   }

   for (auto& pi : m_info) {
      o.width(l);
      o << pi->key();
      o << ": " << pi->desc() << std::endl;
   }
}

inline auto Properties::print_values(std::ostream& o) -> void {
   size_t l = 0;
   for (auto& pi : m_info) {
      l = (pi->key().length() > l) ? pi->key().length() : l;
   }

   for (auto& pi : m_info) {
      o.width(l);
      o << pi->key();
      o << ": " << pi->to_string() << std::endl;
   }
}

}  // namespace jcdp

#endif  // JCDP_INCLUDE_UTIL_PROPERTIES_INL_
