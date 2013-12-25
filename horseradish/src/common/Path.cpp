#include "Path.hpp"
#include "Platform.hpp"

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Funções auxiliares locais		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

namespace HorseRadish
{
namespace IO
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ A classe principal que implementa o sistema de ficheiros		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
void Path::validadePath()
{
	//se for preciso mudar '/' para '\\'
	if (HorseRadish::Platform::DirectorySeparatorChar != '/')
		HorseRadish::String::Replace('/', HorseRadish::Platform::DirectorySeparatorChar);

	//removo sempre duplicados
	HorseRadish::String::RemoveDoubles(HorseRadish::Platform::DirectorySeparatorChar);

	//removo sempre espaços a mais
	HorseRadish::String::Trim(' ');
}

Path::Path()
{
	//basta limpar isto
	HorseRadish::String::SetEmpty();
}

Path::Path(const HorseRadish::hChar * const path)
{
	//basta chamar esta função
	this->Set(path);
}

Path::Path(const char * const path)
{
	//basta chamar esta função
	this->Set((const HorseRadish::hChar *)path);
}

Path::Path(const HorseRadish::String &path)
{
	//basta chamar esta função
	this->Set((const HorseRadish::hChar *)path.GetData());
}

Path::Path(const KnownPath &knownPath)
{
	//basta chamar esta função
	this->Set(knownPath);
}

Path::~Path()
{
	//basta limpar isto
	HorseRadish::String::SetEmpty();
}

void Path::Clear()
{
	//basta limpar isto
	HorseRadish::String::SetEmpty();
}

void Path::Set(const HorseRadish::hChar * const path)
{
	//começo por limpar qualquer caminho anterior
	HorseRadish::String::SetEmpty();

	//se tiver alguma coisa, guardo-a
	if ((path != nullptr) && (*path != '\0'))
		HorseRadish::String::Set(path);

	//operações de manutenção
	validadePath();
}

void Path::Set(const HorseRadish::String &path)
{
	//guardo o caminho indicado
	HorseRadish::String::Set(path);

	//operações de manutenção
	validadePath();
}

void Path::Set(const Path &path)
{
	//basta guardar o caminho indicado
	HorseRadish::String::Set(path);
}

void Path::Set(const Path &path1, const Path &path2)
{
	//fico com o primeiro e adiciono simplesmente o último
	HorseRadish::String::Set(path1);
	this->Combine(path2);
}

void Path::Set(const Path::KnownPath &knownPath)
{
	//se for para a pasta do sistema
	if (knownPath == Path::SystemFolder)
	{
		//basta ler daqui e pronto
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemFolder, *this);
		return;
	}

	//se for para a pasta actual
	if (knownPath == Path::CurrentFolder)
	{
		//basta ler daqui e pronto
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::CurrentFolder, *this);
		return;
	}

	//chegando aqui, temos problema
	HorseRadish::String::SetEmpty();
}

void Path::Combine(const Path &pathToAppend)
{
	//se não tenho nada
	if (pathToAppend.IsEmpty() == true)
		return;

	//se não tenho nada actualmente
	if (HorseRadish::String::IsEmpty() == true)
	{
		//fico com o path enviado e pronto
		HorseRadish::String::Set(pathToAppend);
		return;
	}

	//se actualmente o caminho não termina com o separador correcto, adiciono-o
	if (HorseRadish::String::EndsWith(HorseRadish::Platform::DirectorySeparatorChar) == false)
		HorseRadish::String::operator+=(HorseRadish::Platform::DirectorySeparatorChar);

	//posso acrescentar o caminho
	HorseRadish::String::operator+=(pathToAppend);

	//não preciso de todas as operações manutenção, só remover duplicados
	HorseRadish::String::RemoveDoubles(HorseRadish::Platform::DirectorySeparatorChar);
}

void Path::Combine(const HorseRadish::hChar * const pathToAppend)
{
	//se não tenho nada
	if ((pathToAppend == nullptr) || (*pathToAppend == '\0'))
		return;

	//se não tenho nada actualmente
	if (HorseRadish::String::IsEmpty(true) == true)
	{
		//fico com o path enviado, verifico-o e pronto
		HorseRadish::String::Set(pathToAppend);
		validadePath();
		return;
	}

	//se actualmente o caminho não termina com o separador correcto, adiciono-o
	if (HorseRadish::String::EndsWith(HorseRadish::Platform::DirectorySeparatorChar) == false)
		HorseRadish::String::operator+=(HorseRadish::Platform::DirectorySeparatorChar);

	//posso acrescentar o caminho
	HorseRadish::String::operator+=(pathToAppend);

	//operações de manutenção
	validadePath();
}

void Path::Combine(const HorseRadish::String &pathToAppend)
{
	//se não tenho nada actualmente
	if (HorseRadish::String::IsEmpty(true) == true)
	{
		//fico com o path enviado, verifico-o e pronto
		HorseRadish::String::Set(pathToAppend);
		validadePath();
		return;
	}

	//se actualmente o caminho não termina com o separador correcto, adiciono-o
	if (HorseRadish::String::EndsWith(HorseRadish::Platform::DirectorySeparatorChar) == false)
		HorseRadish::String::operator+=(HorseRadish::Platform::DirectorySeparatorChar);

	//posso acrescentar o caminho
	HorseRadish::String::operator+=(pathToAppend);

	//operações de manutenção
	validadePath();
}

void Path::RemoveLastComponent()
{
	this->RemoveComponents(1);
}

void Path::RemoveComponents(const HorseRadish::hUInt8 &numComponents)
{
	if (numComponents <= 0)
		return;

	auto iterator = HorseRadish::String::Iterator(*this);
	iterator.Last();

	auto componentsRemoved = 0;
	for (; iterator.IsFirst() == false; iterator--)
	{
		if (*iterator != HorseRadish::Platform::DirectorySeparatorChar)
			continue;

		componentsRemoved++;
		if (componentsRemoved >= numComponents)
			break;
	}

	HorseRadish::String::CloseAt(iterator.GetCaracterPosition());
}

void Path::RemoveFile()
{
	//tiro o iterator e movo para o fim
	auto iterator = HorseRadish::String::Iterator(*this);
	iterator.Last();

	//enquanto não chegar ao separador (ou ao início do caminho)
	while ((*iterator != HorseRadish::Platform::DirectorySeparatorChar) && (iterator.IsFirst() == false))
		iterator--;

	//fecho a string onde achei o separador
	HorseRadish::String::CloseAt(iterator.GetCaracterPosition());
}

Path& Path::operator+=(const Path& path)
{
	//basta mandar combinar
	this->Combine(path);

	//devolvo-me
	return *this;
}

Path& Path::operator+=(const HorseRadish::hChar * const path)
{
	//basta mandar combinar
	this->Combine(path);

	//devolvo-me
	return *this;
}

}//namespace IO
}//namespace HorseRadish