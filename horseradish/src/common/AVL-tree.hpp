#pragma once

#include "Platform.hpp"

#include <functional>

namespace HorseRadish
{
	template <class TKey, class TValue>
	class AVLTree
	{
		struct Node {
			int height;
			TKey key;
			Node *bMiddle, *bLeft, *bRight, *bRoot;

			TValue value;
			bool valueSet;

			Node()
				: height(-1), bMiddle(nullptr), bLeft(nullptr), bRight(nullptr), bRoot(nullptr), valueSet(false)
			{ }

			Node(Node * const bRoot, int height)
				: height(height), bMiddle(nullptr), bLeft(nullptr), bRight(nullptr), bRoot(bRoot), valueSet(false)
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
		Node *mainTree;

		void avlRotateLL(Node * const tree)
		{
			//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
			if (tree->bRoot)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->bRoot->bMiddle == tree)
					return;

				//posso mudar
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = tree->bLeft;
				else
					tree->bRoot->bRight = tree->bLeft;
			}
			else
			{
				this->mainTree = tree->bLeft;
			}

			//não esquecer que o filho tem um ponteiro para o pai, logo, tem de actualizar isso tambem
			tree->bLeft->bRoot = tree->bRoot;

			//mas eu deixo de ter um filho à esquerda (que é o meu filho que agora perdi), mas ganho os deles
			auto novoRoot = tree->bLeft;
			tree->bLeft = novoRoot->bRight;
			if (novoRoot->bRight)
				novoRoot->bRight->bRoot = tree;

			//como eu agora vou passar a ser filho do meu filho sei que tenho um valor maior que o dele, logo
			//tenho de ser o seu filho da direita, logo o meu pai também passa a ser ele
			novoRoot->bRight = tree;
			tree->bRoot = novoRoot;

			//recalculo o meu peso e o mesmo para o meu antigo filho
			tree->updateWeight();
			novoRoot->updateWeight();
		}

		void avlRotateRR(Node * const tree)
		{
			//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
			if (tree->bRoot)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->bRoot->bMiddle == tree)
					return;

				//posso mudar
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = tree->bRight;
				else
					tree->bRoot->bRight = tree->bRight;
			}
			else
			{
				this->mainTree = tree->bRight;
			}

			//não esquecer que o filho tem um ponteiro para o pai, logo, tem de actualizar isso tambem
			tree->bRight->bRoot = tree->bRoot;

			//mas eu deixo de ter um filho à esquerda (que é o meu filho que agora perdi), mas ganho os deles
			auto novoRoot = tree->bRight;
			tree->bRight = novoRoot->bLeft;
			if (novoRoot->bLeft)
				novoRoot->bLeft->bRoot = tree;

			//como eu agora vou passar a ser filho do meu filho sei que tenho um valor maior que o dele, logo
			//tenho de ser o seu filho da direita, logo o meu pai também passa a ser ele
			novoRoot->bLeft = tree;
			tree->bRoot = novoRoot;

			//recalculo o meu peso e o mesmo para o meu antigo filho
			tree->updateWeight();
			novoRoot->updateWeight();
		}

		void avlRotateLR(Node * const tree)
		{
			//qual o novo root
			auto novoRoot = tree->bLeft->bRight;
			novoRoot->bRoot = tree->bRoot;

			//o meu pai passa a ter o meu neto (tenho de ver onde estou no meu pai)
			if (tree->bRoot)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->bRoot->bMiddle == tree)
					return;

				//posso mudar
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = novoRoot;
				else
					tree->bRoot->bRight = novoRoot;
			}
			else
			{
				this->mainTree = novoRoot;
			}

			//guardo os lados do novo root
			auto lRoot = novoRoot->bLeft;
			auto rRoot = novoRoot->bRight;

			//os novos filhos do root
			novoRoot->bLeft = tree->bLeft;
			novoRoot->bRight = tree;
			tree->bRoot = novoRoot;
			tree->bLeft->bRoot = novoRoot;

			//arranjos os outros dois
			novoRoot->bLeft->bRight = lRoot;
			novoRoot->bRight->bLeft = rRoot;
			if (lRoot)	lRoot->bRoot = novoRoot->bLeft;
			if (rRoot)	rRoot->bRoot = novoRoot->bRight;

			//recalculo o meu peso e o mesmo para o meu antigo filho
			novoRoot->updateWeight();
			novoRoot->bLeft->updateWeight();
			novoRoot->bRight->updateWeight();
		}

		void avlRotateRL(Node * const tree)
		{
			//qual o novo root
			auto novoRoot = tree->bRight->bLeft;
			novoRoot->bRoot = tree->bRoot;

			//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
			if (tree->bRoot)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->bRoot->bMiddle == tree)
					return;

				//posso mudar
				if (tree->bRoot->bLeft == tree)
					tree->bRoot->bLeft = novoRoot;
				else
					tree->bRoot->bRight = novoRoot;
			}
			else
			{
				this->mainTree = novoRoot;
			}

			//guardo os lados do novo root
			auto lRoot = novoRoot->bLeft;
			auto rRoot = novoRoot->bRight;

			//os novos filhos do root
			novoRoot->bLeft = tree;
			novoRoot->bRight = tree->bRight;
			tree->bRoot = novoRoot;
			tree->bRight->bRoot = novoRoot;

			//arranjos os outros dois
			novoRoot->bLeft->bRight = lRoot;
			novoRoot->bRight->bLeft = rRoot;
			if (lRoot)	lRoot->bRoot = novoRoot->bLeft;
			if (rRoot)	rRoot->bRoot = novoRoot->bRight;

			//recalculo o meu peso e o mesmo para o meu antigo filho
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

			//because we may have added stuff to the left or right of the tree, we should check if the tree is balances
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
		AVLTree()
			: mainTree(nullptr)
		{
		}

		~AVLTree()
		{
			this->mainTree = nullptr;
		}

		void addData(const char * const string, const TValue &data)
		{
			if (string == nullptr || *string == '\0')
				return;

			if (this->mainTree == nullptr)
			{
				this->mainTree = new Node(nullptr, 0);
				this->mainTree->key = *string;
			}

			treeAddData(this->mainTree, string, data);
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

			return findFunc(this->mainTree, what);
		}

		int findAll(const char * const what, const std::function<void(const TValue&)> actionFoundData = nullptr) const
		{
			int count;

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
			findAllFunc(this->mainTree, what);

			return count;
		}

		int findAll(const std::function<void(const TValue&)> actionFoundData = nullptr) const
		{
			int count;

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
			findAllFunc(this->mainTree);

			return count;
		}

		int findAllWithKeys(const std::function<void(const std::string&, const TValue&)> actionFoundData = nullptr) const
		{
			int count;
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
			findAllFunc(this->mainTree);

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

			nextBestKeyMatchFunc(this->mainTree, what);
			*dest = '\0';
		}
	};

} //HorseRadish
