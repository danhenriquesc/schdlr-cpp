#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <string> 
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>

const double EulerConstant = std::exp(1.0);
#define PUNICAO_IMPOSSIVEL 10000
#define PUNICAO_LVL1 9
#define PUNICAO_LVL2 8
#define PUNICAO_LVL3 7
#define PUNICAO_LVL4 6
#define PUNICAO_LVL5 5
#define PUNICAO_LVL6 4
#define PUNICAO_LVL7 3
#define PUNICAO_LVL8 2
#define PUNICAO_LVL9 1

using namespace std;

map<string, int> restricoesMultas;
map<string, int> mapaTemposAulas; //Mapa que relaciona TemposAulas com um índice | EX: SEGM1 = 0 / SEGM2 = 1 / QUIN4 = 106
map<int, string> mapaTemposAulasInvertido; //Inversao do de cima

class TempoAula{
	public:
		TempoAula(string d, string h){
			dia = d;
			horario = h;
		}
		string dia;
		string horario;
};

class SalaRecurso{
	public:
		SalaRecurso(int id, int q){
			idRecurso = id;
			quantidade = q;
		}
		int idRecurso;
		int quantidade;

};

class Sala{
	public:
		Sala(int id, string d, int n, int c){
			idSala = id;
			descricao = d;
			numero = n;
			capacidade = c;
		}
		Sala(){}

		int adicionarRecurso(SalaRecurso r){
			recursos.push_back(r);
			return 0;
		}
		int adicionarTempoAula(TempoAula t){
			temposAula.push_back(t);
			return 0;
		}

		int buscarRecurso(int id){
			vector<SalaRecurso>::iterator it;
			SalaRecurso found(0,0);
			SalaRecurso temp(0,0);

			for(it=recursos.begin() ; it < recursos.end(); it++) {
			    temp = *it;
			    if(temp.idRecurso == id){
			    	found = temp;
			    	break;	
			    }
			}
			return found.quantidade;
		}

		int idSala;
		string descricao;
		int numero;
		int capacidade;
		vector< SalaRecurso > recursos;
		vector< TempoAula > temposAula;
};

class Aula{
	public:
		Aula(int idA, string dA, int idT, string dT, int c, int idcp, int pcp){
			idAula = idA;
			descricaoAula = dA;

			idTurma = idT;
			descricaoTurma = dT;
			capacidade = c;
			idCursoPreferencial = idcp;
			periodoCursoPrefencial = pcp;
		}
		Aula(){}

		int idAula;
		string descricaoAula;

		int idTurma;
		string descricaoTurma;
		int capacidade;
		int idCursoPreferencial;
		int periodoCursoPrefencial;
};

class AgregacaoRecurso{
	public:
		AgregacaoRecurso(int id, int q, int f){
			idRecurso = id;
			quantidade = q;
			flexibilidade = f;
		}
		int idRecurso;
		int quantidade;
		int flexibilidade;
};

class Agregacao{
	public:
		Agregacao(int id, int t, int p=0, int s = 0){
			idAgregacao = id;
			tempos = t;
			idProfessor = p;
			idSala = s;
		}
		int adicionarRecurso(AgregacaoRecurso r){
			if(r.flexibilidade == 4)
				recursosObrigatorios.push_back(r);
			else
				recursosOpcionais.push_back(r);
			return 0;
		}
		int adicionarAulas(Aula a){
			aulas.push_back(a);
			idTurmas.push_back(a.idTurma);
			sort(idTurmas.begin(),idTurmas.end());

			if(a.idCursoPreferencial > 0){
				ostringstream oss;
				oss << a.idCursoPreferencial << "_" << a.periodoCursoPrefencial;
				//cout << oss.str() << endl;
				periodosPreferenciais.push_back(oss.str());
			}

			return 0;
		}
		int adicionarTempoAula(TempoAula t){
			temposAula.push_back(t);

			ostringstream oss;
			oss << t.dia << t.horario;

			temposAulaString.push_back(oss.str());

			// int origem = mapaTemposAulas[oss.str()];
			// for(int i=0; i<tempos; i++){
			// 	temposAulaString.push_back(mapaTemposAulasInvertido[origem+i]);
			// }
			
			return 0;
		}
		int totalAlunos(){
			vector<Aula>::iterator it;
			Aula a;
			int count = 0;

			for(it=aulas.begin() ; it < aulas.end(); it++) {
				a = *it;
			    count += a.capacidade;
			}
			return count;
		}

		int idAgregacao;
		int tempos;
		int idProfessor;
		int idSala;
		vector< AgregacaoRecurso > recursosObrigatorios;
		vector< AgregacaoRecurso > recursosOpcionais;
		vector< Aula > aulas;
		vector< int > idTurmas;
		vector< string > periodosPreferenciais;
		vector< TempoAula > temposAula;
		vector< string > temposAulaString;
};

class Restricoes{
	public:
		string codRestricao;
		string descricao;
		int ordem;
};

