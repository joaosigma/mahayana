#include "common\platform.hpp"

#include <windows.h>

#include "SGPUCounter.hpp"

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§   -= Classe SGPUCounter =-	§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
SGPUCounter::SGPUCounter(unsigned int maxSaveSamples)
{
	//limpo tudo
	this->numMaxSamples = this->curSample = 0;

	//guardo o número de samples a guardar
	this->numMaxSamples = maxSaveSamples;
	if (this->numMaxSamples == 0)
		this->numMaxSamples = 1;
}

SGPUCounter::~SGPUCounter()
{	reset();}

bool SGPUCounter::addCounter(const char * const counterName)
{	return addCounter(counterName, false, 1.0f, 1.0f, 1.0f);}

bool SGPUCounter::addCounter(const char * const counterName, bool isPercent)
{	return addCounter(counterName, isPercent, 1.0f, 1.0f, 1.0f);}

bool SGPUCounter::addCounter(const char * const counterName, const float r, const float g, const float b)
{	return addCounter(counterName, false, 1.0f, 1.0f, 1.0f);}

bool SGPUCounter::addCounter(const char * const counterName, bool isPercent, const float r, const float g, const float b)
{
	CounterData *newCounter;
	float *valuesArray;
	UINT64 attrib;

	//verificar entrada
	if (counterName==nullptr || *counterName=='\0')
		return false;

	//crio a lista para os arrays
	valuesArray = new float[numMaxSamples];

	//aumento o tamanho do array
	newCounter = this->arrayCounters.Add();
	if (newCounter == nullptr)
	{
		delete[] valuesArray;
		return false;
	}

	//preencho o contador e correu tudo bem	
	newCounter->isPercent = isPercent;
	newCounter->index=0;
	newCounter->name.Set(HorseRadish::String::UTF8, counterName);
	newCounter->values=valuesArray;
	newCounter->minValue = HorseRadish::Math::INFINITY;
	newCounter->maxValue = -HorseRadish::Math::INFINITY;
	newCounter->color[0]=r;
	newCounter->color[1]=g;
	newCounter->color[2]=b;
	newCounter->color[3]=1.0f;
	memset(newCounter->values, 0, sizeof(float)*numMaxSamples);
	return true;
}

void SGPUCounter::reset()
{
	//passo por cada um e liberto o espaço das samples
	for(int i = 0; i < this->arrayCounters.GetNumElements(); i++)
		delete[] this->arrayCounters[i].values;

	//liberto a lista inteira
	this->arrayCounters.Clear();
	
	//limpo tudo
	this->curSample = 0;
}

void SGPUCounter::sampleCounter(const char * const counterName, const float &newSample) const
{
	//para cada um dos contadores
	for (unsigned int i=0; i<this->arrayCounters.GetNumElements(); i++) 
	{
		//enquanto não for aquele que quero
		if (this->arrayCounters[i].name != counterName)
			continue;

		//basta gravar o valor, recalcular min e max e já tá
		this->arrayCounters[i].values[curSample] = newSample;
		this->arrayCounters[i].minValue = HorseRadish::Math::fMin(this->arrayCounters[i].minValue, this->arrayCounters[i].values[curSample]);
		this->arrayCounters[i].maxValue = HorseRadish::Math::fMax(this->arrayCounters[i].maxValue, this->arrayCounters[i].values[curSample]);
		break;
	}
}

void SGPUCounter::sampleMoveNext() const
{
	//próximo sample
	this->curSample++;

	//para não ultrapassar
	if (this->curSample >= this->numMaxSamples)
	{
		//começo do início
		this->curSample = 0;

		//para todos os contadores
		for (int i = 0; i < this->arrayCounters.GetNumElements(); i++) 
		{
			this->arrayCounters[i].minValue = HorseRadish::Math::INFINITY;
			this->arrayCounters[i].maxValue = -HorseRadish::Math::INFINITY;
		}
	}
}

const char* SGPUCounter::getCounterName(const unsigned int counterNumber) const
{
	//verifico se o contador tem um valor correcto
	if (counterNumber==0 || counterNumber>this->arrayCounters.GetNumElements())
		return nullptr;
	return this->arrayCounters[counterNumber-1].name.GetData();
}

unsigned int SGPUCounter::getCounterNumber(const char * const counterName) const
{
	//verificar parametro de entrada
	if (counterName==nullptr || *counterName=='\0')
		return 0;

	//procuro pelo contador
	for(int i = 0; i < this->arrayCounters.GetNumElements(); i++)
	{
		//se achei posso logo sair
		if (this->arrayCounters[i].name == counterName)
			return (i+1);
	}

	//chegando aqui não existe nada
	return 0;
}

bool SGPUCounter::getCounterIsPercent(const unsigned int counterNumber) const
{
	//verifico se o contador tem um valor correcto
	if (counterNumber==0 || counterNumber>this->arrayCounters.GetNumElements())
		return nullptr;
	return this->arrayCounters[counterNumber-1].isPercent;
}

const float* SGPUCounter::getCounterColor(const unsigned int counterNumber) const
{
	//verifico se o contador tem um valor correcto
	if (counterNumber==0 || counterNumber>this->arrayCounters.GetNumElements())
		return nullptr;
	return this->arrayCounters[counterNumber-1].color;
}

int SGPUCounter::getNumCounters() const
{	return this->arrayCounters.GetNumElements();}

int SGPUCounter::getMaxNumSamples() const
{	return numMaxSamples;}

int SGPUCounter::getCurrentSample() const
{	return curSample;}

float SGPUCounter::getValue(const unsigned int counterNumber, const unsigned int sample) const
{
	//verifico se o contador e o sample tem um valor correcto
	if (counterNumber==0 || counterNumber>this->arrayCounters.GetNumElements() || sample==0 || sample>numMaxSamples)
		return (0.0f);

	//posso simplesmente devolver
	return this->arrayCounters[counterNumber-1].values[sample-1];
}

float SGPUCounter::getValueCurrent(const unsigned int counterNumber) const
{
	//verifico se o contador tem um valor correcto
	if (counterNumber==0 || counterNumber>this->arrayCounters.GetNumElements())
		return (0.0f);

	//basta verificar o valor do sample e posso devolver o valor
	if (curSample>0)
		return this->arrayCounters[counterNumber-1].values[curSample-1];
	return this->arrayCounters[counterNumber-1].values[0];
}

float SGPUCounter::getValueAvg(const unsigned int counterNumber) const
{
	float sampleVal,numSamplesInv;

	//verifico se o contador tem um valor correcto
	if (counterNumber==0 || counterNumber>this->arrayCounters.GetNumElements())
		return (0.0f);

	//só tenho de calcular a média e devolver
	numSamplesInv = 1.0f/((float)curSample);
	sampleVal = 0.0f;
	for(unsigned int i=0; i<curSample; i++)
		sampleVal += this->arrayCounters[counterNumber-1].values[i]*numSamplesInv;

	//e já tá
	return sampleVal;
}

void SGPUCounter::getValuesMinMax(const unsigned int counterNumber, float * const valueMin, float * const valueMax) const
{
	float sampleVal,numSamplesInv;

	//verifico se o contador tem um valor correcto
	if (counterNumber==0 || counterNumber>this->arrayCounters.GetNumElements())
		return;

	//basta gravar os valores
	if (valueMin != nullptr)
		*valueMin = this->arrayCounters[counterNumber-1].minValue;
	if (valueMax != nullptr)
		*valueMax = this->arrayCounters[counterNumber-1].maxValue;
}