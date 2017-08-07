// 27/02/17
// Indice metrico para nubes de puntos
// Referencias de unos en bitmap + Indice Metrico para busquda
// Libreria SDSL Lite 2.0
#include<sdsl/bit_vectors.hpp>


// STD
#include <vector>
#include <unordered_set>
#include <set>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <limits>
#include <map>
#include <functional>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <vptPointers.h>
#include <string>

// I/O Consola
#include <iostream>
#include <stdio.h>

// Timer
#include <chrono>
#include "codecfactory.h"
#include "intersection.h"

using namespace SIMDCompressionLib;

class Punto
{
public:
	std::vector<unsigned int> Posiciones;
	unsigned int ID;
};

struct Recall
{
public:
	unsigned short int recall_five;
	unsigned short int recall_ten;
	unsigned short int recall_twenty;
};



// Funcion para leer archivo de texto - Declaracion
void Posiciones(std::string fileName, int delta, int cmax, std::vector<Punto> &resultado, std::set<unsigned int> &nubesID);
int Posicion(int x, int y, int cmax, int delta);

//Funcion para calcular distancia coseno
double DistanciaCoseno(const Punto &q, const Punto &bitmap, int pint);

//Funcion para calcular funcion de distancia para todos los bitmaps
void FuncionDistancia(Punto &q, std::vector<Punto> &bitmapsNubes, std::vector<std::pair<unsigned int, double>> &distV);
//Funcion para calcular promedio de un vector
double Promedio(const std::vector<double> v);

//Funcion para calcular desviacion estandar
double DesviacionEstandar(const std::vector<double> v, const double promedio);

//Funcion para calcular producto interno
int InnerProduct(const Punto &va, const Punto &vb);

void LinearSearch(std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);
//double DistanciaHamming(const Punto &q, const Punto &bitmap, int pint);
//double DistanciaJaccard(const Punto &q, const Punto &bitmap, int pint);
double distCoseno(const Punto &p1, const Punto &p2);
double distHamming(const Punto &p1, const Punto &p2);
double distJaccard(const Punto &p1, const Punto &p2);


void VPTSearch(VptPointers<Punto, distCoseno> &vptree, int k, std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);
void VPTSearch(VptPointers<Punto, distHamming> &vptree, int k, std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);
void VPTSearch(VptPointers<Punto, distJaccard> &vptree, int k, std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);


//Definir Pi 
const double pi = std::acos(-1);




