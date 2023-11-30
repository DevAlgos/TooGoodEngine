#pragma once

#include <vector>
#include <ECS/Entity.h>
#include <cassert>

struct Cell
{
	Cell() = default;
	~Cell() = default;

	inline Cell(const Cell& other)
	{
		EntitiesInCell = other.EntitiesInCell;
	}
	
	inline void AddEntityToCell(Ecs::Entity CurrentEntity)
	{
		auto iterator = std::find(EntitiesInCell.begin(), EntitiesInCell.end(), CurrentEntity);
		if (iterator == EntitiesInCell.end())
			EntitiesInCell.push_back(CurrentEntity);
	}

	inline void RemoveEntityFromCell(Ecs::Entity CurrentEntity)
	{
		auto iterator = std::find(EntitiesInCell.begin(), EntitiesInCell.end(), CurrentEntity);
		if (iterator != EntitiesInCell.end())
			EntitiesInCell.erase(iterator);
	}

	std::vector<Ecs::Entity> EntitiesInCell;
};

struct CollisionMap
{
	constexpr CollisionMap() : MapWidth(50),MapHeight(50)
	{
		for (int x = 0; x < MapWidth * MapHeight; x++)
			Map.emplace_back();
	}
	
	CollisionMap(int MapWidth, int MapHeight) : MapWidth(MapWidth), MapHeight(MapHeight)
	{
		for (int x = 0; x < MapWidth * MapHeight; x++) 
			Map.emplace_back();
	}

	void AddEntity(int x, int y, Ecs::Entity Current)
	{
		int adjustedX = x + MapWidth / 2; 
		int adjustedY = y + MapHeight / 2; 
		Map[adjustedX + adjustedY * MapWidth].AddEntityToCell(Current);
	}

	void RemoveEntity(int x, int y, Ecs::Entity Current)
	{
		int adjustedX = x + MapWidth / 2;
		int adjustedY = y + MapHeight / 2;
		Map[adjustedX + adjustedY * MapWidth].RemoveEntityFromCell(Current);
	}

	Cell& GetCell(int x, int y)
	{
		return Map[x + y * MapWidth];
	}

	void Clear()
	{
		Map.clear();
		Map.resize(MapWidth * MapHeight);
	}
	
	~CollisionMap() = default;

private:
	int MapWidth = 0;
	int MapHeight = 0;

	std::vector<Cell> Map;
};