vector<Agregacao> agregacoes; //Lista de Agregações/Aulas que devem ser alocadas
vector<Sala> salas; //Lista de Salas Disponíveis
map<string, int> pesosRestricoes; // Pesos das restricoes
vector< vector<Agregacao> > solucao; //Solução Atual
vector< vector<Agregacao> > solucaoMelhor; //Melhor Solução
vector< vector<Agregacao> > solucaoGerada; //Solução Gerada

map<int, pair<int,int>> agregacoesMultiplosHorarios;
vector<int> agregacoesMultiplosHorariosLista;

int totalSalasVirtuais = 4;
int totalTemposDiarios = 18; //6 MANHÃ | 6 TARDE | 6 NOITE
int totalDiasAulas = 7;

Sala salaPorId(int);
int carregarDados(string);
void inicializarMapas();
void imprimirSolucao(vector< vector<Agregacao> >);
void gerarSolucaoInicial();
int calcularValor(vector< vector<Agregacao> >);
void geraNovaSolucao();
void trocarSala();
void trocarHorario(int);

void marcarTempo(string tag){
 // time_t t = time(0);   // get time now
 //    struct tm * now = localtime( & t );
 //    cout << tag << ": " << (now->tm_year + 1900) << '-' 
 //         << (now->tm_mon + 1) << '-'
 //         <<  now->tm_mday
 //         << endl;

    time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  cout << tag << ":"<<endl;
  strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
  string str(buffer);

  cout << str << endl << endl;
}

int main(int argc, char* argv[]){
	unsigned int seed = time(NULL);
	//cout << "SEMENTE ALEATÓRIA = " << seed << endl;
	srand(seed);
	int objective = atoi(argv[1]);
	// cout << objective << endl;
	
	//marcarTempo("LEITURA");
	carregarDados("entradateste.json");
	inicializarMapas();
	gerarSolucaoInicial();

	//imprimirSolucao(solucao);

	//Simulated Annealing Variaveis
	int nIter = 0;
	int maxIter = 10000;
	double T = 5;
	int delta;
	double alfa = 0.9;
	double x;
	int sg, sm, ss;

	sm = 1000000000;

	//Calculado temperatura inicial
	int accepted = 0;
	double perc = 0;
	// T = 1;
	//maxIter = 1000;
	//int deltaSum = 0;
	// alfa = 0.8;
	

	//imprimirSolucao(solucao);
	solucaoMelhor = solucao;
	sm = calcularValor(solucaoMelhor);
	ss = calcularValor(solucao);

	//marcarTempo("INÍCIO SA");
	//cout << "Temperatura;Percentual;Aceito;Custo;Restrições Graves;Restrições Leves;Salas Virtuais;Sala Pre-Def.;Cap. Sala;Rec. Obr.;Aulas Turma Simult.;Disc. Per. Simult.;Prof. Simult.;Hor. Pre-Def" << endl;

	int itTotal = 0;

	while(sm > objective){
		accepted = 0;

		while(nIter < maxIter){
			nIter++;
			itTotal++;
			if(nIter%100 < 90)
				trocarSala();
			else
				trocarHorario(0);

			sg = calcularValor(solucaoGerada);

			delta = sg - ss;

			// if(delta != 0)
			// 	cout << "DELTA = " << delta << " | " << nIter  << " | " << T << endl;
			if(delta < 0){
				solucao = solucaoGerada;
				ss = sg;

				//cout << nIter << " | " << sg << " | "<< sm << endl;

				if(sg < sm){
				//if(calcularValor(solucaoGerada) < calcularValor(solucaoMelhor)){
					solucaoMelhor = solucaoGerada;
					sm = sg;
					//cout << "NOVA MELHOR SOLUCAO" << endl << "TEMP = " << T  << " | VALOR = " << calcularValor(solucaoMelhor) << endl;
					//imprimirSolucao(solucaoMelhor);
				}

				accepted++;
			}else{
				x = ((double) rand() / (RAND_MAX));

				if(x < pow(EulerConstant, (double) -delta/T)){
					solucao = solucaoGerada;
					ss = sg;
					accepted++;
				}/*else{
					cout << x << " < " << pow(EulerConstant, (double) -delta/T) << "exp(" << (double) (-delta/T) << ")" << endl;
				}*/
			}
			//perc = (accepted*100)/nIter;
			//cout << T << ";" << perc << ";" << accepted << "/" << nIter << ";"<<sm<<";"<<restricoesMultas["TotalGrave"]<<";"<<restricoesMultas["TotalLeve"]<<";"<<restricoesMultas["SV"]<<";"<<restricoesMultas["SPD"]<<";"<<restricoesMultas["CAP"]<<";"<<restricoesMultas["RECOB"]<<";"<<restricoesMultas["AULSIM"]<<";"<<restricoesMultas["DISCSIM"]<<";"<<restricoesMultas["PROFSIM"]<<";" << restricoesMultas["HORPRE"] << endl;

			// cout << (time(NULL)-seed) << ";" << sm << endl;
			
			if(sm <= objective)
				break;
		}

		//perc = (accepted*100)/maxIter;
		// cout << (time(NULL)-seed) << ";" << sm << endl;
		//cout << T << ";" << perc << ";" << accepted << "/" << maxIter << ";"<<sm<<";"<<(sm/100)<<";"<<restricoesMultas["SV"]<<";"<<restricoesMultas["SPD"]<<";"<<restricoesMultas["CAP"]<<";"<<restricoesMultas["RECOB"]<<";"<<restricoesMultas["AULSIM"]<<";"<<restricoesMultas["DISCSIM"]<<";"<<restricoesMultas["PROFSIM"]<<";" << restricoesMultas["HORPRE"] << endl;
		//cout << "TEMPERATURA: " << T << " | PERC: " <<  perc << "% | ACCEPTED: " << accepted << "/" << maxIter << " | VALOR: " << sm <<  endl;

		T = T * alfa;
		nIter = 0;
	}

	cout << (time(NULL)-seed);
	// marcarTempo("FIM SA");

	// imprimirSolucao(solucaoMelhor);

	// cout << "VALOR FINAL: " << calcularValor(solucaoMelhor) << endl;

	return 0;
}


