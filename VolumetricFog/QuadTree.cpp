#include "QuadTree.h"

//template<typename T>
//void QuadTree<T>::AddToNode(const T* elementAdress, const DirectX::BoundingBox& boundingBox, std::unique_ptr<Node>& node)
//{
//	Node currentNode = node.get();
//	bool collision = currentNode.boundingBox.Intersects(boundingBox);
//
//	// Check if the object is inside the volume box
//	if (!collision) // No collision found, node and potential children not relevant
//	{
//		return;
//	}
//
//	if (this->IsLeafNode(currentNode)) // Check if current node is a leaf node
//	{
//		if (currentNode.totalChildren != MAX_NODES) // Check if the node is not full
//		{
//			for (int i = 0; i < MAX_NODES; i++)
//			{
//				if (currentNode.children[i] != nullptr)
//				{
//					// Add the element to the node
//					Node childNode = currentNode.children[i].get();
//					childNode.boundingBox = boundingBox;
//					childNode.element = elementAdress;
//
//					currentNode.element[i] = elementAdress;
//					currentNode.elementBoundingBoxes = boundingBox;
//					currentNode.totalChildren++;
//					break;
//				}
//			}
//		}
//		else // The current node is full
//		{
//			// Add child nodes to the node based on this node's covered volume
//			DirectX::BoundingBox slicedBoxes[MAX_NODES];
//			this->SliceWorldBoxes(currentNode.boundingBox, slicedBoxes);
//
//			for (int i = 0; i < MAX_NODES; i++)
//			{
//				Node childNode = currentNode.children[i].get();
//				DirectX::BoundingBox currentObjectBox = childNode.boundingBox; // The bounding box of the object to be replaced
//				childNode.boundingBox = slicedBoxes[i];
//
//				// For each of the currently stored elements in this node, attempt to add them to the new child nodes
//				for (int j = 0; j < MAX_NODES; j++)
//				{
//					this->AddToNode(childNode.element, currentObjectBox, currentNode.children[j].get());
//				}
//			}
//		}
//
//		return;
//	}
//
//	for (unsigned int i = 0; i < 4; i++)
//	{
//		this->AddToNode(elementAdress, boundingBox, currentNode.children[i]);
//	}
//}

//template<typename T>
//bool QuadTree<T>::IsLeafNode(Node* currentNode)
//{
//	Node* node = currentNode.get();
//	return node->children[0] == nullptr && node->children[1] == nullptr && node->children[2] == nullptr && node->children[3] == nullptr;
//}
//
//template<typename T>
//bool QuadTree<T>::ChildIsEmpty(std::unique_ptr<Node>* child)
//{
//	if (child == nullptr)
//		return true;
//	return false;
//}

//template<typename T>
//void QuadTree<T>::SliceWorldBoxes(DirectX::BoundingBox& currentBox, DirectX::BoundingBox* slicedBoxes)
//{
//	DirectX::XMFLOAT3 half = { currentBox.Extents.x / 2.0f, currentBox.Extents.y, currentBox.Extents.z / 2.0f };
//
//	/*
//		  Z
//		1 | 2
//		--Y--X
//		4 | 3
//	*/
//
//	DirectX::BoundingBox sliced1(DirectX::XMFLOAT3(currentBox.Center.x - half.x, currentBox.Center.y, currentBox.Center.z + half.z), half);
//	DirectX::BoundingBox sliced2(DirectX::XMFLOAT3(currentBox.Center.x + half.x, currentBox.Center.y, currentBox.Center.z + half.z), half);
//	DirectX::BoundingBox sliced3(DirectX::XMFLOAT3(currentBox.Center.x + half.x, currentBox.Center.y, currentBox.Center.z - half.z), half);
//	DirectX::BoundingBox sliced4(DirectX::XMFLOAT3(currentBox.Center.x - half.x, currentBox.Center.y, currentBox.Center.z - half.z), half);
//
//	slicedBoxes[0] = sliced1;
//	slicedBoxes[1] = sliced2;
//	slicedBoxes[2] = sliced3;
//	slicedBoxes[3] = sliced4;
//}

//template<typename T>
//void QuadTree<T>::Initialize(DirectX::XMFLOAT3 center, DirectX::XMFLOAT3 extents)
//{
//	DirectX::BoundingBox rootBox(center, extents);
//
//	Node* root = this->root.get();
//	root->boundingBox = rootBox;
//}

//template<typename T>
//void QuadTree<T>::AddElement(const T* elementAdress, const DirectX::BoundingBox& boundingBox)
//{
//	Node root = this->root.get();
//	root.currentBoundingBox = boundingBox;
//
//	this->AddToNode(elementAdress, boundingBox, this->root);
//}
