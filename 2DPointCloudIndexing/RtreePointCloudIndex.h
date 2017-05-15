#pragma once
//Miguel Ramirez Chacon
//RTree - 30/01/17


// Libreria Boost
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
//#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/parameters.hpp>

// STD
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <limits>
#include <map>
#include <functional> 
#include <iomanip>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

// I/O Consola
#include <iostream>
#include <stdio.h>

// Timer
#include <time.h>
#include <chrono>


// Namespaces
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
typedef bg::model::point<float, 2, bg::cs::cartesian> point;
typedef bg::model::box<point> box;
typedef std::pair<point, unsigned> value;
typedef std::pair<int, int> hits;

// Funcion para leer archivo de texto - Declaracion
std::vector<value> readFile(std::string fileName, int cmax);
std::unordered_map<int, std::vector<value>> readQueries(std::string queryName, std::unordered_set<int> &IDs, int cmax);

int main()
{
	int cmax;
	int vecinos = 1;
	int tipo_consulta = 1;
	int delta = 10;
	std::string fileName;
	std::string queryFile;

	// Generador de numeros aleatorios
	boost::random::mt19937 gen;

	//Variables para calcular tiempo de ejecucion de funciones
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

	// Introducir datos
	std::cout << "Indice Rtree - Esquema de Votacion" << std::endl;
	std::cout << "Introduce nombre de archivo de Nubes de puntos" << std::endl;
	std::cin >> fileName;
	std::cout << "Introduce nombre de archivo de Nubes de consulta" << std::endl;
	std::cin >> queryFile;
	std::cout << "Introduce coordenada maxima" << std::endl;
	std::cin >> cmax;
	std::cout << "Introduce tipo de Consulta" << std::endl;
	std::cout << "K-NN: 1" << std::endl;
	std::cout << "Consulta de Rango: 2" << std::endl;
	std::cin >> tipo_consulta;

	if (tipo_consulta == 1)
	{
		std::cout << "K Vecinos mas cercanos - Introduce k" << std::endl;
		std::cin >> vecinos;
	}
	else
	{
		std::cout << "Consulta de Rango - Introduce delta" << std::endl;
		std::cin >> delta;
	}

	// Inicia construccion de indice
	start = std::chrono::high_resolution_clock::now();

	std::unordered_set<int> IDs;
	std::unordered_map<int, std::vector<value>> queries = readQueries(queryFile, IDs, cmax);

	std::cout << "\nInicia lectura de archivo de texto" << std::endl;
	std::vector<value> nubes = readFile(fileName, cmax);
	std::cout << "Finalizo lectura de archivo de texto" << std::endl;

	// Bulkloading utilizando algoritmo de Packing

	std::cout << "\nInicia indexado de datos - Rtree" << std::endl;
	bgi::rtree<value, bgi::rstar<20, 10> > rtree(nubes.begin(), nubes.end());
	end = std::chrono::high_resolution_clock::now();

	int tconst = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

	int num_q = IDs.size();

	double tiempo[num_q];
	int recall_five = 0;
	int recall_ten = 0;
	int recall_twenty = 0;

	int jj = 0;
	double cx = 0.0;
	double cy = 0.0;

	//Inicia proceso de consulta
	for (auto ind : IDs)
	{

		//Seleccionar nube para consulta
		auto cloud = queries[ind];

		//Vector donde guardo resultados
		std::vector<value> results_2;

		std::chrono::time_point<std::chrono::high_resolution_clock> start1, end1;
		start1 = std::chrono::high_resolution_clock::now();

		//Para cada punto de la nube de la consulta
		for (unsigned i = 0; i < cloud.size(); i++)
		{
			std::vector<value> result_s;

			if (tipo_consulta == 1)
			{
				//Consulta de KNN
				rtree.query(bgi::nearest(cloud[i].first, vecinos), std::back_inserter(result_s));
			}

			else
			{
				//Consulta de Rango
				cx = cloud[i].first.get<0>();
				cy = cloud[i].first.get<1>();

				//Rectangulo delimitador de consulta
				box query_box(point(cx - (delta / 2), cy - (delta / 2)), point(cx + (delta / 2), cy + (delta / 2)));
				rtree.query(bgi::intersects(query_box), std::back_inserter(result_s));
			}

			results_2.insert(results_2.end(), result_s.begin(), result_s.end());
		}

		//Obtener los identificadores de los resultados
		std::vector<int> ids;
		BOOST_FOREACH(value const& v, results_2)
		{
			ids.push_back(v.second);
		}

		// Contar numero de hits
		std::map<int, int> dup;
		std::for_each(ids.begin(), ids.end(), [&dup](int val) { dup[val]++; });

		//Guardar duplicados en vectores
		std::vector<hits> count;
		for (auto p : dup)
		{
			count.push_back(std::make_pair(p.first, p.second));
		}

		//Ordenar los resultados en orden descendente
		std::sort(count.begin(), count.end(), [](const std::pair<int, int> &left, const std::pair<int, int> &right) {
			return left.second > right.second;
		});

		//Obtener los primeros 20 indices del resultado de la consulta
		std::vector<int> r_ind;

		//Obtener los 20 primeros indices
		for (int i = 0; i<20; i++)
		{
			r_ind.push_back(count[i].first);
		}

		// Realizar busqueda de indice en vector
		auto it = find(r_ind.begin(), r_ind.end(), ind);

		//std::cout<<"Iteracion: "<<jj<<'\n';

		if (it != r_ind.end())
		{
			//Calcular posicion de indice en vector
			int posicion = std::distance(r_ind.begin(), it);


			//std::cout<<"Nube Consulta: "<<ind<<"- Nube Respuesta: "<<*it<<" - Posicion: "<<posicion<<'\n';

			//Calcular Recall@5, Recall@10 y Recall@20
			if (posicion<5)
			{
				recall_five += 1;
				recall_ten += 1;
				recall_twenty += 1;
			}
			else if (posicion<10 && posicion >= 5)
			{
				recall_ten += 1;
				recall_twenty += 1;
			}
			else
			{
				recall_twenty += 1;
			}
		}
		else
		{
			std::cout << "No se encontro nube" << '\n';
		}

		end1 = std::chrono::high_resolution_clock::now();
		double tpp = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();

		tiempo[jj] = tpp;
		jj++;
	}

	//Calcular tiempo promedio de consulta de nube
	double sum = 0.0;
	for (int i = 0; i<num_q; i++)
		sum = sum + tiempo[i];

	double promedio = sum / num_q;

	// Calcular varianza de tiempo de consulta de nube
	double var = 0;
	for (int i = 0; i < num_q; i++)
	{
		var += (tiempo[i] - promedio) * (tiempo[i] - promedio);
	}
	var = var / (num_q - 1);
	double sd = std::sqrt(var);

	std::cout << "=============================================" << std::endl;
	//Resumen - Entregar resultados
	std::cout << "R-Tree Indice - Esquema de Votacion" << std::endl;
	std::cout << "Archivo de Nubes de Puntos" << std::endl;
	std::cout << fileName << std::endl;
	std::cout << "Archivo de Nubes de Consulta" << std::endl;
	std::cout << queryFile << std::endl;
	std::cout << "Parametros" << std::endl;

	if (tipo_consulta == 1)
	{
		std::cout << "Vecinos mas cercanos (K): " << vecinos << std::endl;
	}
	else
	{
		std::cout << "Consulta de Rango (delta): " << delta << std::endl;
	}

	std::cout << "Coordenada Maxima: " << cmax << std::endl;
	std::cout << "Resumen - Construccion" << std::endl;
	std::cout << "Numero de Nubes a indexar: " << IDs.size() << std::endl;
	std::cout << "Tiempo de construccion: " << tconst << " s" << std::endl;
	std::cout << "=============================================" << std::endl;

	std::cout << "Resultados de la consulta" << std::endl;
	std::cout << "Promedio de tiempo de consulta de nube: " << promedio << " ms" << std::endl;

	//Desviacion estandar de tiempo de consulta de nube
	std::cout << "Desviacion Estandar de tiempo de consulta: " << sd << " ms" << std::endl;

	//Calcular maximo y minimo
	auto max_min = std::minmax_element(tiempo, tiempo + num_q);
	std::cout << "Minimo tiempo de consulta: " << *max_min.first << " ms" << std::endl;
	std::cout << "Maximo tiempo de consulta: " << *max_min.second << " ms" << std::endl;

	double rc5 = (double)recall_five / (double)IDs.size();
	double rc10 = (double)recall_ten / (double)IDs.size();
	double rc20 = (double)recall_twenty / (double)IDs.size();


	std::cout << "=============================================" << std::endl;
	std::cout << "Seccion - Recall" << std::endl;
	std::cout << "Recall@5: " << rc5 << std::endl;
	std::cout << "Recall@10: " << rc10 << std::endl;
	std::cout << "Recall@20: " << rc20 << std::endl;

	return 0;
}


