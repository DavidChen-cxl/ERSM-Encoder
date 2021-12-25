#pragma once
#include <vector>
#include <string>
#include <fstream>
#define SIZE 27
using namespace std;

extern string model[93];

struct item {
	int type;
	int count;
	item() { type = 0; count = 0; }
};

template<typename NOTE>
class shulker {
public:
	vector<item*> content;
	int size;
	int slot;
	int box;
	void add_box(int count) {
		for (int i = 0; i < count; i++) {
			item* b = new item[27];
			content.push_back(b);
		}
	}
	shulker() { add_box(1); size = 0; slot = 0; box = 0; }
	shulker(NOTE input, string choice) {
		add_box(1); size = 0; slot = 0; box = 0;
		content[0][0].type = input[0]->report(choice);
		content[0][0].count = 1;
		for (int i = 1; i < input.size(); i++) {
			if (input[i]->report(choice) == input[i - 1]->report(choice)) {
				content[box][slot].count++;
			}
			else {
				if (slot == SIZE - 1) {
					add_box(1); box++; slot = 0;
				}
				else slot++;
				content[box][slot].type = input[i]->report(choice);
				content[box][slot].count = 1;
			}
		}
		size = box * 27 + slot;
	}
};


void generate_function(ofstream& f, vector<item*> s, int c, string n, string type) {	//根据每格物品生成指令
	int box = 0, slot = 0; bool is_pitch = (type == "pitch");
	f << "give @p " << model[0] << n << " " << type << model[1];
	for (int i = 0; i <= c; i++) {
		box = i / SIZE; slot = i % SIZE;
		if (slot == 0) f << model[2];
		f << model[3] << model[9 + 52 * (1 - is_pitch) + s[box][slot].type] << model[4] << " \\\"" << model[36 + 41 * (1 - is_pitch) + s[box][slot].type] << model[5] << i % SIZE << model[6] << s[box][slot].count << "},";
		if (slot == 26) f << "]}},\"Slot\":" << box << model[6] << 1 << "},";
	}
	for (int i = c + 1; i < SIZE * (c / SIZE + 1); i++) {
		f << model[3] << model[7 + i % 2] << model[4] << " \\\"" << model[34 + i % 2] << model[5] << i % SIZE << model[6] << 1 << "},";
	}
	if ((c + 1) % SIZE != 0)
		f << "]}},\"Slot\":" << box << model[6] << 1 << "},]}}";
	else f << "]}}";
	f << "\n\n";
}

template<typename T>
void generate_txt(ofstream& f, T t, string n) {
	int l = t.size();
	f << n << endl << 'Z';
	for (int i = 0; i < l; i++) {
		if (t[i]->key != -1)
			f.put(char('A' + t[i]->report("pitch")));
		else
			f.put('A');
		f << hex << t[i]->report("other");
	}
	f << endl << endl;
}