#pragma once

#include "Platform.hpp"

#include <functional>

namespace HorseRadish
{
	template <class T>
	class AVLTree
	{
		struct No{
			int height;
			unsigned int caracter;
			T *dados;
			No *meio, *esquerda, *direita, *pai;
		};
		No *mainTree;

		int avlCalcPeso(No * const tree)
		{
			if (tree == nullptr)
				return -1;
			return (HorseRadish::Math::iMax(tree->esquerda ? tree->esquerda->height : -1, tree->direita ? tree->direita->height : -1) + 1);
		}
		bool avlIsBalanced(No * const tree)
		{
			int l, r;

			l = r = -1;
			if (tree->esquerda)
				l = tree->esquerda->height;
			if (tree->direita)
				r = tree->direita->height;

			return (HorseRadish::Math::iAbs(l - r) < 2);
		}
		void avlRotateLL(No * const tree)
		{
			No *novoRoot;

			//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
			if (tree->pai)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->pai->meio == tree)
					return;

				//posso mudar
				if (tree->pai->esquerda == tree)
					tree->pai->esquerda = tree->esquerda;
				else
					tree->pai->direita = tree->esquerda;
			}
			else
			{
				this->mainTree = tree->esquerda;
			}

			//não esquecer que o filho tem um ponteiro para o pai, logo, tem de actualizar isso tambem
			tree->esquerda->pai = tree->pai;

			//mas eu deixo de ter um filho à esquerda (que é o meu filho que agora perdi), mas ganho os deles
			novoRoot = tree->esquerda;
			tree->esquerda = novoRoot->direita;
			if (novoRoot->direita)
				novoRoot->direita->pai = tree;

			//como eu agora vou passar a ser filho do meu filho sei que tenho um valor maior que o dele, logo
			//tenho de ser o seu filho da direita, logo o meu pai também passa a ser ele
			novoRoot->direita = tree;
			tree->pai = novoRoot;