std::vector<value> readFile(std::string fileName, int cmax)
{

	std::string line;
	std::string item;
	std::ifstream inputFile(fileName);

	std::vector<value> nubes;
	int flag = 0;
	std::vector<double> d;

	unsigned long long i = 0;

	if (inputFile.is_open())
	{
		while (getline(inputFile, line, '\n') && !inputFile.eof())
		{

			//Inicializar vector d
			d.clear();

			//Saltar linea de encabezado
			if (flag == 0)
			{
				flag = 1;
				continue;
			}

			std::istringstream iss(line);

			//Leer valores separados por comas
			while (getline(iss, item, ','))
			{
				double x = 0.0;
				x = std::stod(item);
				d.push_back(x);
			}

			if (d[0] >= 0 && d[1] >= 0 && d[2] >= 0 && d[1]<cmax && d[2]<cmax)
			{
				nubes.push_back(std::make_pair(point(d[1], d[2]), (int)d[0]));
			}

			i = i + 1;

			if (i % 1000000 == 0)
			{
				std::cout << "Iteracion: " << i << "- Nubes: " << i / 1000 << std::endl;
			}

		}
		std::cout << "Inicio para cerrar archivo de texto" << std::endl;
		inputFile.close();
	}
	std::cout << "Archivo de texto cerrado" << std::endl;
	return nubes;
}

