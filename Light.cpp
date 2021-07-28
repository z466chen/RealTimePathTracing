// Termm--Fall 2020

#include <iostream>

#include <glm/ext.hpp>

#include "Light.hpp"

#include "UboConstructor.hpp"

Light::Light()
  : colour(0.0, 0.0, 0.0),
    position(0.0, 0.0, 0.0)
{
  falloff[0] = 1.0;
  falloff[1] = 0.0;
  falloff[2] = 0.0;
}

std::ostream& operator<<(std::ostream& out, const Light& l)
{
  out << "L[" << glm::to_string(l.colour) 
  	  << ", " << glm::to_string(l.position) << ", ";
  for (int i = 0; i < 3; i++) {
    if (i > 0) out << ", ";
    out << l.falloff[i];
  }
  out << "]";
  return out;
}

int Light::construct() const {
  int id = UboConstructor::light_arr.size();
  UboConstructor::light_arr.emplace_back(UboLight());
  UboConstructor::light_arr[id].color = colour;
  UboConstructor::light_arr[id].position = position;
  UboConstructor::light_arr[id].falloff = glm::vec3(falloff[0],falloff[1],falloff[2]);
  return id;
}