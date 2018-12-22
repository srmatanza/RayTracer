#pragma once

#include "vec3.h"
#include "hitable.h"

vec3 random_in_unit_sphere() {
	vec3 p(0, 0, 0);
	do {
		p = 2.0*vec3(drand(), drand(), drand()) - vec3(1, 1, 1);

	} while (p.squared_length() >= 1.0);

	return p;
}

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& atten, ray& scattered) const = 0;
};

class lambertian : public material {
private:
	vec3 albedo;
public:
	lambertian(const vec3& a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& atten, ray& scattered) const {
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, target - rec.p);
		atten = albedo;
		return true;
	}
};

class metal : public material {
private:
	vec3 albedo;
	tfp fuzz;
public:
	metal(const vec3& a, tfp f) : albedo(a) {
		fuzz = fmin(1.0, f);
	}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& atten, ray& scattered) const {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
		atten = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}
};
