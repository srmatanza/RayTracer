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

class dielectric : public material {
private:
	tfp ref_idx;
public:
	dielectric(tfp ri) : ref_idx(ri) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& atten, ray& scattered) const {
		vec3 outward_normal;
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		tfp ni_over_nt;
		atten = vec3(1.0, 1.0, 1.0);
		vec3 refracted;
		tfp reflect_prob, cosine;
		if (dot(r_in.direction(), rec.normal) > 0) {
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
			cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		else {
			outward_normal = rec.normal;
			ni_over_nt = 1.0/ref_idx;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}

		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
			reflect_prob = schlick(cosine, ref_idx);
		}
		else {
			scattered = ray(rec.p, reflected);
			reflect_prob = 1.0;
		}

		if (drand() < reflect_prob) {
			scattered = ray(rec.p, reflected);
		}
		else {
			scattered = ray(rec.p, refracted);
		}
		return true;
	}
};