Sala salaPorId(int idSala){
	vector<Sala>::iterator it;

	for(it = salas.begin(); it < salas.end(); it++){
		Sala sala = *it;
		if(idSala == sala.idSala) return sala;
	}
}

int carregarDados(string nomeArquivo){
	ifstream t(nomeArquivo.c_str()); //Abrindo arquivo de dados gerado pelo sistema web [Está fixo para testes, mas será enviado pelo sistema]
	string dados((istreambuf_iterator<char>(t)), istreambuf_iterator<char>()); //Lendo conteudo do arquivo para variável dados

	rapidjson::Document document;
	document.Parse(dados.c_str());

	/* Lendo Salas */
	rapidjson::Value& s = document["salas"];
	assert(s.IsArray());

	for (rapidjson::SizeType i = 0; i < s.Size(); i++)
	{
	    const rapidjson::Value& c = s[i];

	    Sala sala(stoi(s[i]["idSala"].GetString()), s[i]["descricao"].GetString(), stoi(s[i]["numero"].GetString()), stoi(s[i]["capacidade"].GetString()));

    	/* Lendo Recursos das Salas */
		rapidjson::Value& r = s[i]["recursos"];
		assert(r.IsArray());
		for (rapidjson::SizeType j = 0; j < r.Size(); j++)
		{
			SalaRecurso recurso(stoi(r[j]["idRecurso"].GetString()),stoi(r[j]["quantidade"].GetString()));
			sala.adicionarRecurso(recurso);
	    }

	    /* Lendo TemposAula das Salas */
		rapidjson::Value& ta = s[i]["temposAula"];
		assert(ta.IsArray());
		for (rapidjson::SizeType j = 0; j < ta.Size(); j++)
		{
			TempoAula tempoaula(ta[j]["dia"].GetString(),ta[j]["horario"].GetString());
			sala.adicionarTempoAula(tempoaula);
	    }

	    salas.push_back(sala);
	}

	/* Lendo Agregacoes */
	rapidjson::Value& ag = document["agregacoes"];
	assert(ag.IsArray());

	for (rapidjson::SizeType i = 0; i < ag.Size(); i++)
	{
	    const rapidjson::Value& c = ag[i];

	    Agregacao agregacao(stoi(ag[i]["idAgregacao"].GetString()), stoi(ag[i]["tempos"].GetString()), stoi(ag[i]["idProfessor"].GetString()), stoi(ag[i]["idSala"].GetString()));

    	/* Lendo Recursos das Agregacoes */
		rapidjson::Value& r = ag[i]["recursos"];
		assert(r.IsArray());
		for (rapidjson::SizeType j = 0; j < r.Size(); j++)
		{
			AgregacaoRecurso recurso(stoi(r[j]["idRecurso"].GetString()),stoi(r[j]["quantidade"].GetString()),stoi(r[j]["flexibilidade"].GetString()));
			agregacao.adicionarRecurso(recurso);
	    }

	    /* Lendo TemposAula das Agregacoes */
		rapidjson::Value& ta = ag[i]["temposAula"];
		assert(ta.IsArray());
		for (rapidjson::SizeType j = 0; j < ta.Size(); j++)
		{
			TempoAula tempoaula(ta[j]["dia"].GetString(),ta[j]["horario"].GetString());
			agregacao.adicionarTempoAula(tempoaula);
	    }

	    /* Lendo Aulas das Agregacoes */
		rapidjson::Value& a = ag[i]["aulas"];
		assert(a.IsArray());
		for (rapidjson::SizeType j = 0; j < a.Size(); j++)
		{
			Aula aula(atoi(a[j]["idAula"].GetString()),
						a[j]["aulaDescricao"].GetString(),
						atoi(a[j]["idTurma"].GetString()),
						a[j]["turmaDescricao"].GetString(),
						atoi(a[j]["capacidade"].GetString()),
						atoi(a[j]["idCursoPreferencial"].GetString()),
						atoi(a[j]["periodoCursoPrefencial"].GetString()));
			
			agregacao.adicionarAulas(aula);
	    }

	    agregacoes.push_back(agregacao);
	}

	/* Lendo Restricoes */
	rapidjson::Value& rt = document["restricoes"];
	assert(rt.IsArray());

	int f=1;
	string tmp;
	for (rapidjson::SizeType i = 0; i < rt.Size(); i++)
	{
	    const rapidjson::Value& c = rt[i];
	    tmp = rt[i]["codRestricao"].GetString();
	    pesosRestricoes[tmp] = f;
	    f++;
	}

	return 0;
}

