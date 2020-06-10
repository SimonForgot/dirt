#pragma once
#include <dirt/surface.h>
#include<dirt/common.h>
#include<dirt/image.h>
class Texture
{
public:
	virtual Vec3f value(const HitInfo&)const = 0;
};

class ConstantTexture :public Texture {
public:
	Color3f color;
	ConstantTexture(const json &j) {
		color=j.value("color", color);
	}
	Color3f value(const HitInfo&)const override {
		return color;
	}
};
class CheckerTexture :public Texture {
public:
	shared_ptr<const Texture> odd;
	shared_ptr<const Texture> even;
	float scale;
	CheckerTexture(const json& j) {
		//cout << j.dump()<<endl;
		//cout << j.at("even");
		scale = j.at("scale");
		//cout << scale;
		//cout<<json{ {"even",j.at("even")} }.dump();
		even = make_shared<ConstantTexture>(json{ {"color",j.at("even")} });
		odd = make_shared<ConstantTexture>(json{ {"color",j.at("odd")} });
	}
	Color3f value(const HitInfo& h)const override {
		float sines = sin(scale * h.p.x) * sin(scale * h.p.y) * sin(scale * h.p.z);
		if (sines < 0)
			return odd->value(h);
		else return even->value(h);
		//return Vec3f(1);
	}
};

inline float perlin_interp(Vec3f c[2][2][2], float u, float v, float w) {
	//hermite cubic
	float uu = u * u * (3 - 2 * u);
	float vv = v * v * (3 - 2 * v);
	float ww = w * w * (3 - 2 * w);
	float accum = 0;
	for(int i=0;i<2;i++)
		for(int j=0;j<2;j++)
			for (int k = 0; k < 2; k++)
			{
				Vec3f weight_v(u - i, v - j, w - k);
				accum += ((i * uu) + (1 - i) * (1 - uu)) *
					((j * vv) + (1 - j) * (1 - vv) )*
					(k * ww + (1 - k) * (1 - ww) )*
					dot(c[i][j][k],weight_v);
			}
	//return fabs(accum);
	return accum;
}
class Perlin
{
public:
	static Vec3f* ranvec;
	static int* perm_x;
	static int* perm_y;
	static int* perm_z;
	float noise(const Vec3f& p)const {
		float u = p.x - floor(p.x);
		float v = p.y - floor(p.y);
		float w = p.z - floor(p.z);
		int i = floor(p.x);
		int j = floor(p.y);
		int k = floor(p.z);
		Vec3f c[2][2][2];
		for(int di=0;di<2;di++)
			for(int dj=0;dj<2;dj++)
				for (int dk = 0; dk < 2; dk++)
				{
					//三线性插值
					c[di][dj][dk] = ranvec[perm_x[(i + di) & 255] ^
						perm_y[(j + dj) & 255] ^
						perm_z[(k + dk) & 255]];
				}	
		return perlin_interp(c, u, v, w);
	}
	float turb(const Vec3f& p, int depth = 7)const {
		float accum = 0;
		Vec3f temp_p = p; 
		float weight = 1.0;
			for (int i = 0; i < depth; i++) {
				accum += weight * noise(temp_p);
				weight *= 0.5;
				temp_p *= 2;
			}
		return fabs(accum);
	}
};


class MarbleTexture:public Texture {
public:
	Perlin noise;
	float scale;
	shared_ptr<const Texture> veins;
	shared_ptr<const Texture> base;
	MarbleTexture(const json& j) {
		//cout << j.dump()<<endl;
		//cout << j.at("even");
		scale = j.at("scale");
		//cout << scale;
		//cout << json{ {"even",j.at("even")} }.dump();
		veins = make_shared<ConstantTexture>(json{ {"color",j.at("veins")} });
		base = make_shared<ConstantTexture>(json{ {"color",j.at("base")} });
	}
	//virtual Vec3f value(const Hitinfo)
	Color3f value(const HitInfo& h)const override
	{
		return veins->value(h) + ( base->value(h)-veins->value(h) )
			* 0.5*(1+sin(scale*h.p.z)+19*noise.turb(scale * h.p));
		//return base->value(h)+(veins->value(h)-base->value(h)) * noise.noise(scale*h.p);
	}
};
class ImageTexture :public Texture {
public:
	Image3f image;
	ImageTexture(const json& j) {
		string filename = j.at("filename");
		//cout << filename<<endl;
		//string path = getFileResolver().resolve(filename).str();
		image.load(filename);
	}
	Color3f value(const HitInfo& h)const override
	{
		float u = h.uv.u;
		float v = h.uv.v;
		//auto p = h.p;
		int i = image.width() * u;
		int j = image.height() * (1-v);
		if (i < 0)
		{
			//cout << i << " ";
			i = 0; 
		}
		if (j < 0) {//cout << j << " ";
			j = 0; 
		}
		if (i > image.width() - 1)i = image.width() - 1;
		if (j > image.height() - 1)j = image.height() - 1;
		return image(i,j);
	}
};