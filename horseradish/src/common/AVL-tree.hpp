#pragma once

#include "platform.hpp"

#include <functional>

namespace HorseRadish
{
	template <class TKey, class TValue>
	class AVLTree
	{
		struct Node {
			int height = -1;
			TKey key;
			Node *bMiddle = nullptr, *bLeft = nullptr, *bRight = nullptr, *bRoot = nullptr;

			TValue value;
			bool valueSet = false;

			Node()
			{ }

			Node(Node * const bRoot, int height)
				: height(height), bRoot(bRoot)
			{ }

			bool isBalanced() const
			{
				int l = bLeft ? bLeft->height : -1;
				int r = bRight ? bRight->height : -1;

				return (HorseRadish::Math::iAbs(l - r) < 2);
			}

			bool isLeftHeavier() const
			{
				if (bRight == nullptr)
					return true;
				if (bLeft == nullptr)
					return false;

				return (bLeft->height > bRight->height);
			}

			void updateWeight()
			{
				height = HorseRadish::Math::iMax(bLeft ? bLeft->height : -1, bRight ? bRight->height : -1) + 1;
			}
		};

		Node *mMainTree = nullptr;

		void avlRotateLL(Node * const tree)
		{
			//my parent gets my son
			if (tree->bRoot)
			{
				//should not proceed if match
				if (tree->bRoot->bMiddle == tree)
					return;

				//exchange
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = tree->bLeft;
				else
					tree->bRoot->bRight = tree->bLeft;
			}
			else
			{
				mMainTree = tree->bLeft;
			}

			//dont' forget: the son points to the father also, so we must update it too
			tree->bLeft->bRoot = tree->bRoot;

			//I no longer have a son on my left (which I lost), but I earn his
			auto novoRoot = tree->bLeft;
			tree->bLeft = novoRoot->bRight;
			if (novoRoot->bRight)
				novoRoot->bRight->bRoot = tree;

			//since I'll become the son of my son, I know that I'll have a bigger value,
			//so I'll be on its right (and he's now my father)
			novoRoot->bRight = tree;
			tree->bRoot = novoRoot;

			//recalculate my weight and also my formers son
			tree->updateWeight();
			novoRoot->updateWeight();
		}

		void avlRotateRR(Node * const tree)
		{
			//my parent gets my son
			if (tree->bRoot)
			{
				//should not proceed if match
				if (tree->bRoot->bMiddle == tree)
					return;

				//exchange
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = tree->bRight;
				else
					tree->bRoot->bRight = tree->bRight;
			}
			else
			{
				mMainTree = tree->bRight;
			}

			//dont' forget: the son points to the father also, so we must update it too
			tree->bRight->bRoot = tree->bRoot;

			//I no longer have a son on my left (which I lost), but I earn his
			auto novoRoot = tree->bRight;
			tree->bRight = novoRoot->bLeft;
			if (novoRoot->bLeft)
				novoRoot->bLeft->bRoot = tree;

			//since I'll become the son of my son, I know that I'll have a smaller value,
			//so I'll be on its left (and he's now my father)
			novoRoot->bLeft = tree;
			tree->bRoot = novoRoot;

			//recalculate my weight and also my formers son
			tree->updateWeight();
			novoRoot->updateWeight();
		}

		void avlRotateLR(Node * const tree)
		{
			//check out the new root
			auto novoRoot = tree->bLeft->bRight;
			novoRoot->bRoot = tree->bRoot;

			//my parent gets my son
			if (tree->bRoot)
			{
				//should not proceed if match
				if (tree->bRoot->bMiddle == tree)
					return;

				//exchange
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = novoRoot;
				else
					tree->bRoot->bRight = novoRoot;
			}
			else
			{
				mMainTree = novoRoot;
			}

			//store both sides of the new root
			auto lRoot = novoRoot->bLeft;
			auto rRoot = novoRoot->bRight;

			//adjust roots new sons
			novoRoot->bLeft = tree->bLeft;
			novoRoot->bRight = tree;
			tree->bRoot = novoRoot;
			tree->bLeft->bRoot = novoRoot;

			//adjust roots new grandsons
			novoRoot->bLeft->bRight = lRoot;
			novoRoot->bRight->bLeft = rRoot;
			if (lRoot)	lRoot->bRoot = novoRoot->bLeft;
			if (rRoot)	rRoot->bRoot = novoRoot->bRight;

			//recalculate my weight and also my formers son
			novoRoot->updateWeight();
			novoRoot->bLeft->updateWeight();
			novoRoot->bRight->updateWeight();
		}