void inicializarMapas(){
	string d[7] = {"SEG","TER","QUA","QUI","SEX","SAB","DOM"};
	string h[3] = {"M", "T", "N"};

	for(int i = 0; i < totalDiasAulas; i++){
		for(int j = 0; j<3;j++){
			for(int k=0; k<6; k++){
				mapaTemposAulas[d[i] + h[j] + to_string(k+1)] = i*18 + j*6 + k;
			}
		}
	}

	for (map<string, int>::iterator i = mapaTemposAulas.begin(); i != mapaTemposAulas.end(); ++i)
    mapaTemposAulasInvertido[i->second] = i->first;
}

void imprimirSolucao(vector< vector<Agregacao> > s){
	int i, j;

	cout << ";";
	for(i=0; i<(salas.size()+totalSalasVirtuais); i++){
		if(i < salas.size()){
			cout << salas[i].descricao << "[" << salas[i].capacidade << "];";
		}else{
			cout << ("V" + to_string(i-salas.size()+1)) << ";";
		}
	}

	
	for(j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		cout << endl << mapaTemposAulasInvertido[j] << ";";
		
		for(i=0; i<(salas.size()+totalSalasVirtuais); i++){
			if(s[i][j].idAgregacao>0 && s[i][j].periodosPreferenciais.size()>0){
				cout << s[i][j].idAgregacao << "[" << s[i][j].totalAlunos() << "] [" << s[i][j].periodosPreferenciais[0]<< "];";
			}
			else{
				cout << s[i][j].idAgregacao << "[" << s[i][j].totalAlunos() << "];";
			}
		}
	}

	cout << endl << endl;
}

void gerarSolucaoInicial(){
	vector<Agregacao>::iterator agregacaoIt;
	int i, j, tempos;
	bool vazio, ok, salaDisp;
	Sala s;

	for(i=0; i<(salas.size()+totalSalasVirtuais); i++){
		vector <Agregacao> linhaTemp;

		for(j=0; j<totalTemposDiarios*totalDiasAulas;j++){
			if(i<salas.size()){
				//s = salaPorId(salas[i].idSala);
				s = salas[i];
				salaDisp = false;
				
				for(int k = 0; k < s.temposAula.size(); k++){
					if(mapaTemposAulasInvertido[j] == (s.temposAula[k].dia + s.temposAula[k].horario)){
						salaDisp = true;
						break;		
					}
				}
			}else{
				salaDisp = true;
			}

			if(salaDisp){
				linhaTemp.push_back(Agregacao(0,0,0,0));
			}else{
				linhaTemp.push_back(Agregacao(-2,0,0,0));
			}


		}
		solucao.push_back(linhaTemp);
	}

	int rand_index;
	for(agregacaoIt = agregacoes.begin(), i=0, j=0; agregacaoIt < agregacoes.end(); agregacaoIt++){
		ok = true;
		tempos = agregacaoIt->tempos;
		rand_index = rand()%agregacaoIt->temposAula.size();
		
		string tempoAulaStr = agregacaoIt->temposAula[rand_index].dia;

		ostringstream oss;
		oss << agregacaoIt->temposAula[rand_index].dia << agregacaoIt->temposAula[rand_index].horario;
		int tempoAula = mapaTemposAulas[oss.str()];
		j = tempoAula;

		if(j+tempos < totalTemposDiarios*totalDiasAulas){
			vazio = true;
			while(agregacaoIt->totalAlunos() > salas[i].capacidade){
				i++;
			}

			if(i >= (salas.size()+totalSalasVirtuais)){
				i = 0;
				//cout << "SALAS INSUFICIENTES NO HORARIO " << mapaTemposAulasInvertido[j] << endl;
			}

			for(int k=0; k<tempos;k++){
				if(solucao[i][j+k].idAgregacao != 0) vazio = false;
			}

			if(vazio){
				solucao[i][j] = *agregacaoIt;
				if(agregacaoIt->temposAula.size() > 1){
					agregacoesMultiplosHorarios[agregacaoIt->idAgregacao] = {i,j};
					agregacoesMultiplosHorariosLista.push_back(agregacaoIt->idAgregacao);
				}

				for(int k=1; k<tempos;k++){
					solucao[i][j+k] = Agregacao(-1,0,0,0);
				}	
				i = 0;
			}else{
				ok = false;
			}
		}else{
			ok = false;
		}

		i++;
		if(i >= (salas.size()+totalSalasVirtuais)){
			i = 0;
			//cout << "SALAS INSUFICIENTES NO HORARIO " << mapaTemposAulasInvertido[j] << endl;
		}

		if(!ok){
			agregacaoIt--;
		}
	}
}