			//recalculo o meu peso e o mesmo para o meu antigo filho
			tree->height = avlCalcPeso(tree);
			novoRoot->height = avlCalcPeso(novoRoot);
		}
		void avlRotateRR(No * const tree)
		{
			No *novoRoot;

			//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
			if (tree->pai)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->pai->meio == tree)
					return;

				//posso mudar
				if (tree->pai->esquerda == tree)
					tree->pai->esquerda = tree->direita;
				else
					tree->pai->direita = tree->direita;
			}
			else
			{
				this->mainTree = tree->direita;
			}

			//não esquecer que o filho tem um ponteiro para o pai, logo, tem de actualizar isso tambem
			tree->direita->pai = tree->pai;

			//mas eu deixo de ter um filho à esquerda (que é o meu filho que agora perdi), mas ganho os deles
			novoRoot = tree->direita;
			tree->direita = novoRoot->esquerda;
			if (novoRoot->esquerda)
				novoRoot->esquerda->pai = tree;

			//como eu agora vou passar a ser filho do meu filho sei que tenho um valor maior que o dele, logo
			//tenho de ser o seu filho da direita, logo o meu pai também passa a ser ele
			novoRoot->esquerda = tree;
			tree->pai = novoRoot;

			//recalculo o meu peso e o mesmo para o meu antigo filho
			tree->height = avlCalcPeso(tree);
			novoRoot->height = avlCalcPeso(novoRoot);
		}
		void avlRotateLR(No * const tree)
		{
			No *novoRoot, *lRoot, *rRoot;

			//qual o novo root
			novoRoot = tree->esquerda->direita;
			novoRoot->pai = tree->pai;

			//o meu pai passa a ter o meu neto (tenho de ver onde estou no meu pai)
			if (tree->pai)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->pai->meio == tree)
					return;

				//posso mudar
				if (tree->pai->esquerda == tree)
					tree->pai->esquerda = novoRoot;
				else
					tree->pai->direita = novoRoot;
			}
			else
			{
				this->mainTree = novoRoot;
			}

			//guardo os lados do novo root
			lRoot = novoRoot->esquerda;
			rRoot = novoRoot->direita;

			//os novos filhos do root
			novoRoot->esquerda = tree->esquerda;
			novoRoot->direita = tree;
			tree->pai = novoRoot;
			tree->esquerda->pai = novoRoot;

			//arranjos os outros dois
			novoRoot->esquerda->direita = lRoot;
			novoRoot->direita->esquerda = rRoot;
			if (lRoot)	lRoot->pai = novoRoot->esquerda;
			if (rRoot)	rRoot->pai = novoRoot->direita;

			//recalculo o meu peso e o mesmo para o meu antigo filho
			novoRoot->height = avlCalcPeso(novoRoot);
			novoRoot->esquerda->height = avlCalcPeso(novoRoot->esquerda);
			novoRoot->direita->height = avlCalcPeso(novoRoot->direita);
		}
		void avlRotateRL(No * const tree)
		{
			No *novoRoot, *lRoot, *rRoot;

			//qual o novo root
			novoRoot = tree->direita->esquerda;
			novoRoot->pai = tree->pai;

			//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
			if (tree->pai)
			{
				//isto não pode acontecer se eu vim de um meio (quebro a string se muda-se de sitio)
				if (tree->pai->meio == tree)
					return;

				//posso mudar
				if (tree->pai->esquerda == tree)
					tree->pai->esquerda = novoRoot;
				else
					tree->pai->direita = novoRoot;
			}
			else
			{
				this->mainTree = novoRoot;
			}

			//guardo os lados do novo root
			lRoot = novoRoot->esquerda;
			rRoot = novoRoot->direita;

			//os novos filhos do root
			novoRoot->esquerda = tree;
			novoRoot->direita = tree->direita;
			tree->pai = novoRoot;
			tree->direita->pai = novoRoot;

			//arranjos os outros dois
			novoRoot->esquerda->direita = lRoot;
			novoRoot->direita->esquerda = rRoot;
			if (lRoot)	lRoot->pai = novoRoot->esquerda;
			if (rRoot)	rRoot->pai = novoRoot->direita;

			//recalculo o meu peso e o mesmo para o meu antigo filho
			novoRoot->height = avlCalcPeso(novoRoot);
			novoRoot->esquerda->height = avlCalcPeso(novoRoot->esquerda);
			novoRoot->direita->height = avlCalcPeso(novoRoot->direita);
		}
		bool avlLeftHeavier(const No * const tree)
		{
			//assumo que tem de haver pelo menos um filho e por isso nao verifico se right==left==nullptr
			if (tree->direita == nullptr)
				return true;
			if (tree->esquerda == nullptr)
				return false;

			return (tree->esquerda->height > tree->direita->height);
		}

		void treeAdd(const char * const string, No * const tree, T* const noData)
		{
			//se tiver no proprio caracter e acabei a string, só tenho de escrever os dados
			if (tree->caracter == *string && *(string + 1) == '\0')
			{
				//guardo o ponteiro e já está
				tree->dados = noData;
				return;
			}

			//se tiver no proprio caracter e estou aqui é porque não acabei, logo crio no meio e sigo
			if (tree->caracter == *string)
			{
				//se ainda não tenho nada
				if (tree->meio == nullptr)
				{
					tree->meio = new No();
					tree->meio->height = 0;
					tree->meio->pai = tree;
					tree->meio->caracter = *(string + 1);
					tree->meio->dados = nullptr;
					tree->meio->direita = tree->meio->esquerda = tree->meio->meio = nullptr;
				}

				//senão, desco pelo meio
				treeAdd(string + 1, tree->meio, noData);
				return;
			}

			//se o valor a guardar for mais pequeno
			if (*string < tree->caracter)
			{
				//se não tiver lá nada, crio o filho
				if (tree->esquerda == nullptr)
				{
					//crio o novo filho e preencho os campos
					tree->esquerda = new No();
					tree->esquerda->height = 0;
					tree->esquerda->direita = tree->esquerda->esquerda = tree->esquerda->meio = nullptr;
					tree->esquerda->caracter = *string;
					tree->esquerda->pai = tree;
					tree->esquerda->dados = nullptr;
				}

				//senão, desco pelo meu filho
				treeAdd(string, tree->esquerda, noData);
			}
			//se o valor não é igual nem menor de onde estou, tem de ser maior
			else
			{
				//se não tiver lá nada, crio o filho
				if (tree->direita == nullptr)
				{
					//crio o novo filho e preencho os campos
					tree->direita = new No();
					tree->direita->height = 0;
					tree->direita->direita = tree->direita->esquerda = tree->direita->meio = nullptr;
					tree->direita->caracter = *string;
					tree->direita->pai = tree;
					tree->direita->dados = nullptr;
				}

				//senão, desco pelo meu filho
				treeAdd(string, tree->direita, noData);
			}

			//chegando aqui já tá inserido, mas pode não estar balanceado
			//calculo o novo peso e vejo se tenho de fazer alguma coisa
			tree->height = avlCalcPeso(tree);
			if (avlIsBalanced(tree) == true)
				return;

			//chegando aqui tenho mesmo de balancear a árvore

			//para os casos LL e LR
			/*if (avlLeftHeavier(tree))
			{
			if (avlLeftHeavier(tree->esquerda))
			avlRotateLL(tree);
			else
			avlRotateLR(tree);
			}
			//para os casos RR e RL
			else
			{
			if (avlLeftHeavier(tree->direita))
			avlRotateRL(tree);
			else
			avlRotateRR(tree);
			}*/

			//e já tá
			return;
		}

		T* treeFindCount(const int countTarget, int countCurrent, const No * const arvore)
		{
			if (arvore == nullptr)
				return nullptr;

			if (arvore->dados != nullptr)
			{
				countCurrent++;
				if (countCurrent == countTarget)
					return arvore->dados;
			}

			T *res;

			res = treeFindCount(countTarget, countCurrent, arvore->esquerda);
			if (res != nullptr)
				return res;

			res = treeFindCount(countTarget, countCurrent, arvore->meio);
			if (res != nullptr)
				return res;

			res = treeFindCount(countTarget, countCurrent, arvore->direita);
			if (res != nullptr)
				return res;

			return nullptr;
		}
		T* treeFind(const char * const string, const No * const arvore)
		{
			if (arvore == nullptr || string == nullptr)
				return nullptr;

			//achou este caracter
			if (arvore->caracter == (*string))
			{
				if ((*(string + 1)) == '\0')
					return arvore->dados;
				return treeFind(string + 1, arvore->meio);
			}

			//vai para onde deve
			if (arvore->caracter < (*string))
				return treeFind(string, arvore->direita);
			return treeFind(string, arvore->esquerda);
		}
		void treeDeleteData(No * const arvore)
		{
			//se não tenho nada
			if (arvore == nullptr)
				return;

			//se tiver alguma a apagar
			if (arvore->dados)
			{
				delete arvore->dados;
				arvore->dados = nullptr;
			}

			//faço o mesmo para os outros nós
			treeDeleteData(arvore->direita);
			treeDeleteData(arvore->meio);
			treeDeleteData(arvore->esquerda);
		}
		void treeHitCount(const No * const arvore, const char * const string, int &count, std::function<void(T*)> actionDataHit)
		{
			if ((*string) != '\0')
			{
				if (arvore->caracter > (*string))
				{
					if (arvore->esquerda)
						treeHitCount(arvore->esquerda, string, count, actionDataHit);
					return;
				}
				if (arvore->caracter < (*string))
				{
					if (arvore->direita)
						treeHitCount(arvore->direita, string, count, actionDataHit);
					return;
				}
			}

			if ((*string) == '\0')
			{
				if (arvore->dados)
				{
					count++;
					if (actionDataHit != nullptr)
						actionDataHit(arvore->dados);
				}

				if (arvore->esquerda)
					treeHitCount(arvore->esquerda, string, count, actionDataHit);
				if (arvore->direita)
					treeHitCount(arvore->direita, string, count, actionDataHit);
				if (arvore->meio)
					treeHitCount(arvore->meio, string, count, actionDataHit);
				return;
			}

			if (arvore->meio)
				treeHitCount(arvore->meio, string + 1, count, actionDataHit);
		}
		void treeFinish(const No * const arvore, const char * const string, char *dest)
		{
			if ((*string) != '\0')
			{
				if (arvore->caracter > (*string))
				{
					if (arvore->esquerda)
						treeFinish(arvore->esquerda, string, dest);
					return;
				}
				if (arvore->caracter < (*string))
				{
					if (arvore->direita)
						treeFinish(arvore->direita, string, dest);
					return;
				}
			}

			if ((*string) == '\0')
			{
				if (arvore->esquerda)
				{
					if (arvore->direita || arvore->meio)
						return;
					treeFinish(arvore->esquerda, string, dest);
					return;
				}
				if (arvore->direita)
				{
					if (arvore->esquerda || arvore->meio)
						return;
					treeFinish(arvore->direita, string, dest);
					return;
				}
				if (arvore->meio)
				{
					if (arvore->direita || arvore->esquerda)
						return;
					*(dest++) = arvore->caracter;
					*dest = '\0';
					if (arvore->dados == nullptr)
						treeFinish(arvore->meio, string, dest);
					return;
				}

				*(dest++) = arvore->caracter;
				*(dest++) = ' ';
				*dest = '\0';
				return;
			}

			*(dest++) = *string;
			*dest = '\0';
			if (arvore->meio)
				treeFinish(arvore->meio, string + 1, dest);
		}
		void forEachData(const No * const arvore, std::function<void(T*)> funcCallback, int &numHits)
		{
			if (arvore->dados != nullptr)
			{
				numHits++;

				if (funcCallback != nullptr)
					funcCallback(arvore->dados);
			}

			if (arvore->esquerda)
				forEachData(arvore->esquerda, funcCallback, numHits);
			if (arvore->meio)
				forEachData(arvore->meio, funcCallback, numHits);
			if (arvore->direita)
				forEachData(arvore->direita, funcCallback, numHits);
		}
		void finishTABComplete(const No * const arvore, const char * const string, char * const bufferOut)
		{
			static int pos;
			static bool acabou;

			if (string == nullptr || string[0] == '\0')
				return;

			//pro primeiro caso
			if (arvore == this->mainTree)
			{
				bufferOut[0] = '\0';
				pos = 0;
				acabou = false;
			}

			if (arvore->dados != nullptr && acabou)
			{
				bufferOut[pos] = arvore->caracter;
				bufferOut[pos + 1] = '\0';
				//this->LogTabColor(bufferOut,2,140,140,140);
			}

			if (acabou && string[pos] > arvore->caracter && arvore->esquerda)
				finishTABComplete(arvore->esquerda, string, bufferOut);
			else if (arvore->esquerda)
				finishTABComplete(arvore->esquerda, string, bufferOut);

			if (arvore->meio)
			{
				if (!acabou)
				{
					if (string[pos] == arvore->caracter)
					{
						bufferOut[pos] = arvore->caracter;
						bufferOut[pos + 1] = '\0';
						pos++;
						if (string[pos] == '\0')
						{
							acabou = true;
							//if (arvore->dados)
							//this->LogTab(bufferOut,2);
						}
						finishTABComplete(arvore->meio, string, bufferOut);
						pos--;
						acabou = false;
					}
				}
				else
				{
					bufferOut[pos] = arvore->caracter;
					bufferOut[pos + 1] = '\0';
					pos++;
					finishTABComplete(arvore->meio, string, bufferOut);
					pos--;
				}
			}

			if (acabou && string[pos] < arvore->caracter && arvore->direita)
				finishTABComplete(arvore->direita, string, bufferOut);
			else if (arvore->direita)
				finishTABComplete(arvore->direita, string, bufferOut);
		}

	public:
		AVLTree()
		{
			//por omissão
			this->mainTree = nullptr;
		}
		~AVLTree()
		{
			//limpo tudo
			this->mainTree = nullptr;
		}

		T* FindData(const char *what)
		{
			return this->treeFind(what, this->mainTree);
		}
		T* FindCount(const int countTarget, int countCurrent)
		{
			return this->treeFindCount(countTarget, countCurrent, this->mainTree);
		}
		void HitCount(const char * const string, int &count, std::function<void(T*)> actionDataHit)
		{
			this->treeHitCount(this->mainTree, string, count, actionDataHit);
		}
		void Finish(const char * const string, char *dest)
		{
			this->treeFinish(this->mainTree, string, dest);
		}
		void Add(const char * const string, T * const data)
		{
			//verificar isto
			if ((string == nullptr) || (data == nullptr))
				return;

			//se a àrvore está vazia, tenho de a criar
			if (this->mainTree == nullptr)
			{
				//crio o primeiro nó
				this->mainTree = new No();
				this->mainTree->height = 0;
				this->mainTree->pai = nullptr;
				this->mainTree->caracter = *string;
				this->mainTree->dados = nullptr;
				this->mainTree->direita = this->mainTree->esquerda = this->mainTree->meio = nullptr;
			}

			//avanço pela raíz da árvore
			treeAdd(string, this->mainTree, data);
		}
		void DeleteData()
		{
			treeDeleteData(this->mainTree);
		}
		int ForEachData(std::function<void(T*)> funcCallback)
		{
			int numHits;

			//passo por toda a árvore
			numHits = 0;
			forEachData(this->mainTree, funcCallback, numHits);

			//posso devolver o número de hits
			return numHits;
		}
	};

} //HorseRadish
