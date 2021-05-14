//
// Created by Metal on 2021/5/14.
//

#ifndef METASIM_PARTICLES_HPP
#define METASIM_PARTICLES_HPP

#include "meta.hpp"

class IAttributeArray {
public:
    virtual ~IAttributeArray() = default;
};

template<typename T>
class AttributeArray : public IAttributeArray{
public:
    AttributeArray() = default;
    std::vector<T> attr_array;
};

class ParticleContainer {
public:
    template<typename T>
    std::vector<T>& SetAttrArray(const char* attr_name) {
        auto new_attr_array_ptr = std::make_shared<AttributeArray<T>>();
        attr_arrays_[attr_name] = new_attr_array_ptr;
        return new_attr_array_ptr->attr_array;
    }

    template<typename T>
    std::vector<T>& GetAttrArray(const char* attr_name) {
        auto attr_array_ptr = std::dynamic_pointer_cast<AttributeArray<T>>(attr_arrays_[attr_name]);
        return attr_array_ptr->attr_array;
    }

protected:
    std::unordered_map<const char*, std::shared_ptr<IAttributeArray>> attr_arrays_;
};


#endif //METASIM_PARTICLES_HPP