std::unordered_map<int, std::vector<value>> readQueries(std::string queryName, std::unordered_set<int> &nubesID, int cmax)
{

	std::unordered_map<int, std::vector<value>> results;
	std::string line;
	std::string item;
	std::ifstream inputFile(queryName);

	int flag = 0;
	std::vector<double> d;

	unsigned long long i = 0;

	if (inputFile.is_open())
	{
		while (getline(inputFile, line, '\n') && !inputFile.eof())
		{
			//Inicializar vector d
			d.clear();

			//Saltar linea de encabezado
			if (flag == 0)
			{
				flag = 1;
				continue;
			}

			std::istringstream iss(line);

			//Leer valores separados por comas
			while (getline(iss, item, ','))
			{
				double x = 0.0;
				x = std::stod(item);
				d.push_back(x);
			}

			if (d[0] >= 0 && d[1] >= 0 && d[2] >= 0 && d[1]<cmax && d[2]<cmax)
			{
				results[(int)d[0]].push_back(std::make_pair(point(d[1], d[2]), (int)d[0]));
				//Guardar identificador de la nube en conjunto
				nubesID.insert((int)d[0]);
			}

			i = i + 1;

			if (i % 1000000 == 0)
			{
				std::cout << "Iteracion: " << i << "- Nubes: " << i / 1000 << std::endl;
			}

		}
		std::cout << "Inicio para cerrar archivo de texto" << std::endl;
		inputFile.close();
	}

	std::cout << "Archivo de texto cerrado" << std::endl;
	return results;

}