		void avlRotateRL(Node * const tree)
		{
			//check out the new root
			auto novoRoot = tree->bRight->bLeft;
			novoRoot->bRoot = tree->bRoot;

			//my parent gets my son
			if (tree->bRoot)
			{
				//should not proceed if match
				if (tree->bRoot->bMiddle == tree)
					return;

				//exchange
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = novoRoot;
				else
					tree->bRoot->bRight = novoRoot;
			}
			else
			{
				mMainTree = novoRoot;
			}

			//store both sides of the new root
			auto lRoot = novoRoot->bLeft;
			auto rRoot = novoRoot->bRight;

			//adjust roots new sons
			novoRoot->bLeft = tree;
			novoRoot->bRight = tree->bRight;
			tree->bRoot = novoRoot;
			tree->bRight->bRoot = novoRoot;

			//adjust roots new grandsons
			novoRoot->bLeft->bRight = lRoot;
			novoRoot->bRight->bLeft = rRoot;
			if (lRoot)	lRoot->bRoot = novoRoot->bLeft;
			if (rRoot)	rRoot->bRoot = novoRoot->bRight;

			//recalculate my weight and also my formers son
			novoRoot->updateWeight();
			novoRoot->bLeft->updateWeight();
			novoRoot->bRight->updateWeight();
		}

		void treeAddData(Node * const node, const char * const string, const TValue &data)
		{
			//we just finished, just store the value
			if (node->key == *string && *(string + 1) == '\0')
			{
				node->value = data;
				node->valueSet = true;
				return;
			}

			//we found the key we are looking for, just go down the middle
			if (node->key == *string)
			{
				if (node->bMiddle == nullptr)
				{
					node->bMiddle = new Node(node, 0);
					node->bMiddle->key = *(string + 1);
				}

				treeAddData(node->bMiddle, string + 1, data);
				return;
			}

			//decide if we should go left or right
			if (*string < node->key)
			{
				if (node->bLeft == nullptr)
				{
					node->bLeft = new Node(node, 0);
					node->bLeft->key = *string;
				}

				treeAddData(node->bLeft, string, data);
			}
			else
			{
				if (node->bRight == nullptr)
				{
					node->bRight = new Node(node, 0);
					node->bRight->key = *string;
				}

				treeAddData(node->bRight, string, data);
			}

			//because we may have added stuff to the left or right of the tree, we should check if the tree is balanced
			/*node->updateWeight();
			if (node->isBalanced())
				return;

			if (node->isLeftHeavier())
			{
				if (node->bLeft->isLeftHeavier())
					avlRotateLL(node);
				else
					avlRotateLR(node);
			}
			else
			{
				if (node->bRight->isLeftHeavier())
					avlRotateRL(node);
				else
					avlRotateRR(node);
			}*/
		}

	public:
		void addData(const char * const string, const TValue &data)
		{
			if (string == nullptr || *string == '\0')
				return;

			if (mMainTree == nullptr)
			{
				mMainTree = new Node(nullptr, 0);
				mMainTree->key = *string;
			}

			treeAddData(mMainTree, string, data);
		}

		bool hasData(const char *what) const
		{
			TValue data;
			return findData(what, data);
		}

		TValue getData(const char * const what, TValue defaultValue = TValue()) const
		{
			TValue data;
			if (findData(what, data))
				return data;

			return defaultValue;
		}

		bool findData(const char * const what, TValue &value) const
		{
			if (what == nullptr || *what == '\0')
				return false;

			std::function<bool(const Node * const, const char * const)> findFunc = [&](const Node * const node, const char * const string)
			{
				if (node == nullptr)
					return false;

				if (node->key == (*string))
				{
					if ((*(string + 1)) == '\0')
					{
						if (!node->valueSet)
							return false;

						value = node->value;
						return true;
					}

					return findFunc(node->bMiddle, string + 1);
				}

				if (node->key < (*string))
					return findFunc(node->bRight, string);
				return findFunc(node->bLeft, string);
			};

			return findFunc(mMainTree, what);
		}

