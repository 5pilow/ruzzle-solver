#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <algorithm>
#include <time.h>

using namespace std;

#define NONE 0
#define DL 1
#define TL 2
#define DW 3
#define TW 4

typedef struct Cell {
	int x,y;
} Cell;

struct Index {
	bool letter;
	struct Index** ind;
}; 

typedef struct Word {
	string word;
	int points;
} Word;

list<string> _all_words;
Index* _index;
char _grid[4][4];
int _grid_points[4][4];
bool _visited[4][4];
string _word;
list<Cell> _path;
list<Word> _results;
string _alphabet = "abcdefghijklmnopqrstuvwxyz";
int _points[] = {
//  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x   y  z
	1, 3, 3, 2, 1, 4, 2, 4, 1, 0, 0, 2, 2, 1, 1, 3, 8, 1, 1, 1, 1, 5, 0, 10, 0, 0
};
int _length_scores[] = {
//	0  1  2  3  4  5  6   7   8   9   10  11  12  13  14  15  16
	0, 0, 0, 0, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60
};

int _directions[][2] = {
	{-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
};


bool is_valid_cell(int i, int j) {
	return !(i < 0 || j < 0 || i > 3 || j > 3 || _visited[i][j]);
}

void reset() {
	for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) _visited[i][j] = false;
	_word = "";
	_path.clear();
}

int word_exists_rec(string word, Index* index, int level) {
	
	if (word.size() == level) { // fin du mot
		
		bool letter = index->letter; // il faut que ça soit une lettre
		index->letter = false;
		return letter;
	}
		
	Index* next_index = index->ind[word[level] - 97];
	
	if (next_index->ind != NULL) {
		return word_exists_rec(word, next_index, level + 1);
	} else {
		return 2;
	}
}

int word_exists(std::string word) {
	return word_exists_rec(word, _index, 0);
}

void append_path(int i, int j) {
	Cell cell;
	cell.x = i;
	cell.y = j;
	_path.push_back(cell);
	_visited[i][j] = true;
	_word.push_back(_grid[i][j]);
}

void pop_path() {
	Cell* last_cell = &_path.back();
	_visited[last_cell->x][last_cell->y] = false;
	_path.pop_back();
	_word.erase(_word.size() - 1);
}


void add_result() {
	
	Word word;
	word.word = _word;
	word.points = 0; 
	
	int word_mult = 1;
	for (list<Cell>::iterator i = _path.begin(); i != _path.end(); ++i) {
		int bonus = _grid_points[i->x][i->y];
		int points = _points[_grid[i->x][i->y] - 97];
		if (bonus == DW || bonus == TW) {
			word_mult *= (bonus - 1);
			word.points += points;
		} else {
			word.points += points * (bonus + 1);
		}
	}
	word.points *= word_mult;
	word.points += _length_scores[_word.length()];
	
	_results.push_back(word);
}

void find_rec(int i, int j) {
	
	for (int d = 0; d < 8; ++d) {
				
		int x = i + _directions[d][0];
		int y = j + _directions[d][1];
		
		if (is_valid_cell(x, y)) {
			
			append_path(x, y);
			
			int res = word_exists(_word);
			
			if (res == 1) {
				add_result();
			}

			if (res != 2) {
				find_rec(x, y);
			}
			pop_path();
		}
	}
}	

void start_path(int i, int j) {
	reset();
	append_path(i, j);
	find_rec(i, j);
}

void build_index_rec(list<string>::iterator start, list<string>::iterator end, Index* index, int level) {
	
	index->letter = false;
	index->ind = new Index*[26];
	for (int i = 0; i < 26; ++i) index->ind[i] = new Index();
	
	// si le mot est trop court, il faut l'indiquer
	while (start->size() == level) {
		index->letter = true;
		++start;
		if (start == end) return;
	}
	
	int l1 = start->at(level) - 97;
	int l2;
	
	list<string>::iterator s = start;
	
	list<string>::iterator it;
	for (it = start; it != end; ++it) {
		
		l2 = it->at(level) - 97;
		
		if (l1 != l2) {
			build_index_rec(s, it, index->ind[l1], level + 1);
			s = it;
			l1 = l2;
		}
	}
	build_index_rec(s, end, index->ind[l2], level + 1);
}

