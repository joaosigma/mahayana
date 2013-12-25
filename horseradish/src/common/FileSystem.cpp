#include "FileSystem.hpp"
#include "Math.hpp"
#include "Hashing.hpp"
#include "Sorting.hpp"
#include "UTF.hpp"
#include "Types.hpp"

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Funções auxiliares locais		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
static
uLong ZCALLBACK zwrite(voidpf opaque, voidpf stream, const void* buf, uLong size)
{	return 0;}
static
int ZCALLBACK zerror(voidpf opaque, voidpf stream)
{	return 0;}

static
voidpf ZCALLBACK zopen(voidpf opaque, const char* filename, int mode)
{
	HorseRadish::Streams::FileStream *fileStream;

	//crio o stream para ler o ficheiro
	fileStream = new HorseRadish::Streams::FileStream((const HorseRadish::hChar*)filename, true, false);
	
	//e devolvo esse stream
	return ((voidpf)fileStream);
}

static
uLong ZCALLBACK zread(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	HorseRadish::Streams::FileStream *fileStream;

	//preciso de converter isto
	fileStream = (HorseRadish::Streams::FileStream*)stream;

	//basta mandar ler
	return fileStream->Read(buf,size);
}

static
long ZCALLBACK ztell(voidpf opaque, voidpf stream)
{
	HorseRadish::Streams::FileStream *fileStream;

	//preciso de converter isto
	fileStream = (HorseRadish::Streams::FileStream*)stream;

	//basta mandar ler
	return fileStream->GetPosition();
}

static
long ZCALLBACK zseek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	HorseRadish::Streams::FileStream *fileStream;

	//preciso de converter isto
	fileStream = (HorseRadish::Streams::FileStream*)stream;

	//conforme a origem
	if (origin == ZLIB_FILEFUNC_SEEK_CUR)
		fileStream->Seek(offset, HorseRadish::Streams::Stream::Current);
	else if (origin == ZLIB_FILEFUNC_SEEK_END)
		fileStream->Seek(offset, HorseRadish::Streams::Stream::End);
	else if (origin == ZLIB_FILEFUNC_SEEK_SET)
		fileStream->Seek(offset, HorseRadish::Streams::Stream::Begin);
	else
		return 1;

	//correu bem
	return 0;
}

static
int ZCALLBACK zclose(voidpf opaque, voidpf stream)
{
	HorseRadish::Streams::FileStream *fileStream;

	//preciso de converter isto
	fileStream = (HorseRadish::Streams::FileStream*)stream;

	//mando fechar o ficheiro e destruo o stream
	fileStream->Close();
	delete fileStream;

	//e prontos
	return 0;
}

static
void overloadZLibIO(zlib_filefunc_def * const zlibFileFunc)
{
	//não dá jeito
	if (zlibFileFunc == nullptr)
		return;

	//só tenho de colocar bem os ponteiros
	zlibFileFunc->zclose_file = zclose;
	zlibFileFunc->zopen_file = zopen;
	zlibFileFunc->zread_file = zread;
	zlibFileFunc->zseek_file = zseek;
	zlibFileFunc->ztell_file = ztell;

	zlibFileFunc->zerror_file = zerror;
	zlibFileFunc->zwrite_file = zwrite;
}

