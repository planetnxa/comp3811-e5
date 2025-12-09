#include "cone.hpp"

#include <numbers>

using namespace std;

SimpleMeshData make_cone( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform )
{

	//TODO: implement me

	vector <Vec3f> pos;
	float prevY = cos(0.f);
	float prevZ = sin(0.f);

	for (size_t i = 0; i < aSubdivs; i++) {
		float const angle = (i + 1) / float(aSubdivs) * 2.f * numbers::pi_v<float>;
		float y = cos(angle);
		float z = sin(angle);

		pos.emplace_back(Vec3f(0.f, prevY, prevZ));
		pos.emplace_back(Vec3f(0.f, y, z));
		pos.emplace_back(Vec3f(1,0, 0));


		prevY = y;
		prevZ = z;

	}



	if (aCapped) {
		
		//top cap
		prevY = cos(0.f);
		prevZ = sin(0.f);
		for (size_t i = 0; i < aSubdivs; i++) {
			float const angle = (i + 1) / float(aSubdivs) * 2.f * numbers::pi_v<float>;
			float y = cos(angle);
			float z = sin(angle);
			pos.emplace_back(Vec3f(0.f, prevY, prevZ));
			pos.emplace_back(Vec3f(0.f, y, z));
			pos.emplace_back(Vec3f(0.f, 0.f, 0.f));

			prevY = y;
			prevZ = z;
		}
	}

	for (auto& p : pos) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = aPreTransform * p4;
		t = t / t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}

	vector col(pos.size(), aColor);
	return SimpleMeshData(move(pos), move(col));
	//TODO: implement me
	
}