void delete_index_rec(Index* index) {
	if (index != NULL) {
		if (index->ind != NULL) {
			for (int i = 0; i < 26; ++i) {
				delete_index_rec(index->ind[i]);
			}
			delete[] index->ind;
		}
		delete index;
	}
}

int main() {
	
	timespec start, end;
	float time;
	clock_gettime(CLOCK_REALTIME, &start);
	
	// Get words from dictionary
	ifstream file("dico.txt");

	string line;
	while (getline(file, line)) {
		_all_words.push_back(line);
	}
	
	// Get grid
	string grid_string = "cvedltieemrladam";
	cout << "Entrez les lettres : ";
	cin >> grid_string;
	cout << endl;
	
	int tile = 0;
	for (int i = 0; i < grid_string.length(); ++i) {
		char c = grid_string[i];
		if (isalpha(c)) { // lettre
			_grid_points[tile / 4][tile % 4] = NONE;
			_grid[tile / 4][tile % 4] = c;
		} else {
			if (!isalpha(grid_string[i + 1])) { // deux chiffres
				if (c == '2') _grid_points[tile / 4][tile % 4] = DW;
				if (c == '3') _grid_points[tile / 4][tile % 4] = TW;
				_grid[tile / 4][tile % 4] = grid_string[i + 2];
				i += 2;
			} else { // un chiffre
				if (c == '2') _grid_points[tile / 4][tile % 4] = DL;
				if (c == '3') _grid_points[tile / 4][tile % 4] = TL;
				_grid[tile / 4][tile % 4] = grid_string[i + 1];
				i++;
			}
		}
		tile++;
	}
	
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			cout << _grid[i][j] << " ";
		}
		cout << "   ";
		for (int j = 0; j < 4; ++j) {
			cout << (int)_grid_points[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
	
	// Prepare dico ! remove useless letter
	cout << "Mots dans le dictionnaire : " << _all_words.size() << endl << endl;
	
	string letters = _alphabet;
	
	for (int i = 0; i < grid_string.length(); ++i) {
		if (!isalpha(grid_string[i])) continue;
		int pos = letters.find(grid_string[i]);
		if (pos != -1) letters.erase(letters.begin() + pos);
	}
	
	list<string>::iterator i;
	int num_letters = letters.size(), j;
	for (i = _all_words.begin(); i != _all_words.end(); ++i) {
		for (j = 0; j < num_letters; ++j) {
			if (i->find(letters[j]) != -1) {
				_all_words.erase(i);
				--i; break;
			}
		}
	}
	
	cout << "Recherche parmis : " << _all_words.size() << " mots" << endl << endl;
	
	// Build index !
	cout << "Construction de l'index... ";
	
	_index = new Index();
	build_index_rec(_all_words.begin(), _all_words.end(), _index, 0);
	
	cout << "done." << endl;
	
	// Solve...
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			start_path(i, j);
		}
	}
	
	clock_gettime(CLOCK_REALTIME, &end);
	time = (float)((end.tv_sec * 1000000000 + end.tv_nsec) - (start.tv_sec * 1000000000 + start.tv_nsec)) / 1000000000;
	
	cout << endl;
	
	// Total des mots trouvés
	int num_results = _results.size();
	
	// Calcul des points totaux
	int total_points = 0;
	for (list<Word>::iterator j = _results.begin(); j != _results.end(); ++j) {
		total_points += j->points;
	}
	
	cout << "Meilleurs mots : " << endl << endl;
	
	for (int i = 0; i < 20; ++i) {
		list<Word>::iterator best = _results.begin();
		for (list<Word>::iterator j = ++best; j != _results.end(); ++j) {
			if (best->points < j->points) best = j;
			//~ if (best->word.length() > j->word.length()) best = j;
		}
		cout << best->word << " (" << best->points << ")" << endl;
		_results.erase(best);
	}
	
	cout << endl << "Mots trouvés au total : " << num_results << endl;
	cout << "Score maximum : " << total_points << endl << endl;
	
	cout << "Temps : " << time << " ms" << endl << endl;
	
	cout << "Delete index... ";
	delete_index_rec(_index);
	cout << "done." << endl;
}
