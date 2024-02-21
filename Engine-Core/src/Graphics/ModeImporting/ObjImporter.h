#pragma once

#include "Importer.h"

namespace TooGoodEngine {

	namespace ObjStates {

		const std::string Comment = "#";
		const std::string Vertex = "v";
		const std::string VertexNormal = "vn";
		const std::string VertexTextureCoordinate = "vt";
		const std::string Face = "f";

	}

	class ObjModelImporter : public ModelImporter
	{
	public:
		ObjModelImporter();
		virtual ~ObjModelImporter();

		virtual ModelData Read(const std::filesystem::path& path) override;
		virtual Model	  ImportModel(const std::filesystem::path& path) override { return {}; };
	};
}