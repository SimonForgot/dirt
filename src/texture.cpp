#include<dirt/texture.h>

static Vec3f* perlin_generate() {
	Vec3f* p = new Vec3f[256];
	for (int i = 0; i < 256; i++)
		p[i] = normalize(Vec3f(-1+2*randf(), -1 + 2 * randf(), -1 + 2 * randf()));
	return p;
}

void permute(int* p, int n) {
	for (int i = n - 1; i > 0; i--)
	{
		int target = int(randf() * (i + 1));
		int tmp = p[i];
		p[i] = p[target];
		p[target] = tmp;
	}return;
}

static int* perlin_generate_perm() {
	int* p = new int[256];
	for (int i = 0; i < 256; i++)
		p[i] = i;
	permute(p, 256);
	return p;
}
Vec3f* Perlin::ranvec = perlin_generate();
int* Perlin::perm_x = perlin_generate_perm();
int* Perlin::perm_y = perlin_generate_perm();
int* Perlin::perm_z = perlin_generate_perm();