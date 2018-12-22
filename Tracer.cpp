// Tracer.cpp : Defines the entry point for the application.
//

#include "Tracer.h"

#include <stdlib.h>
#include <thread>
#include <mutex>

#include "vec3.h"
#include "ray.h"

#include "material.h"

#include "sphere.h"
#include "hitableList.h"

#include "camera.h"

using namespace std;

uint64_t gTotalRays = 0;

vec3 color(const ray& r, hitable *world, int &raysCast, int depth=0) {

	raysCast++;
	
	hit_record rec;
	if (world->hit(r, 0.001, TRACER_MAXFLOAT, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.pMat->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, raysCast, depth+1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		tfp t = 0.5*(unit_direction.y() + 1.0);
		return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}

int main(int argc, char** argv)
{
	int nx = 400;
	int ny = 200;
	int ns = 1;
	int maxThreads = 12;

	if (argc > 1) {
		long newSamples = atol(argv[1]);
		ns = static_cast<int>(newSamples);
	}

	if (argc > 3) {
		long newX = atol(argv[2]);
		long newY = atol(argv[3]);
		nx = static_cast<int>(newX);
		ny = static_cast<int>(newY);
	}

	if (argc > 4) {
		long tc = atol(argv[4]);
		maxThreads = tc;
	}

	//for (int i = 0; i < ns; i++) {
	//	cout << drand() << endl;
	//}

	uint32_t* imgBuffer = (uint32_t*)malloc(nx * ny * sizeof(uint32_t));

	hitable *list[4];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.2));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new metal(vec3(0.8, 0.8, 0.8), 1.0));
	hitable *world = new hitable_list(list, sizeof(list) / sizeof(*list));
	camera cam;

	mutex g_uvCoords_mutex;
	vector<pair<int, int>> uvCoords;
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			uvCoords.push_back(make_pair(i, j));
		}
	}

	auto colorPixel = [&imgBuffer, nx, ny, ns, &cam, &world](int u, int v)->int {
		// This is what should be scheduled
		int totalRays = 0;
		vec3 col(0, 0, 0);
		for (int s = 0; s < ns; s++) {
			tfp ux = tfp(u + drand()) / tfp(nx);
			tfp vx = tfp(v + drand()) / tfp(ny);
			ray r = cam.get_ray(ux, vx);
			//vec3 p = r.point_at(2.0);
			int raysCast=0;
			col += color(r, world, raysCast);
			totalRays += raysCast;
		}

		col /= tfp(ns);
		col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

		int ir = int(255.99*col[0]);
		int ig = int(255.99*col[1]);
		int ib = int(255.99*col[2]);

		uint32_t pixelColor = 0;
		pixelColor |= 0xff000000 & (ir << 24);
		pixelColor |= 0x00ff0000 & (ig << 16);
		pixelColor |= 0x0000ff00 & (ib << 8);
		imgBuffer[((ny - v - 1)*nx) + u] = pixelColor;

		return totalRays;
	};

	vector<thread*> threads;
	for (int i = 0; i < maxThreads; i++) {
		thread* t = new thread([&uvCoords, &colorPixel, &g_uvCoords_mutex]() {
			int batchSize = 16;
			while (uvCoords.size() > 0) {
				vector<pair<int, int>> pixelBatch;
				{
					lock_guard<mutex> guard(g_uvCoords_mutex);
					while(uvCoords.size() > 0 && pixelBatch.size()<batchSize) {
						pixelBatch.push_back(uvCoords.back());
						uvCoords.pop_back();
					}					
				}
				for (auto it = pixelBatch.begin(); it < pixelBatch.end(); it++) {
					gTotalRays += colorPixel(it->first, it->second);
				}
			}
		});
		threads.push_back(t);
	}

	for (auto it = threads.begin(); it < threads.end(); it++) {
		thread* t = *it;
		t->join();
	}
	
	ofstream ppm;
	ppm.open("image.ppm", ios::out | ios::binary);
	ppm << "P3" << endl << nx << " " << ny << "\n255\n";

	for (int i = 0; i < nx*ny; i++) {
		uint32_t p = imgBuffer[i];
		int ir = (p >> 24) & 0xff;
		int ig = (p >> 16) & 0xff;
		int ib = (p >> 8) & 0xff;
		ppm << ir << " " << ig << " " << ib << endl;
	}

	ppm.close();
	cout << "We cast " << gTotalRays << " rays." << endl;
	return 0;
}