namespace HorseRadish
{
namespace IO
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Coisas para exportar		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
FileSystem::MountData::MountData(const HorseRadish::hChar * const mountPoint)
{ 
	//guardo isto
	this->mountPoint.Set(mountPoint);
}
FileSystem::MountData::~MountData()
{ 
	//deixo de ter um mount point
	this->mountPoint.Clear();
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ A classe para montar directorias		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
FileSystem::MountDataPath::MountDataPath(const HorseRadish::hChar * const baseFolder, const HorseRadish::hChar * const mountPoint)
	: MountData(mountPoint)
{
	//guardo isto
	this->baseFolder.Set(baseFolder);
}

FileSystem::MountDataPath::~MountDataPath()
{
	//limpo isto
	this->baseFolder.Clear();
}

FileSystem::MountType FileSystem::MountDataPath::GetMountType() const
{
	//basta devolver isto
	return FileSystem::MountTypePath;
}

void FileSystem::MountDataPath::FilesEnumerate()
{
}

Streams::Stream* FileSystem::MountDataPath::FileRead(const HorseRadish::hChar * const filePath)
{
	HorseRadish::Streams::FileStream *fileStream;
	HorseRadish::IO::Path pathFinal;

	//crio o caminho
	pathFinal.Set(this->baseFolder);
	pathFinal += (const HorseRadish::hChar*)filePath;

	//crio o file stream para abrir o ficheiro
	fileStream = new HorseRadish::Streams::FileStream(pathFinal, true, false);

	//se não for válido, apago-o
	if (fileStream->IsValid() == false)
	{
		delete fileStream;
		return nullptr;
	}

	//basta devolver o stream do ficheiro
	return fileStream;
}

bool FileSystem::MountDataPath::FileExists(const HorseRadish::hChar * const filePath)
{
	HorseRadish::IO::Path pathFinal;

	//crio o caminho
	pathFinal.Set(this->baseFolder);
	pathFinal += (const HorseRadish::hChar*)filePath;

	//basta mandar verificar se ele existe
	return FileSystem::FileExists(pathFinal);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ A classe para montar ZIPs		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
int FileSystem::MountDataZip::compareFileNameMD5(const ZipEntry &objA, const ZipEntry &objB)
{
	if (objA.fileNameMD5.i64[0] < objB.fileNameMD5.i64[0])
		return -1;
	if (objA.fileNameMD5.i64[0] > objB.fileNameMD5.i64[0])
		return 1;

	if (objA.fileNameMD5.i64[1] < objB.fileNameMD5.i64[1])
		return -1;
	if (objA.fileNameMD5.i64[1] > objB.fileNameMD5.i64[1])
		return 1;

	return 0;
}

FileSystem::MountDataZip::MountDataZip(const HorseRadish::hChar * const zipPath, const HorseRadish::hChar * const mountPoint)
	: MountData(mountPoint)
{
	ZipEntry *walkerEntradas;
	unz_global_info zipInfo;
	zlib_filefunc_def zlibAPI;
	unz_file_info zipFileCurFileInfo;
	char zipFileCurFileName[512];

	//limpo tudo
	this->zipFile = nullptr;
	this->numEntradas = 0;
	this->numFolders = 0;
	this->numFiles = 0;
	this->listaEntradas = nullptr;
	this->poolStrings = nullptr;

	//guardo isto
	this->zipPath.Set(zipPath);

	//faço overload à maneira como ele le as coisa do SO
	overloadZLibIO(&zlibAPI);

	//abrir o ficheiro
	this->zipFile = unzOpen2(this->zipPath.GetData(), &zlibAPI);
	if (this->zipFile == nullptr)
		return;

	//tiro as propriedades do ZIP
	unzGetGlobalInfo(this->zipFile, &zipInfo);

	//crio espaço para guardar o número de entradas no zip
	this->numEntradas = zipInfo.number_entry;
	this->listaEntradas = new ZipEntry[this->numEntradas];
	if (this->listaEntradas == nullptr)
	{
		//fecho o zip e elimino todas as entradas
		unzClose(this->zipFile);
		this->numEntradas = 0;
		return;
	}

	//limpo tudo
	memset(this->listaEntradas, 0, sizeof(ZipEntry) * this->numEntradas);

	//preciso de criar isto para ter espaço para todas as strings necessárias
	this->poolStrings = new HorseRadish::Memory::PoolGrow(HorseRadish::Platform::KiloByte * 5);

	//vou para o primeiro ficheiro
	unzGoToFirstFile(this->zipFile);

	//para percorrer as entradas
	walkerEntradas = this->listaEntradas;

	//preencho os dados dos ficheiros
	for(int i=0; i < this->numEntradas; i++)
	{
		//tiro a informação e se não tenho tamanho para o ficheiro, é porque é uma directoria
		unzGetCurrentFileInfo(this->zipFile, &zipFileCurFileInfo, zipFileCurFileName, sizeof(zipFileCurFileName), nullptr, 0, nullptr, 0);

		//se o tamanho for zero, é uma directoria
		if (zipFileCurFileInfo.uncompressed_size == 0)
		{
			//incremento isto
			this->numFolders++;

			//passo para o próximo ficheiro
			unzGoToNextFile(this->zipFile);
			continue;
		}

		//chegando aqui é um ficheiro
		this->numFiles++;

		//guardo o tamanho do ficheiro e onde é que ele está no zip
		walkerEntradas->fileSize = zipFileCurFileInfo.uncompressed_size;
		unzGetFilePos(this->zipFile, &walkerEntradas->filePos);

		//preciso de espaço para guardar o nome
		walkerEntradas->fileName = (char*)this->poolStrings->alloc(zipFileCurFileInfo.size_filename + 1);
		memcpy(walkerEntradas->fileName, zipFileCurFileName, zipFileCurFileInfo.size_filename);
		walkerEntradas->fileName[zipFileCurFileInfo.size_filename] = '\0';

		//calculo o MD5 do nome
		HorseRadish::Hashing::CalculateMD5(walkerEntradas->fileName, zipFileCurFileInfo.size_filename, &walkerEntradas->fileNameMD5);

		//faço o ZIP avançar para o próximo ficheiro
		unzGoToNextFile(this->zipFile);

		//posso ir para a próxima entrada
		walkerEntradas++;
	}

	//tenho de ordenar as entradas todas pelo MD5 do nome
	HorseRadish::Sorting::QuickSort<ZipEntry>(this->listaEntradas, this->numFiles, FileSystem::MountDataZip::compareFileNameMD5);

	//para todos os ficheiros que tenho
	/*for (int i=0; i < (this->numFiles - 1); i++)
	{
		//se forem diferentes
		if (this->listaEntradas[i].fileNameMD5.i64[0] != this->listaEntradas[i+1].fileNameMD5.i64[0])
			continue;
		if (this->listaEntradas[i].fileNameMD5.i64[1] != this->listaEntradas[i+1].fileNameMD5.i64[1])
			continue;

		//chegando aqui tenho nomes iguais!

		//elimino todas as entradas
		this->numEntradas = 0;
		this->numFiles = 0;
		this->numFolders = 0;
		return;
	}*/
}

FileSystem::MountDataZip::~MountDataZip()
{
	//apago algumas listas
	if (this->poolStrings != nullptr)
		delete this->poolStrings;
	if (this->listaEntradas != nullptr)
		delete this->listaEntradas;

	//limpo tudo
	this->zipPath.Clear();
	this->poolStrings = nullptr;
	this->listaEntradas = nullptr;
	this->numEntradas = 0;
	this->numFolders = 0;
	this->numFiles = 0;

	//fecho o zip propriamente dito
	if (this->zipFile != nullptr)
		unzClose(this->zipFile);
	this->zipFile = nullptr;
}

FileSystem::MountType FileSystem::MountDataZip::GetMountType() const
{
	//basta devolver isto
	return FileSystem::MountTypeZIP;
}

int FileSystem::MountDataZip::GetNumberFiles() const
{
	//basta devolver isto
	return this->numFiles;
}

void FileSystem::MountDataZip::FilesEnumerate()
{
}

Streams::Stream* FileSystem::MountDataZip::FileRead(const HorseRadish::hChar * const filePath)
{
	ZipEntry *fileZipData, fileProxy;
	Streams::MemoryStream *memStream;
	int fileZipIndex;
	void *fileData;

	//se não tenho nada
	if ((filePath == nullptr) || (*filePath == '\0'))
		return nullptr;

	//preparo uma entrada para usar como comparação
	memset(&fileProxy, 0, sizeof(ZipEntry));
	HorseRadish::Hashing::CalculateMD5(filePath, strlen((const char*)filePath), &fileProxy.fileNameMD5);

	//procuro pelo MD5
	fileZipIndex = HorseRadish::Sorting::BinarySearch<ZipEntry>(this->listaEntradas, this->numFiles, FileSystem::MountDataZip::compareFileNameMD5, fileProxy);
	if (fileZipIndex < 0)
		return nullptr;

	//o ponteiro para os dados pedidos
	fileZipData = this->listaEntradas + fileZipIndex;

	//preciso de allocar espaço para ler o ficheiro
	fileData = malloc(fileZipData->fileSize);
	if (fileData == nullptr)
		return nullptr;

	//vou para onde quero
	unzGoToFilePos(this->zipFile, &fileZipData->filePos);
	
	//abro para leitura
	unzOpenCurrentFile(this->zipFile);

	//leio tudo
	unzReadCurrentFile(this->zipFile, fileData, fileZipData->fileSize);

	//fecho o ficheiro que abri
	unzCloseCurrentFile(this->zipFile);

	//agora que tenho tudo do ficheiro, só tenho de criar um stream
	memStream = new Streams::MemoryStream(fileData, fileZipData->fileSize, false, Streams::MemoryStream::ManagementType::None);
	if (memStream == nullptr)
	{
		free(fileData);
		return nullptr;
	}

	//chegando aqui correu tudo bem
	return memStream;
}

bool FileSystem::MountDataZip::FileExists(const HorseRadish::hChar * const filePath)
{
	ZipEntry fileProxy;
	int fileZipIndex;

	//se não tenho nada
	if ((filePath == nullptr) || (*filePath == '\0'))
		return false;

	//preparo uma entrada para usar como comparação
	memset(&fileProxy, 0, sizeof(ZipEntry));
	HorseRadish::Hashing::CalculateMD5(filePath, strlen((const char*)filePath), &fileProxy.fileNameMD5);

	//procuro pelo MD5
	fileZipIndex = HorseRadish::Sorting::BinarySearch<ZipEntry>(this->listaEntradas, this->numFiles, FileSystem::MountDataZip::compareFileNameMD5, fileProxy);

	//achei se o indice for maior ou igual a 0
	return (fileZipIndex >= 0);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ A classe principal que implementa o sistema de ficheiros		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

//preciso de inicializar estas contantes
const int FileSystem::FolderNameLength = 128;
const int FileSystem::FileNameLength = 256;
const int FileSystem::PathLength = 16383;

FileSystem::FileSystem(int maxNumMounts)
{
	//guardo o número máximo
	this->maxNumMounts = Math::iClamp(maxNumMounts, 1, 10);

	//crio espaço para guardar os dados dos mounts
	this->listMounts = new MountData*[this->maxNumMounts];
	this->numMounts = 0;
}

FileSystem::~FileSystem()
{
	//termino cada mudança
	for (int i = 0; i < this->listWatchChange.size(); i++)
		FindCloseChangeNotification(this->listWatchChange[i].changeHandle);
	this->listWatchChange.clear();

	//removo cada mount
	for(int i = 0; i < this->numMounts; i++)
		delete this->listMounts[i];

	//apago a própria lista
	delete this->listMounts;

	//limpo tudo
	this->listMounts = nullptr;
	this->maxNumMounts = 0;
	this->numMounts = 0;
}

void FileSystem::FindFiles(const HorseRadish::hChar * const baseFolderAndFilter, const bool returnFilesFullPath, std::function<void (const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)> actionFileFound)
{
	HANDLE handleFind;
	WIN32_FIND_DATA findData;
	HorseRadish::String filePath;
	HorseRadish::IO::Path basePath, fileFinalPath;
	HorseRadish::hUInt64 fileSize;
	wchar_t baseFolderAndFilterWChar[256];

	//se não tenho nada a fazer
	if ((baseFolderAndFilter == nullptr) || (actionFileFound == nullptr))
		return;

	//tenho de converter a string para WideChar
	HorseRadish::UTF::ConvertUTF8To(baseFolderAndFilter, HorseRadish::UTF::Windows, baseFolderAndFilterWChar, sizeof(baseFolderAndFilterWChar));

	//o primeiro ficheiro
	handleFind = FindFirstFile(baseFolderAndFilterWChar, &findData);
	if (handleFind == INVALID_HANDLE_VALUE) 
		return;

	//o caminho base (pra formar o nome completo se for preciso
	basePath.Set(baseFolderAndFilter);
	basePath.RemoveFile();

	do
	{
		//se for uma directoria
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		//converto o nome do ficheiro
		filePath.Set(HorseRadish::String::Windows, findData.cFileName);

		//converto o tamanho
		fileSize = (findData.nFileSizeHigh * (MAXDWORD + 1)) + findData.nFileSizeLow;

		//se tenho de devolver o caminho completo
		if (returnFilesFullPath == true)
		{
			//crio o caminho completo
			fileFinalPath.Set(basePath);
			fileFinalPath.Combine(filePath);
		}
		else
		{
			//basta ir o nome do ficheiro
			fileFinalPath.Set(filePath);
		}

		//faço a acção a executar para cada ficheiro encontrado
		actionFileFound(fileFinalPath, fileSize);

	//próximo ficheiro
	}while (FindNextFile(handleFind, &findData) != 0);


	//fecho o handle
	FindClose(handleFind);
}

bool FileSystem::FileExists(const HorseRadish::hChar * const filePath)
{
	wchar_t filePathWChar[256];
	DWORD fileAtributes;

	//se não tenho nada
	if ((filePath == nullptr) || (*filePath == '\0'))
		return false;

	//tenho de converter a string para WideChar
	HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Windows, filePathWChar, sizeof(filePathWChar));

	//peço informação acerca do ficheiro
	fileAtributes = GetFileAttributes(filePathWChar);

	//se não existe
	if (fileAtributes == INVALID_FILE_ATTRIBUTES)
		return false;

	//não quero directorias
	if ((fileAtributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		return false;

	//chegando aqui tenho realmentre um ficheiro
	return true;
}

bool FileSystem::MountPath(const HorseRadish::IO::Path &baseFolder, const HorseRadish::hChar * const mountPoint)
{
	MountDataPath *novoMount;

	//se já atingi o máximo possível
	if (this->numMounts >= this->maxNumMounts)
		return false;

	//crio um novo mount
	novoMount = new MountDataPath(baseFolder, mountPoint);

	//guardo o novo mount
	this->listMounts[this->numMounts] = novoMount;
	this->numMounts++;

	//correu tudo bem
	return true;
}

bool FileSystem::MountZip(const HorseRadish::IO::Path &zipPath, const HorseRadish::hChar * const mountPoint, int * const numFilesZip)
{
	MountDataZip *novoMount;

	//se já atingi o máximo possível
	if (this->numMounts >= this->maxNumMounts)
		return false;

	//crio um novo mount
	novoMount = new MountDataZip(zipPath, mountPoint);

	//guardo o novo mount
	this->listMounts[this->numMounts] = novoMount;
	this->numMounts++;

	//se for para escrever o número de ficheiros no zip
	if (numFilesZip != nullptr)
		*numFilesZip = novoMount->GetNumberFiles();

	//correu tudo bem
	return true;
}

Streams::Stream* FileSystem::FileRead(const char * const filePath)
{
	//verificar o caminho
	if ((filePath == nullptr) || (*filePath == '\0'))
		return nullptr;

	//passo por todos os mounts
	for(int i = 0; i < this->numMounts; i++)
	{
		MountData *curMount;
		Streams::Stream *fileStream;

		//isto dá jeito
		curMount = this->listMounts[i];

		//se o mount for um caminho, tento ver se ele existe primeiro (é mais rápido)
		if ((curMount->GetMountType() == FileSystem::MountTypePath) && (curMount->FileExists((const HorseRadish::hChar *)filePath) == false))
			continue;

		//tento ler e se achei, posso devolver
		fileStream = curMount->FileRead((const HorseRadish::hChar *)filePath);
		if (fileStream != nullptr)
			return fileStream;
	}

	//chegando aqui não achei o ficheiro
	return nullptr;
}

Streams::Stream* FileSystem::FileRead(const char * const filePath, const FileSystem::MountType mountType)
{
	//verificar o caminho
	if ((filePath == nullptr) || (*filePath == '\0'))
		return nullptr;

	//passo por todos os mounts
	for(int i = 0; i < this->numMounts; i++)
	{
		MountData *curMount;
		Streams::Stream *fileStream;

		//isto dá jeito
		curMount = this->listMounts[i];

		//se for para ignorar este mount
		if (curMount->GetMountType() != mountType)
			continue;

		//se o mount for um caminho, tento ver se ele existe primeiro (é mais rápido)
		if ((curMount->GetMountType() == FileSystem::MountTypePath) && (curMount->FileExists((const HorseRadish::hChar *)filePath) == false))
			continue;

		//tento ler e se achei, posso devolver
		fileStream = curMount->FileRead((const HorseRadish::hChar *)filePath);
		if (fileStream != nullptr)
			return fileStream;
	}

	//chegando aqui não achei o ficheiro
	return nullptr;
}

int FileSystem::WatchChangeCreate(const HorseRadish::hChar * const baseFolder, bool includeSubFolders, const FileSystem::ChangeType changeType)
{
	HANDLE handleChange;
	DWORD changeFlags;
	int changeID;
	wchar_t baseFolderWChar[256];

	//verifico parâmetros
	if ((baseFolder == nullptr) || (changeType == 0))
		return -1;

	//tenho de converter a string para WideChar
	HorseRadish::UTF::ConvertUTF8To(baseFolder, HorseRadish::UTF::Windows, baseFolderWChar, sizeof(baseFolderWChar));

	//o que vou ficar à escuta
	changeFlags = 0;
	if (changeType & FileName)
		changeFlags = FILE_NOTIFY_CHANGE_FILE_NAME;
	if (changeType & FileSize)
		changeFlags = FILE_NOTIFY_CHANGE_SIZE;
	if (changeType & FileLastWrite)
		changeFlags = FILE_NOTIFY_CHANGE_LAST_WRITE;

	//inicio a "escuta" da mudança
	handleChange = FindFirstChangeNotification(baseFolderWChar, includeSubFolders ? TRUE : FALSE, changeFlags);
	if (handleChange == INVALID_HANDLE_VALUE) 
		return 0;

	//crio uma nova entrada para esta "escuta"
	changeID = this->listWatchChange.size() + 13;
	this->listWatchChange.push_back(WatchChangeData(changeID, handleChange));

	//devolvo o novo ID
	return changeID;
}

void FileSystem::WatchChangeDelete(const int watchChangeID)
{
	//para cada entrada
	for (int i = 0; i < this->listWatchChange.size(); i++)
	{
		//se não é esta
		if (this->listWatchChange[i].changeID != watchChangeID)
			continue;

		//termino a "escuta" e apago a entrada
		FindCloseChangeNotification(this->listWatchChange[i].changeHandle);
		this->listWatchChange.erase(this->listWatchChange.begin() + i);
		break;
	}
}

bool FileSystem::WatchChanged(const int watchChangeID)
{
	//para cada entrada
	for (int i = 0; i < this->listWatchChange.size(); i++)
	{
		//se não é esta
		if (this->listWatchChange[i].changeID != watchChangeID)
			continue;

		//verifico o estado da "escuta"
		bool houveMudanca = (WaitForSingleObject(this->listWatchChange[i].changeHandle, 0) == WAIT_OBJECT_0);

		//se houve alguma mudança, volto a ficar à escuta
		if (houveMudanca == true)
			FindNextChangeNotification(this->listWatchChange[i].changeHandle);

		//posso devolver se houve ou não mudança
		return houveMudanca;
	}

	//não achei a "escuta"
	return false;
}

}//namespace IO
}//namespace HorseRadish