int calcularValor(vector< vector<Agregacao> > v){
	int total = 0;

	//[OBRIGATÓRIOS]

	//Todas as aulas devem ser alocadas
	//Resolvida pela estrutura

	//Uma sala só pode ter uma agregação por vez
	//Resolvida pela estrutura

	restricoesMultas["TotalGrave"] = 0;
	restricoesMultas["TotalLeve"] = 0;

	//Nao ter aulas em Salas Virtuais
	restricoesMultas["SV"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		for(int i=salas.size(); i<(salas.size()+totalSalasVirtuais); i++){
			if(v[i][j].idAgregacao != 0){
				total += PUNICAO_IMPOSSIVEL;
				restricoesMultas["SV"]++;
				restricoesMultas["TotalGrave"]++;
			}
		}
	}

	restricoesMultas["SPD"] = 0;
	restricoesMultas["CAP"] = 0;
	restricoesMultas["RECOB"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		for(int i=0; i<salas.size(); i++){
			if(v[i][j].idAgregacao > 0){ //tem agregacao alocada nesse horario

				//Aula PODE SER em Sala Pré-definida.
				if(v[i][j].idSala != 0){ //agregacao exige sala pre-definida
					if(v[i][j].idSala != salas[i].idSala){
						total += PUNICAO_IMPOSSIVEL;
						restricoesMultas["SPD"]++;
						restricoesMultas["TotalGrave"]++;
					}
				}

				//Aulas|Agregações com N alunos devem ser alocadas em salas com capacidade X, tal que X >= N. 
				if(v[i][j].totalAlunos() > salas[i].capacidade){
					total += PUNICAO_IMPOSSIVEL;
					restricoesMultas["CAP"]++;
					restricoesMultas["TotalGrave"]++;
				}

				//Restrições de recursos obrigatórios.
				vector<AgregacaoRecurso>::iterator it;
				AgregacaoRecurso arTemp(0,0,0);

				for(it=v[i][j].recursosObrigatorios.begin() ; it < v[i][j].recursosObrigatorios.end(); it++) {
				    arTemp = *it;
				    
				    if(arTemp.quantidade > salas[i].buscarRecurso(arTemp.idRecurso)){
				    	total += PUNICAO_IMPOSSIVEL;
				    	restricoesMultas["RECOB"]++;
				    	restricoesMultas["TotalGrave"]++;
				    }
				}
			}
		}
	}

	//Duas aulas de uma mesma turma não podem ocorrer simultaneamente.
	int h1, h2;
	vector<int> intersecaoTurmas;

	restricoesMultas["AULSIM"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		for(int i=0; i<salas.size()+totalSalasVirtuais; i++){
			h1 = j;

			while(v[i][h1].idAgregacao == -1) h1--;

			for(int k=i+1; k<salas.size()+totalSalasVirtuais; k++){
				h2 = j;
				while(v[k][h2].idAgregacao == -1) h2--;

				intersecaoTurmas.clear();
				set_intersection(v[i][h1].idTurmas.begin(),v[i][h1].idTurmas.end(),v[k][h2].idTurmas.begin(),v[k][h2].idTurmas.end(),back_inserter(intersecaoTurmas));

				if(intersecaoTurmas.size() > 0){
					total += PUNICAO_IMPOSSIVEL;
					restricoesMultas["AULSIM"]++;
					restricoesMultas["TotalGrave"]++;
				}
			}
		}
	}


	//Duas aulas de disciplinas do mesmo periodo não devem ocorrer simultaneamente
	vector<string> pp1;
	set<string> pp2;

	restricoesMultas["DISCSIM"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		pp1.clear();
		pp2.clear();

		for(int i=0; i<salas.size()+totalSalasVirtuais; i++){
			h1 = j;

			while(v[i][h1].idAgregacao == -1) h1--;

			pp1.insert(pp1.end(),v[i][h1].periodosPreferenciais.begin(),v[i][h1].periodosPreferenciais.end());
		}

		unsigned size = pp1.size();
		for( unsigned i = 0; i < size; ++i ) pp2.insert( pp1[i] );

		if(pp1.size() != pp2.size()){
			// cout << endl << endl;
			// for (auto i = pp1.begin(); i != pp1.end(); ++i)
   //  			cout << *i << ' ';

   //  		cout << endl;
   //  		for (auto i = pp2.begin(); i != pp2.end(); ++i)
   //  			cout << *i << ' ';

   //  		cout << endl << endl;

		 	total += PUNICAO_IMPOSSIVEL;
		 	restricoesMultas["DISCSIM"]++;
		 	restricoesMultas["TotalGrave"]++;
		}
	}

	//Professor não pode dar duas aulas simultaneamente
	vector<int> p1;
	set<int> p2;

	restricoesMultas["PROFSIM"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		p1.clear();
		p2.clear();

		for(int i=0; i<salas.size()+totalSalasVirtuais; i++){
			h1 = j;

			while(v[i][h1].idAgregacao == -1) h1--;

			if(v[i][h1].idProfessor > 0){
				p1.push_back(v[i][h1].idProfessor);
				p2.insert(v[i][h1].idProfessor);
			}
		}

		if(p1.size() != p2.size()){
			total += PUNICAO_IMPOSSIVEL;
			restricoesMultas["PROFSIM"]++;
			restricoesMultas["TotalGrave"]++;
		}
	}
	
	//Sala deve estar disponível no horário da aula
	//Resolvido pela estrutura

	//Possíveis Horários pré-estabelecidos
	//Resolvido pela estrutura

	restricoesMultas["HORPRE"] = 0;
	// for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
	// 	for(int i=0; i<salas.size()+totalSalasVirtuais; i++){
	// 		if(v[i][j].idAgregacao > 0){
	// 			if (find(v[i][j].temposAulaString.begin(), v[i][j].temposAulaString.end(), mapaTemposAulasInvertido[j]) == v[i][j].temposAulaString.end() ){
	// 				total += PUNICAO_IMPOSSIVEL;
	// 				restricoesMultas["HORPRE"]++;
	// 				restricoesMultas["TotalGrave"]++;
	// 			}
	// 		}
	// 	}
	// }


	//[NÃO OBRIGATÓRIOS]
	
	int multaParcial;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		for(int i=0; i<salas.size(); i++){
			if(v[i][j].idAgregacao > 0){ //tem agregacao alocada nesse horario

				//Aulas devem, preferencialmente, ser alocadas em salas com capacidade igual ou o mais próximo possível da sua necessidade. 
				multaParcial = 0;

				if(salas[i].capacidade > v[i][j].totalAlunos())
					multaParcial = (salas[i].capacidade-v[i][j].totalAlunos())/10;

				total += multaParcial * PUNICAO_LVL4;
				restricoesMultas["TotalLeve"] = restricoesMultas["TotalLeve"] + multaParcial * PUNICAO_LVL4;


				//Aulas devem, preferencialmente, serem alocadas em salas com recursos iguais ou o mais próximo possível da sua necessidade.
				vector<AgregacaoRecurso>::iterator it;
				AgregacaoRecurso arTemp(0,0,0);

				for(it=v[i][j].recursosObrigatorios.begin() ; it < v[i][j].recursosObrigatorios.end(); it++) {
				    arTemp = *it;
			
					multaParcial = 0;	

					if(salas[i].buscarRecurso(arTemp.idRecurso) > arTemp.quantidade)
						multaParcial = salas[i].buscarRecurso(arTemp.idRecurso)/arTemp.quantidade - 1;

					total += multaParcial * PUNICAO_LVL3;
					restricoesMultas["TotalLeve"] = restricoesMultas["TotalLeve"] + multaParcial * PUNICAO_LVL3;
				}

				for(it=v[i][j].recursosOpcionais.begin() ; it < v[i][j].recursosOpcionais.end(); it++) {
				    arTemp = *it;
			
					multaParcial = 0;	

					if(salas[i].buscarRecurso(arTemp.idRecurso) > arTemp.quantidade)
						multaParcial = (salas[i].buscarRecurso(arTemp.idRecurso)*10)/arTemp.quantidade - 10;

					total += multaParcial * PUNICAO_LVL3;
					restricoesMultas["TotalLeve"] = restricoesMultas["TotalLeve"] + multaParcial * PUNICAO_LVL3;
				}

				//Restrições de recursos NÃO-obrigatórios. [RECDISP] [RECNECE] [RECALTNECE]
				AgregacaoRecurso arTemp2(0,0,0);

				multaParcial = 0;
				int punicao = 0;
				for(it=v[i][j].recursosOpcionais.begin() ; it < v[i][j].recursosOpcionais.end(); it++) {
				    arTemp2 = *it;

				    if(arTemp2.flexibilidade == 3)
				    	punicao = PUNICAO_LVL1;
				    else if(arTemp2.flexibilidade == 2)
				    	punicao = PUNICAO_LVL5;
				    else if(arTemp2.flexibilidade == 1)
				    	punicao = PUNICAO_LVL9;

				    if(arTemp2.quantidade > salas[i].buscarRecurso(arTemp2.idRecurso)){
				    	total += punicao;	
				    	restricoesMultas["TotalLeve"] += punicao;
				    }
				}
			}
		}
	}	

	//Aulas de turmas do mesmo período devem, preferencialmente, serem alocadas na mesma sala ou em salas próximas, evitando o deslocamento em grandes distâncias[SALAPROX]
	int f;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		if(!((j+1)%(totalTemposDiarios)) ) continue;

		for(int i=0; i<salas.size()+totalSalasVirtuais; i++){
			if ( (v[i][j].idAgregacao != 0) && (v[i][j].idAgregacao != -2) && (v[i][j+1].idAgregacao != -1) && (v[i][j+1].idAgregacao != -2) ){
				f = j;
				while(v[i][f].idAgregacao == -1){
					f--;
				}

				sort(v[i][f].periodosPreferenciais.begin(), v[i][f].periodosPreferenciais.end());
				for(int k=0; k<salas.size()+totalSalasVirtuais; k++){
					if( (v[k][j+1].idAgregacao == 0) || (v[k][j+1].idAgregacao == -2) ) continue;

					vector<string> intersecao;
					
					sort(v[k][j+1].periodosPreferenciais.begin(), v[k][j+1].periodosPreferenciais.end());
					set_intersection(v[i][f].periodosPreferenciais.begin(), v[i][f].periodosPreferenciais.end(),v[k][j+1].periodosPreferenciais.begin(), v[k][j+1].periodosPreferenciais.end(),back_inserter(intersecao));

					if(intersecao.size() > 0){
						if(i!=k){
							multaParcial = abs(salas[i].numero - salas[k].numero)/10;
							total += multaParcial * PUNICAO_LVL2;
							restricoesMultas["TotalLeve"] += multaParcial * PUNICAO_LVL2;
							//break;
						}
					}

				}
			}
		}
	}


	//Aulas do mesmo professor devem ser na mesma sala (vantagem da chave) [MSMPROF]
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		if(!((j+1)%(totalTemposDiarios)) ) continue;

		for(int i=0; i<salas.size()+totalSalasVirtuais; i++){
			if ( (v[i][j].idAgregacao != 0) && (v[i][j].idAgregacao != -2) && (v[i][j+1].idAgregacao != -1) && (v[i][j+1].idAgregacao != -2) ){
				f = j;
				while(v[i][f].idAgregacao == -1){
					f--;
				}

				if(v[i][f].idProfessor <= 0) continue;

				for(int k=0; k<salas.size()+totalSalasVirtuais; k++){
					if( (v[k][j+1].idAgregacao == 0) || (v[k][j+1].idAgregacao == -2) || (v[k][j+1].idProfessor <= 0) ) continue;

					if(v[i][f].idProfessor == v[k][j+1].idProfessor){
						if(i!=k){
							total += PUNICAO_LVL6;
							restricoesMultas["TotalLeve"] += PUNICAO_LVL6;
							break;
						}
					}

				}
			}
		}
	}

	//Gerar intervalos entre aulas na mesma sala para dar tempo de limpeza[INTERVLIMP]
	int aulastotais;
	for(int i=0; i<salas.size(); i++){

		for(int k=0; k<totalDiasAulas; k++){
			aulastotais = 0;
			for(int j=k*totalTemposDiarios+2; j<(k+1)*totalTemposDiarios;j++){
				if(v[i][j].idAgregacao > 0){
					aulastotais++;
				}
			}

			if(aulastotais >= 16){
				total += PUNICAO_LVL7;
				restricoesMultas["TotalLeve"] += PUNICAO_LVL8;
			}
		}
	}

	//Evitar aulas no N5 N6
	for(int j=totalTemposDiarios-2; j<totalTemposDiarios*totalDiasAulas;j+=totalTemposDiarios){
		for(int i=0; i<salas.size(); i++){
			if(v[i][j].idAgregacao != 0 && v[i][j].idAgregacao != -1){
				total += PUNICAO_LVL7;
				restricoesMultas["TotalLeve"] += PUNICAO_LVL7;
			}

			if(v[i][j+1].idAgregacao != 0 && v[i][j+1].idAgregacao != -1){
				total += PUNICAO_LVL7;
				restricoesMultas["TotalLeve"] += PUNICAO_LVL7;
			}
		}
	}

	return total;
}

