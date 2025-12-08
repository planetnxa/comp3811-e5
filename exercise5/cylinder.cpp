#include "cylinder.hpp"

#include <numbers>


using namespace std;
SimpleMeshData make_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform )
{//TODO: implement me

	vector <Vec3f> pos;
	vector<Vec3f> nrm;

	float prevY = cos(0.f);
	float prevZ = sin(0.f);

	for (size_t i = 0; i < aSubdivs; i++) {
		float const angle = (i + 1) / float(aSubdivs) * 2.f * numbers::pi_v<float>;
		float y = cos(angle);
		float z = sin(angle);

		pos.emplace_back(Vec3f(0.f, prevY, prevZ));
		nrm.emplace_back(Vec3f(0.f, prevY, prevZ));

		pos.emplace_back(Vec3f(0.f, y, z));
		nrm.emplace_back(Vec3f(0.f, y, z));


		pos.emplace_back(Vec3f(1.f, prevY, prevZ));
		nrm.emplace_back(Vec3f(0.f, prevY, prevZ));

		pos.emplace_back(Vec3f(0.f, y, z));
		nrm.emplace_back(Vec3f(0.f, y, z));

		pos.emplace_back(Vec3f(1.f, y, z));
		nrm.emplace_back(Vec3f(0.f, y, z));

		pos.emplace_back(Vec3f(1.f, prevY, prevZ));
		nrm.emplace_back(Vec3f(0.f, prevY, prevZ));

		prevY = y;
		prevZ = z;

	}



	if (aCapped) {
		//bottom cap
		prevY = cos(0.f);
		prevZ = sin(0.f);
		for (size_t i = 0; i < aSubdivs; i++) {
			float const angle = (i + 1) / float(aSubdivs) * 2.f * numbers::pi_v<float>;
			float y = cos(angle);
			float z = sin(angle);
			pos.emplace_back(Vec3f(0.f, 0.f, 0.f));
			nrm.emplace_back(Vec3f(-1.f, 0.f, 0.f));
			pos.emplace_back(Vec3f(0.f, y, z));
			nrm.emplace_back(Vec3f(-1.f, 0.f, 0.f));
			pos.emplace_back(Vec3f(0.f, prevY, prevZ));
			nrm.emplace_back(Vec3f(-1.f, 0.f, 0.f));

			prevY = y;
			prevZ = z;
		}
		//top cap
		prevY = cos(0.f);
		prevZ = sin(0.f);
		for (size_t i = 0; i < aSubdivs; i++) {
			float const angle = (i + 1) / float(aSubdivs) * 2.f * numbers::pi_v<float>;
			float y = cos(angle);
			float z = sin(angle);
			pos.emplace_back(Vec3f(1.f, prevY, prevZ));
			nrm.emplace_back(Vec3f(1.f, 0.f, 0.f));
			pos.emplace_back(Vec3f(1.f, y, z));
			nrm.emplace_back(Vec3f(1.f, 0.f, 0.f));
			pos.emplace_back(Vec3f(1.f, 0.f, 0.f));
			nrm.emplace_back(Vec3f(1.f, 0.f, 0.f));


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

	Mat44f normalMat = transpose(invert(aPreTransform));
	for (auto& n : nrm) {
		Vec4f n4{ n.x, n.y, n.z, 0.f }; // direction -> w = 0
		Vec4f nt = normalMat * n4;
		Vec3f nn{ nt.x, nt.y, nt.z };
		n = normalize(nn);
	}
	return SimpleMeshData(move(pos), move(col), move(nrm));
}
