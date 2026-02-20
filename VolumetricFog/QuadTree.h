#pragma once

#include <iostream>
#include <memory>
#include <algorithm>
#include <DirectXCollision.h>

#include "MeshD3D11.h"

#define MAX_NODES 4

template<typename T>
class QuadTree
{
private:
	struct Node
	{
		bool isLeafNode = true;
		DirectX::BoundingBox boundingBox;

		T* element = nullptr;
		unsigned int totalChildren = 0;
		std::unique_ptr<Node> children[MAX_NODES];
	};

	std::unique_ptr<Node> root;

	void AddToNode(const T* elementAdress,
		const DirectX::BoundingBox& boundingBox, std::unique_ptr<Node>& node);
	void CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node, std::vector<const T*>& foundObjects);

	// Slices the current world bounding box into MAX_NODES equal slices
	void SliceWorldBoxes(DirectX::BoundingBox& currentBox, DirectX::BoundingBox* slicedBoxes);

	void GenerateGraphvizLinks(std::string& data, std::unique_ptr<Node>& node, size_t& nodeCounter);
	std::string BoundingBoxToGraphviz(const DirectX::BoundingBox& boundingBox, const std::string& nodeName,
		const std::string& label, const std::string& color);

	size_t GenerateGraphvizLinksTree(std::string& data, std::unique_ptr<Node>& node, size_t& nodeCounter);

public:
	QuadTree() = default;
	~QuadTree() = default;

	void Initialize(DirectX::XMFLOAT3 center, DirectX::XMFLOAT3 extents);

	void AddElement(const T* elementAdress,
		const DirectX::BoundingBox& boundingBox);
	std::vector<const T*> CheckTree(const DirectX::BoundingFrustum& frusum);

	std::string ToGraphviz();
	std::string ToGraphvizTree();
};

template<typename T>
void QuadTree<T>::AddToNode(const T* elementAdress, const DirectX::BoundingBox& boundingBox, std::unique_ptr<Node>& node)
{
	Node* currentNode = node.get();
	bool collision = currentNode->boundingBox.Intersects(boundingBox);

	// Check if the object is inside the volume box
	if (!collision) // No collision found, node and potential children not relevant
	{
		return;
	}

	if (currentNode->isLeafNode) // Check if current node is a leaf node
	{
		if (currentNode->totalChildren != MAX_NODES) // Check if the node is not full
		{
			for (int i = 0; i < MAX_NODES; i++)
			{
				if (currentNode->children[i] == nullptr)
				{
					// Add the element to the node
					currentNode->children[i] = std::make_unique<Node>();
					Node* childNode = currentNode->children[i].get();
					childNode->boundingBox = boundingBox;
					childNode->element = (T*)elementAdress;

					currentNode->totalChildren++;
					return;
				}
			}
		}
		else // The current node is full
		{
			// Add child nodes to the node based on this node's covered volume
			// Get the smaller volume boxes
			DirectX::BoundingBox slicedBoxes[MAX_NODES];
			this->SliceWorldBoxes(currentNode->boundingBox, slicedBoxes);

			DirectX::BoundingBox objectBoxes[MAX_NODES];

			for (int i = 0; i < MAX_NODES; i++)
			{
				Node* childNode = currentNode->children[i].get();
				objectBoxes[i] = childNode->boundingBox; // Save all object boxes
				childNode->boundingBox = slicedBoxes[i]; // Replace the object boxes with the new volume boxes
			}

			// For each of the currently stored elements in this node, attempt to add them to the new child nodes
			for (int i = 0; i < MAX_NODES; i++)
			{
				Node* childNode = currentNode->children[i].get();
				for (int j = 0; j < MAX_NODES; j++)
				{
					this->AddToNode(childNode->element, objectBoxes[i], currentNode->children[j]);
				}
				childNode->element = nullptr;
			}

			currentNode->isLeafNode = false;
		}
	}

	// The parent node either was a parent node all along, or it was a full leaf node and turned into a parent node
	// For each of the child nodes of this node, recursively call this function with the same element and volume that 
	// were receivied by this function call
	for (unsigned int i = 0; i < 4; i++)
	{
		this->AddToNode(elementAdress, boundingBox, currentNode->children[i]);
	}
}

template<typename T>
void QuadTree<T>::CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node, std::vector<const T*>& foundObjects)
{
	Node* currentNode = node.get();
	bool collision = currentNode->boundingBox.Intersects(frustum);

	if (!collision)
	{
		return;
	}

	if (currentNode->isLeafNode) // Check if current node is a leaf node
	{
		for (int i = 0; i < currentNode->totalChildren; i++)
		{
			Node* childNode = currentNode->children[i].get();
			collision = childNode->boundingBox.Intersects(frustum);

			if (collision)
			{
				// Check if the object is already present in the return vector, add if not
				bool found = false;
				for (int j = 0; j < foundObjects.size(); j++)
				{
					if (foundObjects[j] == childNode->element)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					foundObjects.push_back(childNode->element);
				}
			}
		}
	}
	else
	{
		// Recursively run this function for each of the child nodes of this node
		for (int i = 0; i < MAX_NODES; i++)
		{
			this->CheckNode(frustum, currentNode->children[i], foundObjects);
		}
	}
}

