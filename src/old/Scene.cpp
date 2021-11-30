#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

Vector3f Scene::castRay(const Ray &ray, int depth) const
{
	Intersection inter = intersect(ray);
	
	if (inter.happened)
	{
		// If the ray hits the light source for the first time, return directly
		if (inter.m->hasEmission())
		{
			if (depth == 0) 
			{
				return inter.m->getEmission();
			}
			else return Vector3f(0, 0, 0);
		}

		Vector3f L_dir(0, 0, 0);
		Vector3f L_indir(0, 0, 0);

		// Random sample the light and use the result of the sampling to determine whether the ray hits the light source
		Intersection lightInter;
		float pdf_light = 0.0f;
		sampleLight(lightInter, pdf_light);

		// object surface normal
		auto& N = inter.normal;
		// light surface normal
		auto& NN = lightInter.normal;

		auto& objPos = inter.coords;
		auto& lightPos = lightInter.coords;

		auto diff = lightPos - objPos;
		auto lightDir = diff.normalized();
		float lightDistance = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		Ray light(objPos, lightDir);
		Intersection light2obj = intersect(light);

		// If the reflection hits the light source
		if (light2obj.happened && (light2obj.coords - lightPos).norm() < 1e-2)
		{
			Vector3f f_r = inter.m->eval(ray.direction, lightDir, N);
			L_dir = lightInter.emit * f_r * dotProduct(lightDir, N) * dotProduct(-lightDir, NN) / lightDistance / pdf_light;
		}

		if (get_random_float() < RussianRoulette)
		{
			
			Vector3f nextDir = inter.m->sample(ray.direction, N).normalized();

			Ray nextRay(objPos, nextDir);
			Intersection nextInter = intersect(nextRay);
			if (nextInter.happened && !nextInter.m->hasEmission())
			{
				float pdf = inter.m->pdf(ray.direction, nextDir, N);
				Vector3f f_r = inter.m->eval(ray.direction, nextDir, N);
				L_indir = castRay(nextRay, depth + 1) * f_r * dotProduct(nextDir, N) / pdf / RussianRoulette;
			}
		}
		return L_dir + L_indir;
	}

	return Vector3f(0.0, 0.0, 0.0);
}
