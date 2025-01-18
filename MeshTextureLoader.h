#pragma once
#include <string>
#include <map>
#include <memory>
#include "Mesh.h"
#include "Texture.h"
 
template <typename T> class Type2MeshAndTexture {
public:
	std::string GetName() const {
		return "Type2meshAndTexture"; 
	};

private:
	void log(LogLevel level, std::string text) { LOG(level, GetName() + "::" + text); }

public:
	std::string assetDirectory = "E:/repos/[DuckFishing]/model";

	std::map<T, std::string> type2ObjPath;
	std::map<T, std::unique_ptr<GeneralMesh>> type2Mesh;

	std::map<T, std::string> type2TexturePath;
	std::map<T, std::unique_ptr<Texture>> type2Texture;

	T errorIndex; 

	// Constructor takes the asset directory and mapping
	Type2MeshAndTexture(
		const std::map<T, const std::string>& typeToNameMap
	) {
		log(LOG_INFO, "Initializing Type2MeshAndTexture");  

		errorIndex = typeToNameMap.begin()->first; 
		
		InitObjPaths(typeToNameMap);
		InitTexturePaths(typeToNameMap);

		SetMeshes();
		SetTextures();
	}

	// Constructor takes the asset directory and mapping
	Type2MeshAndTexture(
		const std::string& directory,
		const std::map<T, const std::string>& typeToNameMap
	) : 
		assetDirectory(directory) 
	{
		errorIndex = typeToNameMap.begin()->first;

		InitObjPaths(typeToNameMap);
		InitTexturePaths(typeToNameMap);

		SetMeshes();
		SetTextures();
	}

	void InitObjPaths(const std::map<T, const std::string>& typeToNameMap) {
		log(LOG_INFO, "Initializing path to obj files"); 

		for (const auto& pair : typeToNameMap) {
			const T& type = pair.first;
			const std::string& name = pair.second;

			type2ObjPath[type] = assetDirectory + "/" + name + ".obj";
			log(LOG_INFO, "  " + type2ObjPath[type]);
		}
	}


	void InitTexturePaths(const std::map<T, const std::string>& typeToNameMap) {
		for (const auto& pair : typeToNameMap) {
			const T& type = pair.first;
			const std::string& name = pair.second;

			type2TexturePath[type] = assetDirectory + "/texture/" + name + ".png";
			log(LOG_INFO, "  " + type2TexturePath[type]);
		}
	}


	void SetMeshes() {
		GeneralMesh* currMesh;
		for (const auto& pair : type2ObjPath) {
			const auto& type = pair.first;
			const auto& path = pair.second;

			currMesh = new StaticMesh();
			currMesh->LoadMesh(path);
			type2Mesh[type].reset(currMesh);
		}
	}

	void SetTextures() {
		Texture* currTexture;
		for (const auto& pair : type2TexturePath) {
			const auto& type = pair.first;
			const auto& path = pair.second;

			currTexture = new Texture;
			currTexture->LoadandSetTextureIndexFromPath(path);
			type2Texture[type].reset(currTexture);
		}
	}


	GeneralMesh* GetMesh(const T& type) {
		auto meshIt = type2Mesh.find(type);
		if (meshIt != type2Mesh.end() && meshIt->second) {
			return meshIt->second.get();
		}

		return type2Mesh[errorIndex];
	}

	Texture* GetTexture(const T& type) {
		auto textureIt = type2Texture.find(type);
		if (textureIt != type2Texture.end() && textureIt->second) {
			return textureIt->second.get();
		}

		return type2Texture[errorIndex]; 
	}
	
};