void geraNovaSolucao(){
	int ai, aj, bi, bj;
	int totSalas = salas.size()+totalSalasVirtuais;
	int totAulas = totalTemposDiarios*totalDiasAulas;
	int maxTempos, iTmp;
	bool ok;

	solucaoGerada = solucao;

	ai = rand()%totSalas;
	aj = rand()%totAulas;
	bi = rand()%totSalas;
	bj = rand()%totAulas;

	if((solucaoGerada[ai][aj].idAgregacao >= 0) && (solucaoGerada[bi][bj].idAgregacao >=0)){
		maxTempos = max(solucaoGerada[ai][aj].tempos,solucaoGerada[bi][bj].tempos);

		ok = true;
		for(int k=1;k<maxTempos;k++){
			if((solucaoGerada[ai][aj+k].idAgregacao!=0) && (solucaoGerada[ai][aj+k].idAgregacao!=-1)) ok = false;
			if((solucaoGerada[bi][bj+k].idAgregacao!=0) && (solucaoGerada[bi][bj+k].idAgregacao!=-1)) ok = false;
		}
		if((aj + maxTempos > totAulas) || (bj + maxTempos > totAulas)) ok = false;

		if(ok){
			for(int k=1;k<maxTempos;k++){
				iTmp = solucaoGerada[ai][aj+k].idAgregacao;
				solucaoGerada[ai][aj+k].idAgregacao = solucaoGerada[bi][bj+k].idAgregacao;
				solucaoGerada[bi][bj+k].idAgregacao = iTmp;
			}

			Agregacao tmp = solucaoGerada[ai][aj];
			solucaoGerada[ai][aj] = solucaoGerada[bi][bj];
			solucaoGerada[bi][bj] = tmp;
		}else{
			geraNovaSolucao();
		}
	}else{
		geraNovaSolucao();
	}
}

