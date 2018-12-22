#pragma once

#include "ray.h"
#include "material.h"

class material;

struct hit_record {
	tfp t;
	vec3 p;
	vec3 normal;
	material *pMat;
};

class hitable {
public:
	virtual bool hit(const ray& r, tfp t_min, tfp t_max, hit_record& rec) const = 0;
};