int main()
{
	// Declaracion de variables
	std::string fileName;
	std::string queryName;
	int delta;
	int cmax;
	int num_nubes;

	// Generar numeros aleatorios con distribucion uniforme
	//boost::random::mt19937 gen;

	// Map para almacenar para cada nube una lista de las celdas donde existe un punto en la nube (1's)
	std::vector<Punto> resultadoNubes;
	std::vector<Punto> resultadoQueries;

	// Set para almacenar los identificadores de las nubes
	std::set<unsigned int> nubesID;
	std::set<unsigned int> nubesID2;

	//Variables para calcular tiempo de ejecucion de funciones
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

	//Vector para guardar tiempos de consulta
	std::vector<double> tconsulta;

	//Vector para guardar tamano de resultado
	//std::vector<double>resultado_size;

	//Vector para almacenar espacio promedio en mb de bitvectors
	std::vector<double> bvsize;
	std::vector<double> bvsize2;

	std::vector<double> t;

	int tipo_indice = 1;
	int k = 1;
	int tipo_distancia = 1;
	std::string indice_name = "Busqueda secuencial";
	std::string distancia_name = "default";


	//-------------------------------------------------------------------------------------------------------
	// Inicia Construccion de indice

	// Introducir nombre de archivo, tamano de celda y coordenada maxima
	std::cout << "Indice Metrico para nubes de puntos" << std::endl;
	std::cout << "Lista de enteros de 1's de bitmap + Intersecciones utilizando SIMD + Indice Metrico para busqueda" << std::endl;
	std::cout << "Introduce nombre de archivo de nubes de puntos a indexar" << std::endl;
	std::cin >> fileName;
	std::cout << "Introduce nombre de archivo de nubes de puntos de consulta" << std::endl;
	std::cin >> queryName;
	std::cout << "Introduce coordenada maxima de operacion" << std::endl;
	std::cin >> cmax;
	std::cout << "Introduce tamano de rejilla - Delta" << std::endl;
	std::cin >> delta;
	std::cout << "Introduce tipo de consulta:" << std::endl;
	std::cout << "Busqueda secuencial: 1" << std::endl;
	std::cout << "Vantage Point Tree: 2" << std::endl;
	std::cin >> tipo_indice;

	if (tipo_indice == 2)
	{
		indice_name = "Vantage Point Tree";
		std::cout << "Busqueda Knn: Introduce k" << std::endl;
		std::cin >> k;
		std::cout << "Introduce Distancia" << std::endl;
		std::cout << "Distancia Coseno: 1" << std::endl;
		std::cout << "Distancia Hamming: 2" << std::endl;
		std::cout << "Distancia Jaccard: 3" << std::endl;
		std::cin >> tipo_distancia;
	}


	//Iniciar Lectura de archivo
	std::cout << "\nInicia Construccion de Indice" << std::endl;
	start = std::chrono::high_resolution_clock::now();
	std::cout << "Inicie lectura de archivo de nubes" << std::endl;
	Posiciones(fileName, delta, cmax, resultadoNubes, nubesID);
	std::cout << "Inicie lectura de archivo de nubes de consulta" << std::endl;
	Posiciones(queryName, delta, cmax, resultadoQueries, nubesID2);
	std::cout << "Finalizo Lectura de archivo" << std::endl;


	//Generar Indice
	//std::vector<Punto> bitmapsNubes = GenerarIndice(resultadoNubes,nubesID,cmax,delta,bvsize);
	//std::vector<Punto> queriesNubes = GenerarIndice(resultadoQueries,nubesID2,cmax,delta,bvsize2);

	VptPointers<Punto, distCoseno> vptree1;
	VptPointers<Punto, distHamming> vptree2;
	VptPointers<Punto, distJaccard> vptree3;

	if (tipo_indice == 2)
	{

		switch (tipo_distancia)
		{
		case 1:
			distancia_name = "Coseno";
			vptree1.create(resultadoNubes);
			break;
		case 2:
			distancia_name = "Hamming";
			vptree2.create(resultadoNubes);
			break;
		case 3:
			distancia_name = "Jaccard";
			vptree3.create(resultadoNubes);
			break;
		default:
			distancia_name = "Coseno";
			vptree1.create(resultadoNubes);

		}
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Finalizo Construccion de indice" << std::endl;


	//----------------------------------------------------------
	//Inicia seccion de consulta
	std::vector<std::pair<unsigned int, double>> distResultado;

	Recall recall;
	if (tipo_indice == 2)
	{

		switch (tipo_distancia)
		{
		case 1:
			VPTSearch(vptree1, k, resultadoQueries, resultadoNubes, tconsulta, recall);
			break;
		case 2:
			VPTSearch(vptree2, k, resultadoQueries, resultadoNubes, tconsulta, recall);
			break;
		case 3:
			VPTSearch(vptree3, k, resultadoQueries, resultadoNubes, tconsulta, recall);
			break;
		default:
			VPTSearch(vptree1, k, resultadoQueries, resultadoNubes, tconsulta, recall);
		}
	}
	else
	{
		//std::cout<<"Inicia busqueda secuencial"<<std::endl;

		LinearSearch(resultadoQueries, resultadoNubes, tconsulta, recall);
		//std::cout<<"Termine busqueda lineal"<<std::endl;
	}

	//std::cout<<"LLegue aqui - tamano de tiempo: "<<tconsulta.size()<<std::endl;

	//Tiempo promedio de consulta
	double tm = Promedio(tconsulta);
	//std::cout<<"LLegue aqui2"<<std::endl;
	//Calcular desviacion estandar
	double tsd = DesviacionEstandar(tconsulta, tm);
	//std::cout<<"LLegue aqui3"<<std::endl;

	//Calcular maximo y minimo
	auto max_min = std::minmax_element(tconsulta.begin(), tconsulta.end());
	//std::cout<<"LLegue aqui3"<<std::endl;
	//Calcular tamano promedio de vector de resultados
	//double rsize = Promedio(resultado_size);

	//Tiempo promedio de Inner Product
	//double tinnerp = Promedio(t);
	//std::cout<<"LLegue aqui3"<<std::endl;
	//Tamano promedio de sd bitvector
	//double prom_vector = Promedio(bvsize);
	//std::cout<<"LLegue aqui3"<<std::endl;
	//double suma =  std::accumulate(bvsize.begin(), bvsize.end(),0)/1000000.0;

	//Imprimir resultados
	std::cout << "Indice Metrico para nubes de puntos" << std::endl;
	std::cout << "Lista de enteros de 1's de bitmap + Intersecciones utilizando SIMD + Indice Metrico para busqueda" << std::endl;
	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "Resultados de Construccion de Indice" << std::endl;
	std::cout << indice_name << std::endl;
	std::cout << "Nombre del Archivo de Nubes a indexar: " << fileName << std::endl;
	std::cout << "Nombre del Archivo de Nubes de consulta: " << queryName << std::endl;
	std::cout << "Coordenada maxima: " << cmax << std::endl;
	std::cout << "Tamano de celda (delta): " << delta << std::endl;
	if (tipo_indice == 2)
	{
		std::cout << "Distancia: " << distancia_name << std::endl;
		std::cout << "Numero de vecinos mas cercanos: " << k << std::endl;
	}

	std::cout << "Numero de nubes a indexar: " << resultadoNubes.size() << std::endl;
	std::cout << "Numero de nubes de consulta: " << resultadoNubes.size() << std::endl;
	std::cout << "Tiempo de construccion: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " s" << std::endl;

	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "Resultados de las consultas" << std::endl;
	std::cout << "Tiempo promedio de consulta: " << tm << " us" << std::endl;
	std::cout << "Desviacion estandar de tiempo de consulta: " << tsd << " us" << std::endl;
	std::cout << "Tiempo maximo de consulta: " << *max_min.second << " us" << std::endl;
	std::cout << "Tiempo minimo de consulta: " << *max_min.first << " us" << std::endl;
	//std::cout<<"Tamano promedio en bytes de SD Vector: "<<prom_vector<<" bytes"<<std::endl;
	//std::cout<<"Tamano total en Mbytes de todos los SD Vectors: "<<suma<<std::endl;
	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "Seccion de Recall" << std::endl;
	std::cout << "Recall@5: " << (double)recall.recall_five / nubesID2.size() << std::endl;
	std::cout << "Recall@10: " << (double)recall.recall_ten / nubesID2.size() << std::endl;
	std::cout << "Recall@20: " << (double)recall.recall_twenty / nubesID2.size() << std::endl;

	return 0;
}




void Posiciones(std::string fileName, int delta, int cmax, std::vector<Punto> &resultado, std::set<unsigned int> &nubesID)
{

	//Variables para almacenar una linea del archivo de texto (csv,txt)
	std::string line;
	std::string item;
	std::ifstream inputFile(fileName);
	std::unordered_map<int, std::set<unsigned int>> temporal;

	//Bandera para identificar el primer renglon del archivo (Encabezado)
	int flag{ 0 };

	//Vector que dado un renglon del archivo de texto almacena las coordenadas e identificador de la nube ()
	std::vector<float> d;

	//Inicia lectura de archivo de texto
	if (inputFile.is_open())
	{
		//Ejecutar codigo mientras no se llegue al final del documento
		while (!inputFile.eof())
		{

			//Guardar contenido de una linea del documento
			getline(inputFile, line, '\n');
			d.clear();

			//No almacenar primera linea (Encabezado del archivo)
			if (flag == 0)
			{
				flag = 1;
				continue;
			}

			//Almacenar contenido de primera linea
			std::istringstream iss(line);
			float x{ 0.0 };

			//Leer los datos separados por comas y almacenarlos
			while (getline(iss, item, ','))
			{
				//Convertir string a double
				x = std::stof(item);
				d.push_back(x);
			}

			//Verificar que coordenadas e identificadores sean mayores que cero
			if ((d[0] >= 0) && (d[1] >= 0) && (d[2] >= 0))
			{
				unsigned int p{ 0 };

				//Calcular posicion de 1 en bitvector
				p = (unsigned int)Posicion(std::floor(d[1] / delta), std::floor(d[2] / delta), cmax, delta);
				unsigned int cid = (unsigned int)d[0];

				//Guardar identificador de la nube en conjunto
				nubesID.insert(cid);
				//Almacenar resultados en mapa ID_Nube -> Posicion de 1 en bitmap
				temporal[(unsigned int)d[0]].insert(p);
			}
		}
		inputFile.close();
	}

	//std::cout<<"LLegue aqui"<<std::endl;
	std::set<unsigned int> temp1;
	std::vector<unsigned int> temp2;
	//Ordenas listas de enteros
	for (auto p : nubesID)
	{
		temp1 = temporal[p];
		std::vector<unsigned int> temp2;

		for (auto item : temp1)
		{
			temp2.push_back(item);

		}
		std::sort(temp2.begin(), temp2.end());
		Punto punto;
		punto.ID = p;
		punto.Posiciones = temp2;
		resultado.push_back(punto);

	}

}

int Posicion(int x, int y, int cmax, int delta)
{
	return x + (cmax / delta*y);
}

double DistanciaCoseno(const Punto &q, const Punto &bitmap, int pint)
{
	//Calcular norma de bitmap de consulta
	double nq = std::sqrt(q.Posiciones.size());

	//Calcular norma de bitmap en base de datos
	//double nn = norm(bitmap, unos_b);
	double nn = std::sqrt(bitmap.Posiciones.size());

	double rp = pint / (nq*nn);

	if (rp >= 1)
	{
		return 0;
	}

	return std::acos(rp);

}

int DistanciaHamming(const Punto &q, const Punto &bitmap, int pint)
{
	return q.Posiciones.size() + bitmap.Posiciones.size() - (2 * pint);
}

double DistanciaJaccard(const Punto &q, const Punto &bitmap, int pint)
{
	return 1 - ((double)pint / (q.Posiciones.size() + bitmap.Posiciones.size() - pint));
}


void FuncionDistancia(Punto &q, std::vector<Punto> &bitmapsNubes, std::vector<std::pair<unsigned int, double>> &distV)
{
	for (auto nubeV : bitmapsNubes)
	{
		double distc = distCoseno(q, nubeV);
		distV.push_back(std::make_pair(nubeV.ID, distc));
	}
}

double Promedio(const std::vector<double> v)
{
	return std::accumulate(v.begin(), v.end(), 0) / v.size();
}

double DesviacionEstandar(const std::vector<double> v, const double promedio)
{
	double var{ 0 };

	for (int i{ 0 }; i<v.size(); i++)
	{
		var += (v[i] - promedio)*(v[i] - promedio);
	}

	var = var / (v.size() - 1);
	return std::sqrt(var);
}


int InnerProduct(const Punto &va, const Punto &vb)
{

	std::vector<unsigned int> a = va.Posiciones;
	std::vector<unsigned int> b = vb.Posiciones;

	//Tamano de bitmaps
	unsigned int size_a = a.size();
	unsigned int size_b = b.size();
	unsigned int size_max = size_a;

	if (size_a <= size_b)
	{
		size_max = size_b;
	}

	//Variable para guardar resultado
	std::vector<unsigned int>r(size_max);

	// using SIMD intersection
	intersectionfunction inter = IntersectionFactory::getFromName("lemire_highlyscalable_intersect_SIMD");

	// we are going to intersect mydata and mydata2 and write back
	// the result to mydata2
	//site_t
	//std::cout<<"Tamano1: "<<a.size()<<" - Tamano2: "<<b.size()<<std::endl;
	unsigned int intersize = inter(a.data(), a.size(), b.data(),
		b.size(), r.data());



	return intersize;
}

void LinearSearch(std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r)
{
	//Realizar consulta
	r.recall_five = 0;
	r.recall_ten = 0;
	r.recall_twenty = 0;


	for (auto q : queriesNubes)
	{

		std::vector<std::pair<unsigned int, double>> distVector;

		std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
		start1 = std::chrono::high_resolution_clock::now();

		//Calcular producto interno para todos los bitmaps
		FuncionDistancia(q, bitmapsNubes, distVector);

		//Ordenar vector de tuplas en orden descendiente
		std::sort(distVector.begin(), distVector.end(), [](const std::pair<unsigned int, double> &left, const std::pair<unsigned int, double> &right) {
			return left.second < right.second;
		});
		int icosa = 0;
		for (auto p : distVector)
		{
			if (p.second <= 0.1)
			{
				icosa++;
			}
		}

		//std::cout<<"Elementos con distancia pequena: "<<icosa<<std::endl;

		unsigned int indice = q.ID;
		auto iter = std::find_if(distVector.begin(), distVector.end(), [&indice](const std::pair<unsigned int, double> &item) {
			return item.first == indice; });
		//Calcular recall

		if (iter != distVector.end())
		{
			auto i = std::distance(distVector.begin(), iter);

			std::cout << "Nube de Consulta:  " << indice << " - Nube Respuesta: " << distVector[i].first << " - Distancia: " << distVector[i].second << "\n";


			if (i<5)
			{
				r.recall_five += 1;
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}
			else if (i >= 5 && i<10)
			{
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}

			else if (i >= 10 && i<20)
			{
				r.recall_twenty += 1;
			}
		}
		else
		{
			std::cout << "No se encontro nube" << "\n";
		}


		end1 = std::chrono::high_resolution_clock::now();

		double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

		//Guardar tiempo de consulta y tamano de resultado en vector
		tconsulta.push_back(t);


	}

}

void VPTSearch(VptPointers<Punto, distCoseno> &vptree, int k, std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r)
{
	//Realizar consulta
	r.recall_five = 0;
	r.recall_ten = 0;
	r.recall_twenty = 0;
	int ii = 0;
	for (auto q : queriesNubes)
	{

		std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
		start1 = std::chrono::high_resolution_clock::now();

		//Calcular producto interno para todos los bitmaps
		std::vector<double> distances;
		std::vector<Punto> neighbors;
		vptree.search(q, k, &neighbors, &distances);

		std::vector<int> indicesFinales;

		//Obtener identificadores - Las respuestas ya se encuentran ordenadas por distancia
		for (auto resp : neighbors)
		{
			indicesFinales.push_back(resp.ID);
		}

		int indice = q.ID;
		auto iter = std::find_if(indicesFinales.begin(), indicesFinales.end(), [&indice](const int &item) {
			return item == indice; });
		//Calcular recall

		if (iter != indicesFinales.end())
		{
			auto i = std::distance(indicesFinales.begin(), iter);
			std::cout << "Nube de Consulta:  " << indice << " - Nube Respuesta: " << indicesFinales[i] << " - Distancia: " << distances[i] << "\n";


			if (i<5)
			{
				r.recall_five += 1;
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}
			else if (i >= 5 && i<10)
			{
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}

			else if (i >= 10 && i<20)
			{
				r.recall_twenty += 1;
			}
		}

		else
		{
			std::cout << "No se encontro nube" << "\n";
		}

		end1 = std::chrono::high_resolution_clock::now();

		double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

		//Guardar tiempo de consulta y tamano de resultado en vector
		tconsulta.push_back(t);


	}

}

void VPTSearch(VptPointers<Punto, distHamming> &vptree, int k, std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r)
{
	//Realizar consulta
	r.recall_five = 0;
	r.recall_ten = 0;
	r.recall_twenty = 0;
	int ii = 0;
	for (auto q : queriesNubes)
	{

		std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
		start1 = std::chrono::high_resolution_clock::now();

		//Calcular producto interno para todos los bitmaps
		std::vector<double> distances;
		std::vector<Punto> neighbors;
		vptree.search(q, k, &neighbors, &distances);

		std::vector<int> indicesFinales;

		//Obtener identificadores - Las respuestas ya se encuentran ordenadas por distancia
		for (auto resp : neighbors)
		{
			indicesFinales.push_back(resp.ID);
		}

		int indice = q.ID;
		auto iter = std::find_if(indicesFinales.begin(), indicesFinales.end(), [&indice](const int &item) {
			return item == indice; });
		//Calcular recall

		if (iter != indicesFinales.end())
		{
			int i = std::distance(indicesFinales.begin(), iter);
			std::cout << "Nube de Consulta:  " << indice << " - Nube Respuesta: " << indicesFinales[i] << " - Distancia: " << distances[i] << "\n";


			if (i<5)
			{
				r.recall_five += 1;
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}
			else if (i >= 5 && i<10)
			{
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}

			else if (i >= 10 && i<20)
			{
				r.recall_twenty += 1;
			}
		}


		else
		{
			std::cout << "No se encontro nube" << "\n";
		}

		end1 = std::chrono::high_resolution_clock::now();

		double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

		//Guardar tiempo de consulta y tamano de resultado en vector
		tconsulta.push_back(t);


	}

}

void VPTSearch(VptPointers<Punto, distJaccard> &vptree, int k, std::vector<Punto> &queriesNubes, std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r)
{
	//Realizar consulta
	r.recall_five = 0;
	r.recall_ten = 0;
	r.recall_twenty = 0;
	int ii = 0;
	for (auto q : queriesNubes)
	{

		std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
		start1 = std::chrono::high_resolution_clock::now();

		//Calcular producto interno para todos los bitmaps
		std::vector<double> distances;
		std::vector<Punto> neighbors;
		vptree.search(q, k, &neighbors, &distances);

		std::vector<int> indicesFinales;

		//Obtener identificadores - Las respuestas ya se encuentran ordenadas por distancia
		for (auto resp : neighbors)
		{
			indicesFinales.push_back(resp.ID);
		}

		int indice = q.ID;
		auto iter = std::find_if(indicesFinales.begin(), indicesFinales.end(), [&indice](const int &item) {
			return item == indice; });
		//Calcular recall

		if (iter != indicesFinales.end())
		{
			int i = std::distance(indicesFinales.begin(), iter);
			std::cout << "Nube de Consulta:  " << indice << " - Nube Respuesta: " << indicesFinales[i] << " - Distancia: " << distances[i] << "\n";


			if (i<5)
			{
				r.recall_five += 1;
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}
			else if (i >= 5 && i<10)
			{
				r.recall_ten += 1;
				r.recall_twenty += 1;
			}

			else if (i >= 10 && i<20)
			{
				r.recall_twenty += 1;
			}
		}
		else
		{
			std::cout << "No se encontro nube" << "\n";
		}

		end1 = std::chrono::high_resolution_clock::now();

		double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

		//Guardar tiempo de consulta y tamano de resultado en vector
		tconsulta.push_back(t);

	}

}

double distCoseno(const Punto &p1, const Punto &p2)
{
	return DistanciaCoseno(p1, p2, InnerProduct(p1, p2));
}

double distHamming(const Punto &p1, const Punto &p2)
{
	return (double)DistanciaHamming(p1, p2, InnerProduct(p1, p2));
}

double distJaccard(const Punto &p1, const Punto &p2)
{
	return DistanciaJaccard(p1, p2, InnerProduct(p1, p2));
}

/*
* // 27/02/17
// Indice metrico para nubes de puntos
// Referencias de unos en bitmap + Indice Metrico para busquda
// Libreria SDSL Lite 2.0
#include<sdsl/bit_vectors.hpp>


// STD
#include <vector>
#include <unordered_set>
#include <set>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <limits>
#include <map>
#include <functional>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <vp-tree.h>
#include <string>

// I/O Consola
#include <iostream>
#include <stdio.h>

// Timer
#include <chrono>
#include "codecfactory.h"
#include "intersection.h"

using namespace SIMDCompressionLib;

class Punto{
public:
std::vector<unsigned int> Posiciones;
unsigned int ID;
};

struct Recall{
public:
unsigned short int recall_five;
unsigned short int recall_ten;
unsigned short int recall_twenty;
};



// Funcion para leer archivo de texto - Declaracion
void Posiciones(std::string fileName, int delta, int cmax,std::vector<Punto> &resultado, std::set<unsigned int> &nubesID);
int Posicion(int x, int y, int cmax, int delta);

//Funcion para calcular distancia coseno
double DistanciaCoseno(const Punto &q, const Punto &bitmap, int pint);

//Funcion para calcular funcion de distancia para todos los bitmaps
void FuncionDistancia(Punto &q,std::vector<Punto> &bitmapsNubes, std::vector<std::pair<unsigned int, double>> &distV);
//Funcion para calcular promedio de un vector
double Promedio(const std::vector<double> v);

//Funcion para calcular desviacion estandar
double DesviacionEstandar(const std::vector<double> v, const double promedio);

//Funcion para calcular producto interno
int InnerProduct(const Punto &va,const Punto &vb);

void LinearSearch(std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);
//double DistanciaHamming(const Punto &q, const Punto &bitmap, int pint);
//double DistanciaJaccard(const Punto &q, const Punto &bitmap, int pint);
double distCoseno(const Punto &p1, const Punto &p2);
double distHamming(const Punto &p1, const Punto &p2);
double distJaccard(const Punto &p1, const Punto &p2);


void VPTSearch(VpTree<Punto, distCoseno> &vptree,int k,std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);
void VPTSearch(VpTree<Punto, distHamming> &vptree,int k,std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);
void VPTSearch(VpTree<Punto, distJaccard> &vptree,int k,std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r);


//Definir Pi
const double pi = std::acos(-1);




int main(){
// Declaracion de variables
std::string fileName;
std::string queryName;
int delta;
int cmax;
int num_nubes;

// Generar numeros aleatorios con distribucion uniforme
//boost::random::mt19937 gen;

// Map para almacenar para cada nube una lista de las celdas donde existe un punto en la nube (1's)
std::vector<Punto> resultadoNubes;
std::vector<Punto> resultadoQueries;

// Set para almacenar los identificadores de las nubes
std::set<unsigned int> nubesID;
std::set<unsigned int> nubesID2;

//Variables para calcular tiempo de ejecucion de funciones
std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

//Vector para guardar tiempos de consulta
std::vector<double> tconsulta;

//Vector para guardar tamano de resultado
//std::vector<double>resultado_size;

//Vector para almacenar espacio promedio en mb de bitvectors
std::vector<double> bvsize;
std::vector<double> bvsize2;

std::vector<double> t;

int tipo_indice=1;
int k=1;
int tipo_distancia = 1;
std::string indice_name = "Busqueda secuencial";
std::string distancia_name = "default";


//-------------------------------------------------------------------------------------------------------
// Inicia Construccion de indice

// Introducir nombre de archivo, tamano de celda y coordenada maxima
std::cout<<"Indice Metrico para nubes de puntos"<< std::endl;
std::cout<<"Lista de enteros de 1's de bitmap + Intersecciones utilizando SIMD + Indice Metrico para busqueda"<<std::endl;
std::cout<<"Introduce nombre de archivo de nubes de puntos a indexar"<< std::endl;
std::cin>>fileName;
std::cout<<"Introduce nombre de archivo de nubes de puntos de consulta"<<std::endl;
std::cin>>queryName;
std::cout<<"Introduce coordenada maxima de operacion"<<std::endl;
std::cin>>cmax;
std::cout<<"Introduce tamano de rejilla - Delta"<< std::endl;
std::cin>>delta;
std::cout<<"Introduce tipo de consulta:"<<std::endl;
std::cout<<"Busqueda secuencial: 1"<<std::endl;
std::cout<<"Vantage Point Tree: 2"<<std::endl;
std::cin>>tipo_indice;

if(tipo_indice==2){
indice_name = "Vantage Point Tree";
std::cout<<"Busqueda Knn: Introduce k"<<std::endl;
std::cin>>k;
std::cout<<"Introduce Distancia"<<std::endl;
std::cout<<"Distancia Coseno: 1"<<std::endl;
std::cout<<"Distancia Hamming: 2"<<std::endl;
std::cout<<"Distancia Jaccard: 3"<<std::endl;
std::cin>>tipo_distancia;
}


//Iniciar Lectura de archivo
std::cout<<"\nInicia Construccion de Indice"<< std::endl;
start = std::chrono::high_resolution_clock::now();
std::cout<<"Inicie lectura de archivo de nubes"<<std::endl;
Posiciones(fileName,delta,cmax,resultadoNubes, nubesID);
std::cout<<"Inicie lectura de archivo de nubes de consulta"<<std::endl;
Posiciones(queryName,delta,cmax,resultadoQueries, nubesID2);
std::cout<<"Finalizo Lectura de archivo"<< std::endl;


//Generar Indice
//std::vector<Punto> bitmapsNubes = GenerarIndice(resultadoNubes,nubesID,cmax,delta,bvsize);
//std::vector<Punto> queriesNubes = GenerarIndice(resultadoQueries,nubesID2,cmax,delta,bvsize2);

VpTree<Punto, distCoseno> vptree1;
VpTree<Punto, distHamming> vptree2;
VpTree<Punto, distJaccard> vptree3;

if(tipo_indice ==2){

switch(tipo_distancia){
case 1:
distancia_name = "Coseno";
vptree1.create(resultadoNubes);
break;
case 2:
distancia_name = "Hamming";
vptree2.create(resultadoNubes);
break;
case 3:
distancia_name = "Jaccard";
vptree3.create(resultadoNubes);
break;
default:
distancia_name = "Coseno";
vptree1.create(resultadoNubes);

}
}
end = std::chrono::high_resolution_clock::now();
std::cout<<"Finalizo Construccion de indice"<< std::endl;


//----------------------------------------------------------
//Inicia seccion de consulta
std::vector<std::pair<unsigned int, double>> distResultado;

Recall recall;
if(tipo_indice==2){

switch(tipo_distancia){
case 1:
VPTSearch(vptree1,k,resultadoQueries,resultadoNubes,tconsulta,recall);
break;
case 2:
VPTSearch(vptree2,k,resultadoQueries,resultadoNubes,tconsulta,recall);
break;
case 3:
VPTSearch(vptree3,k,resultadoQueries,resultadoNubes,tconsulta,recall);
break;
default:
VPTSearch(vptree1,k,resultadoQueries,resultadoNubes,tconsulta,recall);
}
}
else{
//std::cout<<"Inicia busqueda secuencial"<<std::endl;

LinearSearch(resultadoQueries,resultadoNubes,tconsulta, recall);
//std::cout<<"Termine busqueda lineal"<<std::endl;
}

//std::cout<<"LLegue aqui - tamano de tiempo: "<<tconsulta.size()<<std::endl;

//Tiempo promedio de consulta
double tm = Promedio(tconsulta);
//std::cout<<"LLegue aqui2"<<std::endl;
//Calcular desviacion estandar
double tsd = DesviacionEstandar(tconsulta, tm);
//std::cout<<"LLegue aqui3"<<std::endl;

//Calcular maximo y minimo
auto max_min = std::minmax_element(tconsulta.begin(), tconsulta.end());
//std::cout<<"LLegue aqui3"<<std::endl;
//Calcular tamano promedio de vector de resultados
//double rsize = Promedio(resultado_size);

//Tiempo promedio de Inner Product
//double tinnerp = Promedio(t);
//std::cout<<"LLegue aqui3"<<std::endl;
//Tamano promedio de sd bitvector
//double prom_vector = Promedio(bvsize);
//std::cout<<"LLegue aqui3"<<std::endl;
//double suma =  std::accumulate(bvsize.begin(), bvsize.end(),0)/1000000.0;

//Imprimir resultados
std::cout<<"Indice Metrico para nubes de puntos"<< std::endl;
std::cout<<"Lista de enteros de 1's de bitmap + Intersecciones utilizando SIMD + Indice Metrico para busqueda"<<std::endl;
std::cout<<"--------------------------------------------------"<<std::endl;
std::cout<<"Resultados de Construccion de Indice"<<std::endl;
std::cout<<indice_name<<std::endl;
std::cout<<"Nombre del Archivo de Nubes a indexar: "<<fileName<<std::endl;
std::cout<<"Nombre del Archivo de Nubes de consulta: "<<queryName<<std::endl;
std::cout<<"Coordenada maxima: "<<cmax<<std::endl;
std::cout<<"Tamano de celda (delta): "<<delta<<std::endl;
if(tipo_indice ==2)
{
std::cout<<"Distancia: "<< distancia_name<<std::endl;
std::cout<<"Numero de vecinos mas cercanos: "<<k<<std::endl;
}

std::cout<<"Numero de nubes a indexar: "<<resultadoNubes.size()<<std::endl;
std::cout<<"Numero de nubes de consulta: "<<resultadoNubes.size()<<std::endl;
std::cout<<"Tiempo de construccion: "<<std::chrono::duration_cast<std::chrono::seconds>(end - start).count()<<" s"<<std::endl;

std::cout<<"--------------------------------------------------"<<std::endl;
std::cout<<"Resultados de las consultas"<<std::endl;
std::cout<<"Tiempo promedio de consulta: "<<tm<< " us"<<std::endl;
std::cout<<"Desviacion estandar de tiempo de consulta: "<<tsd<<" us"<<std::endl;
std::cout<<"Tiempo maximo de consulta: "<<*max_min.second<<" us"<<std::endl;
std::cout<<"Tiempo minimo de consulta: "<<*max_min.first<<" us"<<std::endl;
//std::cout<<"Tamano promedio en bytes de SD Vector: "<<prom_vector<<" bytes"<<std::endl;
//std::cout<<"Tamano total en Mbytes de todos los SD Vectors: "<<suma<<std::endl;
std::cout<<"--------------------------------------------------"<<std::endl;
std::cout<<"Seccion de Recall"<<std::endl;
std::cout<<"Recall@5: "<<(double)recall.recall_five/nubesID2.size()<<std::endl;
std::cout<<"Recall@10: "<<(double)recall.recall_ten/nubesID2.size()<<std::endl;
std::cout<<"Recall@20: "<<(double)recall.recall_twenty/nubesID2.size()<<std::endl;

return 0;
}




void Posiciones(std::string fileName, int delta, int cmax, std::vector<Punto> &resultado, std::set<unsigned int> &nubesID){

//Variables para almacenar una linea del archivo de texto (csv,txt)
std::string line;
std::string item;
std::ifstream inputFile(fileName);
std::unordered_map<int,std::set<unsigned int>> temporal;

//Bandera para identificar el primer renglon del archivo (Encabezado)
int flag {0};

//Vector que dado un renglon del archivo de texto almacena las coordenadas e identificador de la nube ()
std::vector<float> d;

//Inicia lectura de archivo de texto
if (inputFile.is_open()){
//Ejecutar codigo mientras no se llegue al final del documento
while(!inputFile.eof()){

//Guardar contenido de una linea del documento
getline(inputFile, line, '\n');
d.clear();

//No almacenar primera linea (Encabezado del archivo)
if (flag==0){
flag = 1;
continue;}

//Almacenar contenido de primera linea
std::istringstream iss(line);
float x {0.0};

//Leer los datos separados por comas y almacenarlos
while(getline(iss,item,',')){
//Convertir string a double
x = std::stof(item);
d.push_back(x);
}

//Verificar que coordenadas e identificadores sean mayores que cero
if((d[0]>=0) && (d[1]>=0) && (d[2]>=0)){
unsigned int p {0};

//Calcular posicion de 1 en bitvector
p = (unsigned int) Posicion(std::floor(d[1]/delta),std::floor(d[2]/delta),cmax,delta);
unsigned int cid = (unsigned int) d[0];

//Guardar identificador de la nube en conjunto
nubesID.insert(cid);
//Almacenar resultados en mapa ID_Nube -> Posicion de 1 en bitmap
temporal[(unsigned int) d[0]].insert(p);
}
}
inputFile.close();
}

//std::cout<<"LLegue aqui"<<std::endl;
std::set<unsigned int> temp1;
std::vector<unsigned int> temp2;
//Ordenas listas de enteros
for(auto p:nubesID){
temp1 = temporal[p];
std::vector<unsigned int> temp2;

for(auto item:temp1)
{
temp2.push_back(item);

}
std::sort(temp2.begin(), temp2.end());
Punto punto;
punto.ID = p;
punto.Posiciones = temp2;
resultado.push_back(punto);

}

}

int Posicion(int x, int y, int cmax, int delta){
return x+ (cmax/delta*y);
}

double DistanciaCoseno(const Punto &q, const Punto &bitmap, int pint){
//Calcular norma de bitmap de consulta
double nq = std::sqrt(q.Posiciones.size());

//Calcular norma de bitmap en base de datos
//double nn = norm(bitmap, unos_b);
double nn = std::sqrt(bitmap.Posiciones.size());

double rp = pint/(nq*nn);

if(rp >= 1){
return 0;}

return std::acos(rp);

}

int DistanciaHamming(const Punto &q, const Punto &bitmap, int pint){
return q.Posiciones.size() + bitmap.Posiciones.size() - (2*pint);
}

double DistanciaJaccard(const Punto &q, const Punto &bitmap, int pint){
return 1 - ((double)pint / (q.Posiciones.size() + bitmap.Posiciones.size() - pint));
}


void FuncionDistancia(Punto &q,std::vector<Punto> &bitmapsNubes, std::vector<std::pair<unsigned int, double>> &distV){
for(auto nubeV:bitmapsNubes){
double distc = distCoseno(q,nubeV);
distV.push_back(std::make_pair(nubeV.ID,distc));
}
}

double Promedio(const std::vector<double> v){
return std::accumulate(v.begin(), v.end(),0)/v.size();
}

double DesviacionEstandar(const std::vector<double> v, const double promedio){
double var {0};

for(int i{0}; i<v.size(); i++){
var+= (v[i] - promedio)*(v[i]-promedio);
}

var = var/(v.size()-1);
return std::sqrt(var);
}


int InnerProduct(const Punto &va,const Punto &vb){

std::vector<unsigned int> a = va.Posiciones;
std::vector<unsigned int> b = vb.Posiciones;

//Tamano de bitmaps
unsigned int size_a = a.size();
unsigned int size_b = b.size();
unsigned int size_max = size_a;

if(size_a<=size_b){
size_max = size_b;
}

//Variable para guardar resultado
std::vector<unsigned int>r(size_max);

// using SIMD intersection
intersectionfunction inter = IntersectionFactory::getFromName("lemire_highlyscalable_intersect_SIMD");

// we are going to intersect mydata and mydata2 and write back
// the result to mydata2
//site_t
//std::cout<<"Tamano1: "<<a.size()<<" - Tamano2: "<<b.size()<<std::endl;
unsigned int intersize = inter(a.data(), a.size(), b.data(),
b.size(), r.data());



return intersize;
}

void LinearSearch(std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r){
//Realizar consulta
r.recall_five = 0;
r.recall_ten = 0;
r.recall_twenty = 0;


for(auto q: queriesNubes){

std::vector<std::pair<unsigned int, double>> distVector;

std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
start1 = std::chrono::high_resolution_clock::now();

//Calcular producto interno para todos los bitmaps
FuncionDistancia(q, bitmapsNubes,distVector);

//Ordenar vector de tuplas en orden descendiente
std::sort(distVector.begin(), distVector.end(), [](const std::pair<unsigned int,double> &left, const std::pair<unsigned int,double> &right) {
return left.second < right.second;
});
int icosa = 0;
for(auto p:distVector){
if(p.second <= 0.1){
icosa++;
}
}

//std::cout<<"Elementos con distancia pequena: "<<icosa<<std::endl;

unsigned int indice = q.ID;
auto iter = std::find_if(distVector.begin(), distVector.end(),[&indice](const std::pair<unsigned int,double> &item){
return item.first==indice;});
//Calcular recall

if(iter != distVector.end()){
auto i = std::distance(distVector.begin(), iter);

std::cout<<"Nube de Consulta:  "<< indice<<" - Nube Respuesta: "<<distVector[i].first<<" - Distancia: "<<distVector[i].second<< "\n";


if(i<5)
{
r.recall_five += 1;
r.recall_ten +=1;
r.recall_twenty += 1;
}
else if (i>=5 && i<10){
r.recall_ten += 1;
r.recall_twenty +=1;
}

else if(i>=10 && i<20){
r.recall_twenty +=1;
}
}
else
{
std::cout<<"No se encontro nube"<<"\n";
}


end1 = std::chrono::high_resolution_clock::now();

double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

//Guardar tiempo de consulta y tamano de resultado en vector
tconsulta.push_back(t);


}

}

void VPTSearch(VpTree<Punto, distCoseno> &vptree,int k,std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r){
//Realizar consulta
r.recall_five = 0;
r.recall_ten = 0;
r.recall_twenty = 0;
int ii =0;
for(auto q: queriesNubes){

std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
start1 = std::chrono::high_resolution_clock::now();

//Calcular producto interno para todos los bitmaps
std::vector<double> distances;
std::vector<Punto> neighbors;
vptree.search(q, k, &neighbors, &distances);

std::vector<int> indicesFinales;

//Obtener identificadores - Las respuestas ya se encuentran ordenadas por distancia
for(auto resp: neighbors)
{
indicesFinales.push_back(resp.ID);
}

int indice = q.ID;
auto iter = std::find_if(indicesFinales.begin(), indicesFinales.end(),[&indice](const int &item){
return item==indice;});
//Calcular recall

if(iter != indicesFinales.end()){
auto i = std::distance(indicesFinales.begin(), iter);
std::cout<<"Nube de Consulta:  "<< indice<<" - Nube Respuesta: "<<indicesFinales[i]<<" - Distancia: "<<distances[i]<< "\n";


if(i<5)
{
r.recall_five += 1;
r.recall_ten +=1;
r.recall_twenty += 1;
}
else if (i>=5 && i<10){
r.recall_ten += 1;
r.recall_twenty +=1;
}

else if(i>=10 && i<20){
r.recall_twenty +=1;
}
}

else
{
std::cout<<"No se encontro nube"<<"\n";
}

end1 = std::chrono::high_resolution_clock::now();

double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

//Guardar tiempo de consulta y tamano de resultado en vector
tconsulta.push_back(t);


}

}

void VPTSearch(VpTree<Punto, distHamming> &vptree,int k,std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r){
//Realizar consulta
r.recall_five = 0;
r.recall_ten = 0;
r.recall_twenty = 0;
int ii =0;
for(auto q: queriesNubes){

std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
start1 = std::chrono::high_resolution_clock::now();

//Calcular producto interno para todos los bitmaps
std::vector<double> distances;
std::vector<Punto> neighbors;
vptree.search(q, k, &neighbors, &distances);

std::vector<int> indicesFinales;

//Obtener identificadores - Las respuestas ya se encuentran ordenadas por distancia
for(auto resp: neighbors)
{
indicesFinales.push_back(resp.ID);
}

int indice = q.ID;
auto iter = std::find_if(indicesFinales.begin(), indicesFinales.end(),[&indice](const int &item){
return item==indice;});
//Calcular recall

if(iter != indicesFinales.end()){
int i = std::distance(indicesFinales.begin(), iter);
std::cout<<"Nube de Consulta:  "<< indice<<" - Nube Respuesta: "<<indicesFinales[i]<<" - Distancia: "<<distances[i]<< "\n";


if(i<5)
{
r.recall_five += 1;
r.recall_ten +=1;
r.recall_twenty += 1;
}
else if (i>=5 && i<10){
r.recall_ten += 1;
r.recall_twenty +=1;
}

else if(i>=10 && i<20){
r.recall_twenty +=1;
}
}


else
{
std::cout<<"No se encontro nube"<<"\n";
}

end1 = std::chrono::high_resolution_clock::now();

double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

//Guardar tiempo de consulta y tamano de resultado en vector
tconsulta.push_back(t);


}

}

void VPTSearch(VpTree<Punto, distJaccard> &vptree,int k,std::vector<Punto> &queriesNubes,std::vector<Punto> &bitmapsNubes, std::vector<double> &tconsulta, Recall &r){
//Realizar consulta
r.recall_five = 0;
r.recall_ten = 0;
r.recall_twenty = 0;
int ii =0;
for(auto q: queriesNubes){

std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
start1 = std::chrono::high_resolution_clock::now();

//Calcular producto interno para todos los bitmaps
std::vector<double> distances;
std::vector<Punto> neighbors;
vptree.search(q, k, &neighbors, &distances);

std::vector<int> indicesFinales;

//Obtener identificadores - Las respuestas ya se encuentran ordenadas por distancia
for(auto resp: neighbors)
{
indicesFinales.push_back(resp.ID);
}

int indice = q.ID;
auto iter = std::find_if(indicesFinales.begin(), indicesFinales.end(),[&indice](const int &item){
return item==indice;});
//Calcular recall

if(iter != indicesFinales.end()){
int i = std::distance(indicesFinales.begin(), iter);
std::cout<<"Nube de Consulta:  "<< indice<<" - Nube Respuesta: "<<indicesFinales[i]<<" - Distancia: "<<distances[i]<< "\n";


if(i<5)
{
r.recall_five += 1;
r.recall_ten +=1;
r.recall_twenty += 1;
}
else if (i>=5 && i<10){
r.recall_ten += 1;
r.recall_twenty +=1;
}

else if(i>=10 && i<20){
r.recall_twenty +=1;
}
}
else
{
std::cout<<"No se encontro nube"<<"\n";
}

end1 = std::chrono::high_resolution_clock::now();

double t = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

//Guardar tiempo de consulta y tamano de resultado en vector
tconsulta.push_back(t);

}

}

double distCoseno(const Punto &p1, const Punto &p2) {
return DistanciaCoseno(p1,p2,InnerProduct(p1,p2));
}

double distHamming(const Punto &p1, const Punto &p2) {
return (double) DistanciaHamming(p1,p2,InnerProduct(p1,p2));
}

double distJaccard(const Punto &p1, const Punto &p2) {
return DistanciaJaccard(p1,p2,InnerProduct(p1,p2));
}

*
*
*
*/
