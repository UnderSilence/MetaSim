#ifndef METASIM_DATA_HPP
#define METASIM_DATA_HPP

#include "Core/data_array.hpp"
#include <functional>

namespace MS {

/* type handler */
template <typename T> struct AttributeTag {
  std::string type_name;
  size_t type_hash;

  AttributeTag(const char *type_name)
      : type_name(type_name), type_hash(std::hash<std::string>()(type_name)) {}

  AttributeTag(const std::string &type_name)
      : type_name(type_name), type_hash(std::hash<std::string>()(type_name)) {}
};

// DataBase for manifolds attributes
class DataContainer {};

class DataContainerIterator {};

} // namespace MS

#endif // METASIM_DATA_HPP