template<typename T>
void QuadTree<T>::SliceWorldBoxes(DirectX::BoundingBox& currentBox, DirectX::BoundingBox* slicedBoxes)
{
	DirectX::XMFLOAT3 half = { currentBox.Extents.x / 2.0f, currentBox.Extents.y, currentBox.Extents.z / 2.0f };

	/*
		  Z
		1 | 2
		--Y--X
		4 | 3
	*/

	DirectX::BoundingBox sliced1(DirectX::XMFLOAT3(currentBox.Center.x - half.x, currentBox.Center.y, currentBox.Center.z + half.z), half);
	DirectX::BoundingBox sliced2(DirectX::XMFLOAT3(currentBox.Center.x + half.x, currentBox.Center.y, currentBox.Center.z + half.z), half);
	DirectX::BoundingBox sliced3(DirectX::XMFLOAT3(currentBox.Center.x + half.x, currentBox.Center.y, currentBox.Center.z - half.z), half);
	DirectX::BoundingBox sliced4(DirectX::XMFLOAT3(currentBox.Center.x - half.x, currentBox.Center.y, currentBox.Center.z - half.z), half);

	slicedBoxes[0] = sliced1;
	slicedBoxes[1] = sliced2;
	slicedBoxes[2] = sliced3;
	slicedBoxes[3] = sliced4;
}

template < typename T>
void QuadTree<T>::GenerateGraphvizLinks(std::string & data, std::unique_ptr<Node>&node, size_t & nodeCounter)
{
	Node* currentNode = node.get();
	std::string nodeName = "Node_" + std::to_string(nodeCounter);
	data += BoundingBoxToGraphviz(currentNode->boundingBox, nodeName, "", "black");

	for (size_t i = 0; i < currentNode->totalChildren; ++i)
	{
		std::string elementName = nodeName + "_Element_" + std::to_string(i);
		std::string elementLabel = ""; // std::to_string(*currentNode->children[i].elementAddress); // Only possible if the type is "to_stringable",
		// which many custom types are not.Either provide an overload of to_string for your type, or change this line
		Node* childNode = currentNode->children[i].get();
		if (childNode->element != nullptr)
		{
			elementLabel = childNode->element->GetName();
		}
		else
		{
			//elementLabel = "Node_" + std::to_string(nodeCounter);
		}
		data += BoundingBoxToGraphviz(childNode->boundingBox, elementName, elementLabel, "blue");
	}

	if (!currentNode->isLeafNode)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			GenerateGraphvizLinks(data, currentNode->children[i], ++nodeCounter);
		}
	}
}

template<typename T>
inline std::string QuadTree<T>::BoundingBoxToGraphviz(const DirectX::BoundingBox& boundingBox, const std::string& nodeName, 
	const std::string& label, const std::string& color)
{
	float width = boundingBox.Extents.x * 2 / 10;
	float height = boundingBox.Extents.z * 2 / 10;
	float centerX = boundingBox.Center.x / 10;
	float centerY = boundingBox.Center.z / 10;

	std::string toReturn = nodeName + " [label=\"" + label + "\", color=\"" + color + "\", fixedsize=true, shape=box, width="
		+ std::to_string(width) + ", height=" + std::to_string(height) +
		", pos=\"" + std::to_string(centerX) + ',' + std::to_string(centerY) + "!\"]\n";

	return toReturn;
}

template < typename T>
size_t QuadTree<T>::GenerateGraphvizLinksTree(std::string & data, std::unique_ptr<Node>&node, size_t & nodeCounter)
{
	size_t myID = nodeCounter;
	Node* currentNode = node.get();

	if (currentNode == nullptr)
	{
		return myID;
	}

	std::string meshName;
	if (currentNode->element != nullptr)
	{
		meshName = currentNode->element->GetName();
	}
	else
	{
		meshName = "Node: " + std::to_string(myID);
	}

	data += std::to_string(myID) + "[label = \"" + meshName;
	// Here you can add additional information about the node as desired
	data += "\"]\n";

	//if (!currentNode->isLeafNode)
	if (currentNode->element == nullptr)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			std::string childID = std::to_string(GenerateGraphvizLinksTree(data, currentNode->children[i], ++nodeCounter));
			data += std::to_string(myID) + " -> " + childID + '\n';
		}
	}

	return myID;
}


template<typename T>
void QuadTree<T>::Initialize(DirectX::XMFLOAT3 center, DirectX::XMFLOAT3 extents)
{
	this->root = std::make_unique<Node>();
	DirectX::BoundingBox rootBox(center, extents);

	Node* root = this->root.get();
	root->boundingBox = rootBox;
}

template<typename T>
void QuadTree<T>::AddElement(const T* elementAdress, const DirectX::BoundingBox& boundingBox)
{
	Node* root = this->root.get();

	this->AddToNode(elementAdress, boundingBox, this->root);
}

template<typename T>
std::vector<const T*> QuadTree<T>::CheckTree(const DirectX::BoundingFrustum& frusum)
{
	std::vector<const T*> toReturn;

	this->CheckNode(frusum, this->root, toReturn);

	return toReturn;
}

template < typename T>
std::string QuadTree<T>::ToGraphviz() // Public function that drives the recursive chain
{
	std::string toReturn = "digraph D{\n";

	size_t counter = 0;
	GenerateGraphvizLinks(toReturn, this->root, counter);

	return toReturn + '}';
}

template<typename T>
std::string QuadTree<T>::ToGraphvizTree()
{
	std::string toReturn = "digraph D{\n";

	size_t counter = 0;
	GenerateGraphvizLinksTree(toReturn, this->root, counter);
	
	return toReturn + '}';
}
