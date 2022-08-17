#pragma once

#include <glm/glm.hpp>
#include "../core/types.h"

namespace Engine {
    class Shape {
    public:
        enum ShapeType {
            SPHERE,
            BOX,
            CONVEX,
        };

        Shape() = default;
        virtual ~Shape() = default;

        virtual glm::vec3 get_center_mass() const { return center_mass;}
        virtual ShapeType get_type() const = 0;

    protected:
	    glm::vec3 center_mass;

    };

    class Sphere : public Shape {
    public:
        explicit Sphere(const f32& _radius) : radius{_radius} { center_mass = { 0.0f, 0.0f, 0.0f }; }
        
        ShapeType get_type() const override { return ShapeType::SPHERE; }
    public:
        f32 radius;
    };
}