#include "pch.h"
#include "ObjImporter.h"

#include <unordered_map>

namespace TooGoodEngine {

	ObjModelImporter::ObjModelImporter()
	{
	}
	ObjModelImporter::~ObjModelImporter()
	{
	}
	ModelData ObjModelImporter::Read(const std::filesystem::path& path)
	{
		FILE* file = nullptr;
		fopen_s(&file, path.string().c_str(), "r");

		if (!file)
			return {};

		std::vector<ModelVertex> Vertexs;
		std::vector<ModelVertex> VertexNormals;
		std::vector<TexelCoord> VertexTextureCoords;

		std::vector<Face> FaceData;

		char LineBuffer[30]{};
		while (fgets(LineBuffer, sizeof(LineBuffer), file) != nullptr)
		{
			std::string line = LineBuffer;

			if (line.starts_with(ObjStates::Comment))
				continue;
			else if (line.starts_with(ObjStates::VertexNormal))
			{
				std::istringstream VertexNormalData(line.substr(ObjStates::VertexNormal.size()));

				float a = 0.0f, b = 0.0f, c = 0.0f;

				VertexNormalData >> a >> b >> c;
				VertexNormals.push_back(ModelVertex(a, b, c));
			}
			else if (line.starts_with(ObjStates::VertexTextureCoordinate))
			{
				std::istringstream VertexTextureCoordinateData(line.substr(ObjStates::VertexTextureCoordinate.size()));

				float a = 0.0f, b = 0.0f;

				VertexTextureCoordinateData >> a >> b;
				VertexTextureCoords.push_back(TexelCoord(a, b));
			}
			else if (line.starts_with(ObjStates::Vertex))
			{
				std::istringstream VertexData(line.substr(ObjStates::Vertex.size()));
				float a = 0.0f, b = 0.0f, c = 0.0f;

				VertexData >> a >> b >> c;

				Vertexs.push_back(ModelVertex(a, b, c));
			}
			else if (line.starts_with(ObjStates::Face))
			{
				std::istringstream FaceStream(line.substr(ObjStates::Face.size()));
				std::string Token;

				while (std::getline(FaceStream, Token, ' '))
				{
					std::istringstream TokenStream(Token);
					std::string Value;

					Face face{};
					uint32_t* base = &face.a;
					bool Parse = false;

					while (std::getline(TokenStream, Value, '/'))
					{
						if (Value.empty() || Value == "\n")
						{
							*base = 0U;
							base++;
							continue;
						}

						*base = (uint32_t)std::stoul(Value);
						base++;

						Parse = true;
					}

					if (Parse)
						FaceData.push_back(face);
				}

			}
			else
				continue;
		}

		fclose(file);

		//no texture coordinates found will fill the data with 0s
		bool TexCoordsFound = true;

		if (VertexTextureCoords.size() == 0)
			TexCoordsFound = false;

		bool FoundNormals = true;

		if (VertexNormals.size() == 0)
			FoundNormals = false;

		std::vector<SingleVertexInBuffer> VertexBufferData;
		std::vector<uint32_t> Indices;

		std::unordered_map<Face, uint32_t> FaceToIndex;

		for (size_t i = 0; i < FaceData.size(); i++)
		{
			if (FaceToIndex.contains(FaceData[i]))
				Indices.push_back(FaceToIndex[FaceData[i]]);
			else
			{
				TexelCoord TexCoord{FLT_MIN, FLT_MIN};
				ModelVertex NormalVertex{ FLT_MIN, FLT_MIN, FLT_MIN };

				if (!TexCoordsFound)
					TexCoord.a = 0.0f, TexCoord.b = 0.0f;
				else
				{
					if ((int)FaceData[i].b - 1 < 0)
					{
						//TGE_LOG_WARN("Invalid face");
						TexCoord.a = 0.0f, TexCoord.b = 0.0f;
					}
					else
						TexCoord = VertexTextureCoords[FaceData[i].b - 1];
				}

				if (!FoundNormals)
					NormalVertex.a = 0.0f, NormalVertex.b = 0.0f, NormalVertex.c = 0.0f;
				else
				{
					if ((int)FaceData[i].c - 1 < 0)
					{
						//TGE_LOG_WARN("Invalid face");
						NormalVertex.a = 0.0f, NormalVertex.b = 0.0f, NormalVertex.c = 0.0f;
					}
					else
						NormalVertex = VertexNormals[FaceData[i].c - 1];
					
				}


				SingleVertexInBuffer d(Vertexs[FaceData[i].a - 1], NormalVertex, TexCoord);

				VertexBufferData.push_back(d);


				size_t CurrIndex = VertexBufferData.size() - 1;

				FaceToIndex[FaceData[i]] = CurrIndex;

				Indices.push_back(CurrIndex);
			}
		
		}

		std::vector<float> VertexBufferToFloat;
		VertexBufferToFloat.resize(VertexBufferData.size() * 8);

		memcpy(VertexBufferToFloat.data(), VertexBufferData.data(), sizeof(SingleVertexInBuffer) * VertexBufferData.size());


		return { VertexBufferToFloat, Indices };
	}
}