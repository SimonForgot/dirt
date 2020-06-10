#include<dirt/common.h>
#include<fstream>
#include<functional>
using namespace std;
Vec3f  randomOnUnitSphere()
{
	float phi = randf() * 2 * M_PI;
	float cosTheta = 2 * randf() - 1;
	float 	sinTheta = sqrt(1 - cosTheta * cosTheta);
	float	x = cos(phi) * sinTheta;
	float	y = sin(phi) * sinTheta;
	float	z = cosTheta;
	return Vec3f(x, y, z);
}

Vec3f  randomOnUnitHemisphere()
{
	float phi = randf() * 2 * M_PI;
	float cosTheta = randf();
	float 	sinTheta = sqrt(1 - cosTheta * cosTheta);
	float	x = cos(phi) * sinTheta;
	float	y = sin(phi) * sinTheta;
	float	z = cosTheta;
	return Vec3f(x, y, z);
}
Vec3f   randomCosineHemisphere()
{
	float phi = randf() * 2 * M_PI;
	float cosTheta = sqrt(randf());
	float 	sinTheta = sqrt(1 - cosTheta * cosTheta);
	float	x = cos(phi) * sinTheta;
	float	y = sin(phi) * sinTheta;
	float	z = cosTheta;
	return Vec3f(x, y, z);
}
Vec3f  randomCosinePowerHemisphere(float e)
{
	float phi = randf() * 2 * M_PI;
	float cosTheta = powf(randf(),1/(e+1));
	float 	sinTheta = sqrt(1 - cosTheta * cosTheta);
	float	x = cos(phi) * sinTheta;
	float	y = sin(phi) * sinTheta;
	float	z = cosTheta;
	return Vec3f(x, y, z);
}
void fa()
{
	cout << "??";
}

constexpr int Pow(int base, int N)
{
	if (N == 1)return base;
	else return Pow(base, N - 1)*base;
}
int main()
{
	int x;
	cin >> x;
	auto value= Pow(2,x);
	constexpr auto v2= Pow(2, 4);
	return 0;
}