		size_t findAll(const char * const what, const std::function<void(const TValue&)> actionFoundData = nullptr) const
		{
			size_t count;

			std::function<void(const Node * const, const char * const)> findAllFunc = [&](const Node * const node, const char * const string)
			{
				if ((*string) != '\0')
				{
					if (node->key > (*string))
					{
						if (node->bLeft)
							findAllFunc(node->bLeft, string);
						return;
					}
					if (node->key < (*string))
					{
						if (node->bRight)
							findAllFunc(node->bRight, string);
						return;
					}
				}

				if ((*string) == '\0')
				{
					if (node->valueSet)
					{
						count++;
						if (actionFoundData)
							actionFoundData(node->value);
					}

					if (node->bLeft)
						findAllFunc(node->bLeft, string);
					if (node->bRight)
						findAllFunc(node->bRight, string);
					if (node->bMiddle)
						findAllFunc(node->bMiddle, string);
					return;
				}

				if (node->bMiddle)
					findAllFunc(node->bMiddle, string + 1);
			};

			count = 0;
			findAllFunc(mMainTree, what);

			return count;
		}

		size_t findAll(const std::function<void(const TValue&)> actionFoundData = nullptr) const
		{
			size_t count;

			std::function<void(const Node * const)> findAllFunc = [&](const Node * const node)
			{
				if (node->valueSet)
				{
					count++;
					if (actionFoundData)
						actionFoundData(node->value);
				}

				if (node->bLeft)
					findAllFunc(node->bLeft);
				if (node->bMiddle)
					findAllFunc(node->bMiddle);
				if (node->bRight)
					findAllFunc(node->bRight);
			};

			count = 0;
			findAllFunc(mMainTree);

			return count;
		}

		size_t findAllWithKeys(const std::function<void(const std::string&, const TValue&)> actionFoundData = nullptr) const
		{
			size_t count;
			std::string curKey;

			std::function<void(const Node * const)> findAllFunc = [&](const Node * const node)
			{
				if (node->valueSet)
				{
					count++;
					if (actionFoundData)
					{
						curKey += node->key;
						actionFoundData(curKey, node->value);
						curKey.erase(curKey.size() - 1);
					}
				}

				if (node->bLeft)
					findAllFunc(node->bLeft);
				if (node->bMiddle)
				{
					curKey += node->key;
					findAllFunc(node->bMiddle);
					curKey.erase(curKey.size() - 1);
				}
				if (node->bRight)
					findAllFunc(node->bRight);
			};

			count = 0;
			findAllFunc(mMainTree);

			return count;
		}

		void nextBestKeyMatch(const char * const what, char *dest) const
		{
			std::function<void(const Node * const, const char * const)> nextBestKeyMatchFunc = [&](const Node * const node, const char * const string)
			{
				if ((*string) != '\0')
				{
					if (node->key > (*string))
					{
						if (node->bLeft)
							nextBestKeyMatchFunc(node->bLeft, string);
						return;
					}

					if (node->key < (*string))
					{
						if (node->bRight)
							nextBestKeyMatchFunc(node->bRight, string);
						return;
					}
				}

				if ((*string) == '\0')
				{
					if (node->bLeft)
					{
						if (node->bRight || node->bMiddle)
							return;

						nextBestKeyMatchFunc(node->bLeft, string);
						return;
					}

					if (node->bRight)
					{
						if (node->bLeft || node->bMiddle)
							return;

						nextBestKeyMatchFunc(node->bRight, string);
						return;
					}

					if (node->bMiddle)
					{
						if (node->bRight || node->bLeft)
							return;

						*(dest++) = node->key;

						if (!node->valueSet)
							nextBestKeyMatchFunc(node->bMiddle, string);
						return;
					}

					*(dest++) = node->key;
					return;
				}

				*(dest++) = *string;

				if (node->bMiddle)
					nextBestKeyMatchFunc(node->bMiddle, string + 1);
			};

			nextBestKeyMatchFunc(mMainTree, what);
			*dest = '\0';
		}
	};

} //HorseRadish
