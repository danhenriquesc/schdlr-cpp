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
#include <algorithm>

const double EulerConstant = std::exp(1.0);
#define PUNICAO_IMPOSSIVEL 100
#define PUNICAO_LVL1 7
#define PUNICAO_LVL2 6
#define PUNICAO_LVL3 5
#define PUNICAO_LVL4 4
#define PUNICAO_LVL5 3
#define PUNICAO_LVL6 2
#define PUNICAO_LVL7 1

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

			ostringstream oss;
			oss << a.idCursoPreferencial << "_" << a.periodoCursoPrefencial;
			periodosPreferenciais.push_back(oss.str());

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

int main(){
	srand(time(NULL));
	
	marcarTempo("LEITURA");
	inicializarMapas();
	carregarDados("entradateste.json");
	gerarSolucaoInicial();

	//imprimirSolucao(solucao);

	//LAHC variaveis
	int custoInicial;
	int T = 1000;
	int L[1000];
	int iteracoes = 0;
	int iteracoes_ociosas = 0;
	int novoCusto;
	int custoAtual;
	int v;

	custoInicial = calcularValor(solucao);
	fill(begin(L), end(L), custoInicial);

	cout << "It;Custo;Restrições Violadas;Salas Virtuais;Sala Pre-Def.;Cap. Sala;Rec. Obr.;Aulas Turma Simult.;Disc. Per. Simult.;Prof. Simult.;Hor. Pre-Def" << endl;
	
	while((iteracoes < 5000000) || iteracoes_ociosas < iteracoes*0.02){
		geraNovaSolucao();
		novoCusto = calcularValor(solucaoGerada);
		custoAtual = calcularValor(solucao);

		if(novoCusto>=custoAtual)
			iteracoes_ociosas++;
		else
			iteracoes_ociosas=0;

		v = iteracoes % T;

		if((novoCusto < L[v]) || (novoCusto <= custoAtual)){
			solucao = solucaoGerada;
			custoAtual = novoCusto;
		}

		if(custoAtual < L[v])
			L[v] = custoAtual;

		iteracoes++;

		//cout <<  << " : " << custoAtual << endl;
		cout << iteracoes << ";"<<custoAtual<<";"<<(custoAtual/100)<<";"<<restricoesMultas["SV"]<<";"<<restricoesMultas["SPD"]<<";"<<restricoesMultas["CAP"]<<";"<<restricoesMultas["RECOB"]<<";"<<restricoesMultas["AULSIM"]<<";"<<restricoesMultas["DISCSIM"]<<";"<<restricoesMultas["PROFSIM"]<<";" << restricoesMultas["HORPRE"] << endl;
	}
	
	marcarTempo("FIM LAHC");
	imprimirSolucao(solucao);

	cout << "VALOR FINAL: " << calcularValor(solucao) << endl;

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
			cout << s[i][j].idAgregacao << "[" << s[i][j].totalAlunos() << "];";
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

	for(agregacaoIt = agregacoes.begin(), i = 0, j = 0; agregacaoIt < agregacoes.end(); agregacaoIt++){
		ok = true;
		tempos = agregacaoIt->tempos;

		if(j+tempos < totalTemposDiarios*totalDiasAulas){
			vazio = true;
			for(int k=0; k<tempos;k++){
				if(solucao[i][j+k].idAgregacao != 0) vazio = false;
			}
			if(vazio){
				solucao[i][j] = *agregacaoIt;
				for(int k=1; k<tempos;k++){
					solucao[i][j+k] = Agregacao(-1,0,0,0);
				}	
			}else{
				ok = false;
			}
		}else{
			ok = false;
		}

		i++;
		if(i >= (salas.size()+totalSalasVirtuais)){
			i = 0;
			j++;
		}
		if(j >= totalTemposDiarios*totalDiasAulas){
			cout << "SALAS INSUFICIENTES" << endl;
			break;
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

	//Nao ter aulas em Salas Virtuais
	restricoesMultas["SV"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		for(int i=salas.size(); i<(salas.size()+totalSalasVirtuais); i++){
			if(v[i][j].idAgregacao != 0){
				total += PUNICAO_IMPOSSIVEL;
				restricoesMultas["SV"]++;
			}
		}
	}

	restricoesMultas["SPD"] = 0;
	restricoesMultas["CAP"] = 0;
	restricoesMultas["RECOB"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		for(int i=0; i<salas.size(); i++){

			//Aula PODE SER em Sala Pré-definida.
			if(v[i][j].idSala != 0){ //agregacao exige sala pre-definida
				if(v[i][j].idSala != salas[i].idSala){
					total += PUNICAO_IMPOSSIVEL;
					restricoesMultas["SPD"]++;
				}
			}

			
			if(v[i][j].idAgregacao != 0){ //tem agregacao alocada nesse horario

				//Aulas|Agregações com N alunos devem ser alocadas em salas com capacidade X, tal que X >= N. 
				if(v[i][j].totalAlunos() > salas[i].capacidade){
					total += PUNICAO_IMPOSSIVEL;
					restricoesMultas["CAP"]++;
				}

				//Restrições de recursos obrigatórios.
				vector<AgregacaoRecurso>::iterator it;
				AgregacaoRecurso arTemp(0,0,0);
				int quantidade;

				for(it=v[i][j].recursosObrigatorios.begin() ; it < v[i][j].recursosObrigatorios.end(); it++) {
				    arTemp = *it;
				    
				    if(arTemp.quantidade > salas[i].buscarRecurso(arTemp.idRecurso)){
				    	total += PUNICAO_IMPOSSIVEL;
				    	restricoesMultas["RECOB"]++;
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
		 	total += PUNICAO_IMPOSSIVEL;
		 	restricoesMultas["DISCSIM"]++;
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
		}
	}
	
	//Sala deve estar disponível no horário da aula
	//Resolvido pela estrutura

	//Possíveis Horários pré-estabelecidos

	restricoesMultas["HORPRE"] = 0;
	for(int j=0; j<totalTemposDiarios*totalDiasAulas;j++){
		for(int i=0; i<salas.size()+totalSalasVirtuais; i++){
			if(v[i][j].idAgregacao > 0){
				if (find(v[i][j].temposAulaString.begin(), v[i][j].temposAulaString.end(), mapaTemposAulasInvertido[j]) == v[i][j].temposAulaString.end() ){
					total += PUNICAO_IMPOSSIVEL;
					restricoesMultas["HORPRE"]++;
				}
			}
		}
	}


	//[NÃO OBRIGATÓRIOS]

	//Aulas devem, preferencialmente, ser alocadas em salas com capacidade igual ou o mais próximo possível da sua necessidade. 

	//Aulas devem, preferencialmente, serem alocadas em salas com recursos iguais ou o mais próximo possível da sua necessidade.

	//Aulas de turmas do mesmo período devem, preferencialmente, serem alocadas na mesma sala ou em salas próximas, evitando o deslocamento em grandes distâncias[SALAPROX]

	//Restrições de recursos NÃO-obrigatórios. [RECDISP] [RECNECE] [RECALTNECE]

	//Aulas do mesmo professor devem ser na mesma sala (vantagem da chave) [MSMPROF]

	//Gerar intervalos entre aulas na mesma sala para dar tempo de limpeza[INTERVLIMP]

	//Evitar aulas no N5 N6

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