void trocarSala(){
	int ai, aj, bi, bj;
	int totSalas = salas.size()+totalSalasVirtuais;
	int totAulas = totalTemposDiarios*totalDiasAulas;
	int maxTempos, iTmp;
	bool ok;

	solucaoGerada = solucao;

	ai = rand()%totSalas;
	aj = rand()%totAulas;
	bi = rand()%totSalas;
	bj = aj;

	if((solucaoGerada[ai][aj].idAgregacao >= 0) && (solucaoGerada[bi][bj].idAgregacao >=0)){
		maxTempos = max(solucaoGerada[ai][aj].tempos,solucaoGerada[bi][bj].tempos);

		ok = true;
		for(int k=1;k<maxTempos;k++){
			if((solucaoGerada[ai][aj+k].idAgregacao!=0) && (solucaoGerada[ai][aj+k].idAgregacao!=-1)) ok = false;
			if((solucaoGerada[bi][bj+k].idAgregacao!=0) && (solucaoGerada[bi][bj+k].idAgregacao!=-1)) ok = false;
		}
		if((aj + maxTempos > totAulas) || (bj + maxTempos > totAulas)) ok = false;

		if(ok){
			for(int k=1;k<maxTempos;k++){
				iTmp = solucaoGerada[ai][aj+k].idAgregacao;
				solucaoGerada[ai][aj+k].idAgregacao = solucaoGerada[bi][bj+k].idAgregacao;
				solucaoGerada[bi][bj+k].idAgregacao = iTmp;
			}

			Agregacao tmp = solucaoGerada[ai][aj];
			solucaoGerada[ai][aj] = solucaoGerada[bi][bj];
			solucaoGerada[bi][bj] = tmp;
		}else{
			trocarSala();
		}
	}else{
		trocarSala();
	}
}
void trocarHorario(int rep){
	if(rep < 10){

		vector<string>::iterator it;
		string ta;

		int ai, aj, bi, bj;

		int totSalas = salas.size()+totalSalasVirtuais;
		int totAulas = totalTemposDiarios*totalDiasAulas;

		int maxTempos, iTmp;
		bool ok, lvl1;

		int randInt;

		solucaoGerada = solucao;

		//agregacoesMultiplosHorarios[agregacaoIt->idAgregacao].second;
		//agregacoesMultiplosHorariosLista

		int indice = agregacoesMultiplosHorariosLista[rand()%agregacoesMultiplosHorariosLista.size()];

		// ai = rand()%totSalas;
		// aj = rand()%totAulas;
		ai = agregacoesMultiplosHorarios[indice].first;
		aj = agregacoesMultiplosHorarios[indice].second;

		//bi = ai;
		bi = rand()%totSalas;

		lvl1 = false;

		if(solucaoGerada[ai][aj].idAgregacao > 0){

			if(solucaoGerada[ai][aj].temposAulaString.size() > 1){
				randInt = rand() % ( solucaoGerada[ai][aj].temposAulaString.size());

				bj = mapaTemposAulas[solucaoGerada[ai][aj].temposAulaString[randInt]];
				lvl1 = true;
			}else{
				trocarHorario(++rep);
			}
		}else{
			trocarHorario(++rep);
		}


		if(lvl1){
			// cout << ai << "x" << aj << " | " << bi << "x" << bj << "|" << solucaoGerada[ai][aj].idAgregacao << "|" << solucaoGerada[bi][bj].idAgregacao << endl;

			if((solucaoGerada[ai][aj].idAgregacao >= 0) && (solucaoGerada[bi][bj].idAgregacao ==0)){
				//cout << "ta" << endl;
				maxTempos = max(solucaoGerada[ai][aj].tempos,solucaoGerada[bi][bj].tempos);

				ok = true;
				for(int k=1;k<maxTempos;k++){
					if((solucaoGerada[ai][aj+k].idAgregacao!=0) && (solucaoGerada[ai][aj+k].idAgregacao!=-1)) ok = false;
					if((solucaoGerada[bi][bj+k].idAgregacao!=0) && (solucaoGerada[bi][bj+k].idAgregacao!=-1)) ok = false;
				}
				if((aj + maxTempos > totAulas) || (bj + maxTempos > totAulas)) ok = false;

				if(ok){
					for(int k=1;k<maxTempos;k++){
						iTmp = solucaoGerada[ai][aj+k].idAgregacao;
						solucaoGerada[ai][aj+k].idAgregacao = solucaoGerada[bi][bj+k].idAgregacao;
						solucaoGerada[bi][bj+k].idAgregacao = iTmp;
					}

					Agregacao tmp = solucaoGerada[ai][aj];
					solucaoGerada[ai][aj] = solucaoGerada[bi][bj];
					solucaoGerada[bi][bj] = tmp;

					//cout << "trocou" << endl;
				}else{
					trocarHorario(++rep);
				}
			}else{
				trocarHorario(++rep);
			}
		}

	}
}
